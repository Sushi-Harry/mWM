#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "../include/raygui.h"

#include <sqlite3.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

#define SOCKET_PATH "/tmp/mwm.sock"

struct AppStat {
    std::string name;
    int seconds;
};

/* 1. Send a single command string to mWM over /tmp/mwm.sock */
static void send_ipc(const char *cmd) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return;

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (sockaddr *)&addr, sizeof(addr)) == 0) {
        write(fd, cmd, strlen(cmd));
    }
    close(fd);
}

/* 2. Restart picom with the chosen GLSL colorblind shader */
static void apply_colorblind_mode(int mode) {
    const char *shaders[] = {
        nullptr,
        "protanopia.glsl",
        "deuteranopia.glsl",
        "tritanopia.glsl",
        "achromatopsia.glsl"
    };

    if (mode == 0 || !shaders[mode]) {
        system("pkill -x picom; picom -b >/dev/null 2>&1 &");
        return;
    }

    const char *home = getenv("HOME");
    if (!home) return;

    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "pkill -x picom; picom -b --backend glx --window-shader-fg \"%s/.config/mwm/shaders/%s\" >/dev/null 2>&1 &",
             home, shaders[mode]);
    system(cmd);
}

/* 3. Load today's screen time and per-app rows from SQLite */
static void load_usage_stats(std::vector<AppStat> &stats, int &total_sec) {
    stats.clear();
    total_sec = 0;

    const char *home = getenv("HOME");
    if (!home) return;

    char db_path[512];
    snprintf(db_path, sizeof(db_path), "%s/.local/share/mwm/usage.db", home);

    sqlite3 *db = nullptr;
    if (sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return;
    }

    time_t now = time(nullptr);
    char today[16];
    strftime(today, sizeof(today), "%Y-%m-%d", localtime(&now));

    const char *sql = "SELECT app_name, seconds FROM app_usage WHERE date = ? ORDER BY seconds DESC LIMIT 8;";
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, today, -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *app = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            int sec = sqlite3_column_int(stmt, 1);
            stats.push_back({ app ? app : "Unknown", sec });
            total_sec += sec;
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
}

static std::string format_duration(int sec) {
    int h = sec / 3600;
    int m = (sec % 3600) / 60;
    int s = sec % 60;
    char buf[32];
    if (h > 0) snprintf(buf, sizeof(buf), "%dh %02dm", h, m);
    else snprintf(buf, sizeof(buf), "%dm %02ds", m, s);
    return buf;
}

int main() {
    InitWindow(500, 380, "mWM Control Center");
    SetTargetFPS(60);

    int active_tab = 0;
    bool night_light = false;
    float kelvin = 3500.0f;
    int cb_mode = 0, prev_cb_mode = 0;
    float master_ratio = 0.50f;
    float border_width = 2.0f;
    bool show_bar = true;

    struct Swatch { Color color; const char *hex; };
    Swatch palette[] = {
        { {  82, 148, 226, 255 }, "0x5294e2" }, // Blue
        { { 180, 142, 173, 255 }, "0xb48ead" }, // Purple
        { { 163, 190, 140, 255 }, "0xa3be8c" }, // Green
        { { 235, 203, 139, 255 }, "0xebcb8b" }, // Amber
        { { 191,  97, 106, 255 }, "0xbf616a" }  // Coral
    };

    std::vector<AppStat> stats;
    int total_sec = 0;
    load_usage_stats(stats, total_sec);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        // Top navigation tabs (3 buttons x 150px = 450px wide)
        GuiToggleGroup((Rectangle){ 25, 15, 150, 28 }, "Accessibility;Tiling;Screen Time", &active_tab);

        if (active_tab == 0) {
            // --- Night Light ---
            if (GuiCheckBox((Rectangle){ 25, 70, 20, 20 }, "Enable Night Light", &night_light)) {
                system(night_light ? TextFormat("mwm-gamma %d", (int)kelvin) : "mwm-gamma reset");
            }

            GuiSliderBar((Rectangle){ 25, 105, 340, 20 }, nullptr, TextFormat("%d K", (int)kelvin), &kelvin, 1800.0f, 6500.0f);
            if (night_light && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                system(TextFormat("mwm-gamma %d", (int)kelvin));
            }

            // --- Colorblind Filter ---
            GuiLabel((Rectangle){ 25, 155, 220, 20 }, "Colorblind Filter (picom GLSL)");
            GuiComboBox((Rectangle){ 25, 180, 250, 28 }, "Off (Normal);Protanopia;Deuteranopia;Tritanopia;Achromatopsia", &cb_mode);
            if (cb_mode != prev_cb_mode) {
                apply_colorblind_mode(cb_mode);
                prev_cb_mode = cb_mode;
            }
        }
        else if (active_tab == 1) {
            // 1. Master Split Ratio
            GuiLabel((Rectangle){ 25, 60, 250, 20 }, "Master Area Split Ratio");
            GuiSliderBar((Rectangle){ 25, 85, 340, 20 }, nullptr, TextFormat("%.2f", master_ratio), &master_ratio, 0.15f, 0.90f);
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), (Rectangle){ 25, 85, 340, 20 })) {
                send_ipc(TextFormat("SET_MASTER_FACT %.2f", master_ratio));
            }

            // 2. Border Width
            GuiLabel((Rectangle){ 25, 120, 250, 20 }, "Window Border Width");
            GuiSliderBar((Rectangle){ 25, 145, 340, 20 }, nullptr, TextFormat("%d px", (int)border_width), &border_width, 0.0f, 10.0f);
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), (Rectangle){ 25, 145, 340, 20 })) {
                send_ipc(TextFormat("SET_BORDER_WIDTH %d", (int)border_width));
            }

            // 3. Active Border Color Swatches
            GuiLabel((Rectangle){ 25, 180, 250, 20 }, "Active Border Accent");
            for (int i = 0; i < 5; i++) {
                Rectangle box = { 25.0f + i * 42.0f, 208.0f, 32.0f, 26.0f };
                DrawRectangleRec(box, palette[i].color);
                DrawRectangleLinesEx(box, 1, DARKGRAY);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), box)) {
                    send_ipc(TextFormat("SET_BORDER_ACTIVE %s", palette[i].hex));
                }
            }

            // 4. Bar Toggle & Hot Reload
            if (GuiCheckBox((Rectangle){ 25, 255, 20, 20 }, "Show Top Status Bar", &show_bar)) {
                send_ipc("TOGGLE_BAR");
            }

            if (GuiButton((Rectangle){ 25, 305, 150, 30 }, "Reload mWM")) {
                send_ipc("RESTART_WM");
            }
        }
        else if (active_tab == 2) {
            // --- Screen Time & Per-App Breakdown ---
            std::string total_str = "Total Today: " + format_duration(total_sec);
            GuiLabel((Rectangle){ 25, 65, 280, 24 }, total_str.c_str());

            if (GuiButton((Rectangle){ 385, 62, 90, 26 }, "Refresh")) {
                load_usage_stats(stats, total_sec);
            }

            if (stats.empty()) {
                GuiLabel((Rectangle){ 25, 110, 350, 20 }, "No usage data recorded yet (start mwm-tracker).");
            } else {
                float y = 105.0f;
                for (const auto &app : stats) {
                    float frac = (total_sec > 0) ? (static_cast<float>(app.seconds) / total_sec) : 0.0f;
                    std::string dur = format_duration(app.seconds);
                    GuiProgressBar((Rectangle){ 125, y, 260, 20 }, app.name.c_str(), dur.c_str(), &frac, 0.0f, 1.0f);
                    y += 30.0f;
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}