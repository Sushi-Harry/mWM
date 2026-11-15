#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>
#include <sqlite3.h>

#define IDLE_LIMIT_MS (5 * 6 * 1000)
#define FLUSH_INTERVAL_SEC 5

static volatile sig_atomic_t running = 1;

static void handle_sig(int sig){
    (void) sig;
    running = 0;
}

static int ignore_bad_window(Display *dpy, XErrorEvent *e){
    (void)dpy;
    if(e->error_code == BadWindow) return 0;
    return 0;
}

static sqlite3 *init_db(void){
    const char* home = getenv("HOME");
    if(!home) return NULL;

    char dir_path[512], db_path[512];
    snprintf(dir_path, sizeof(dir_path), "%s/.local/share", home);
    mkdir(dir_path, 0755);
    snprintf(dir_path, sizeof(dir_path), "%s/.local/share/mwm", home);
    mkdir(dir_path, 0755);
    snprintf(db_path, sizeof(dir_path), "%s/usage.db", dir_path);

    sqlite3 *db = NULL;
    if(sqlite3_open(db_path, &db) != SQLITE_OK){
        fprintf(stderr, "[mwm-tracker] SQLite open error: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS app_usage ("
        "  date TEXT NOT NULL,"
        "  app_name TEXT NOT NULL,"
        "  seconds INTEGER NOT NULL DEFAULT 0,"
        "  PRIMARY KEY (date, app_name)"
        ");";
    
    char *err = NULL;
    if(sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK){
        fprintf(stderr, "[mwm-tracker] SQLite init error: %s\n", err);
        sqlite3_free(err);
    }

    return db;
}

static void log_usage(sqlite3 *db, const char* app_name, int delta_sec) {
    if (!db || delta_sec <= 0 || !app_name || strcmp(app_name, "Desktop") == 0) return;

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char date_str[16];
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);

    const char* sql =
        "INSERT INTO app_usage (date, app_name, seconds) VALUES (?, ?, ?) "
        "ON CONFLICT(date, app_name) DO UPDATE SET seconds = seconds + excluded.seconds;";
    
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, date_str, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, app_name, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, delta_sec);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

static void get_active_app(Display *dpy, Window root, Atom net_active, char *out_name, size_t len) {
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;

    snprintf(out_name, len, "Desktop");

    if (XGetWindowProperty(dpy, root, net_active, 0, 1, False, XA_WINDOW,
                           &actual_type, &actual_format, &nitems, &bytes_after, &prop) == Success && prop) {
        if (nitems > 0) {
            Window active_win = *(Window *)prop;
            if (active_win != None) {
                XClassHint hint;
                if (XGetClassHint(dpy, active_win, &hint)) {
                    if (hint.res_class && strlen(hint.res_class) > 0) {
                        snprintf(out_name, len, "%s", hint.res_class);
                    } else if (hint.res_name) {
                        snprintf(out_name, len, "%s", hint.res_name);
                    }
                    if (hint.res_name) XFree(hint.res_name);
                    if (hint.res_class) XFree(hint.res_class);
                }
            }
        }
        XFree(prop);
    }
}

int main(void) {
    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);

    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "[mwm-tracker] Cannot open X display.\n");
        return EXIT_FAILURE;
    }
    XSetErrorHandler(ignore_bad_window);

    sqlite3 *db = init_db();
    if (!db) {
        XCloseDisplay(dpy);
        return EXIT_FAILURE;
    }

    Window root = DefaultRootWindow(dpy);
    Atom net_active = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    XSelectInput(dpy, root, PropertyChangeMask);

    XScreenSaverInfo *ss_info = XScreenSaverAllocInfo();
    int x11_fd = ConnectionNumber(dpy);

    char current_app[128];
    get_active_app(dpy, root, net_active, current_app, sizeof(current_app));
    time_t last_tick = time(NULL);

    printf("[mwm-tracker] Tracking started. Logging to ~/.local/share/mwm/usage.db\n");

    while (running) {
        while (XPending(dpy)) {
            XEvent ev;
            XNextEvent(dpy, &ev);
            if (ev.type == PropertyNotify && ev.xproperty.atom == net_active) {
                time_t now = time(NULL);
                int elapsed = (int)(now - last_tick);

                XScreenSaverQueryInfo(dpy, root, ss_info);
                if (ss_info->idle < IDLE_LIMIT_MS && elapsed > 0) {
                    log_usage(db, current_app, elapsed);
                }

                last_tick = now;
                get_active_app(dpy, root, net_active, current_app, sizeof(current_app));
            }
        }

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(x11_fd, &fds);
        struct timeval tv = { .tv_sec = FLUSH_INTERVAL_SEC, .tv_usec = 0 };

        int ready = select(x11_fd + 1, &fds, NULL, NULL, &tv);
        if (ready == 0) {
            /* Periodic flush so active session time is saved even without window switches */
            time_t now = time(NULL);
            int elapsed = (int)(now - last_tick);

            XScreenSaverQueryInfo(dpy, root, ss_info);
            if (ss_info->idle < IDLE_LIMIT_MS && elapsed > 0) {
                log_usage(db, current_app, elapsed);
            }
            last_tick = now;
        }
    }

    /* Final flush on shutdown */
    int final_elapsed = (int)(time(NULL) - last_tick);
    if (final_elapsed > 0) log_usage(db, current_app, final_elapsed);

    if (ss_info) XFree(ss_info);
    sqlite3_close(db);
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
}