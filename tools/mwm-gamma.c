#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

static double clamp(double val, double min, double max){
    if(val >= max) return max;
    if(val <= min) return min;
    return val; 
}

static void kelvin_to_rgb(int temp, double *r, double *g, double *b){
    temp = clamp(temp, 1000, 10000);
    double t = temp / 100.0;

    if(t <= 66.0){
        *r = 1.0;
        *g = clamp((99.4708025861 * log(t) - 161.1195681631) / 255.0, 0.0, 1.0);
    }else{
        *r = clamp(329.698727446 * pow(t - 60.0, -0.1332047592) / 255.0, 0.0, 1.0);
        *g = clamp(288.1221695283 * pow(t - 60.0, -0.0755148492) / 255.0, 0.0, 1.0);
    }

    if(t >= 66.0){
        *b = 1.0;
    }else if(t <= 19.0){
        *b = 0.0;
    }else{
        *b = clamp((138.5177312231 * log(t - 10.0) - 305.0447927307) / 255.0, 0.0, 1.0);
    }
}

static void apply_gamma(Display *dpy, double r, double g, double b){
    Window root = DefaultRootWindow(dpy);
    XRRScreenResources *res = XRRGetScreenResources(dpy, root);
    if(!res){
        fprintf(stderr, "[mWM-gamma] Failed to query XRandR resources\n");
        return;
    }

    for(int i = 0; i < res->ncrtc; i++){
        XRRCrtcGamma *gamma = XRRGetCrtcGamma(dpy, res->crtcs[i]);
        if(!gamma || gamma->size <= 1) continue;

        for(int j = 0; j < gamma->size; j++){
            double norm = (double)j / (gamma->size - 1);
            gamma->red[j] = (unsigned short)(norm * 65535.0 * r);
            gamma->green[j] = (unsigned short)(norm * 65535.0 * g);
            gamma->blue[j] = (unsigned short)(norm * 65535.0 * b);
        }

        XRRSetCrtcGamma(dpy, res->crtcs[i], gamma);
        XRRFreeGamma(gamma);
    }

    XRRFreeScreenResources(res);
    XSync(dpy, False);
}

int main(int argc, char **argv){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <Kelvin: 1500-6500 | reset>\n", argv[0]);
        return EXIT_FAILURE;
    }

    Display *dpy = XOpenDisplay(NULL);
    if(!dpy){
        fprintf(stderr, "[mWM-gamma] Cannot Open X Display\n");
        return EXIT_FAILURE;
    }

    if(strcmp(argv[1], "reset") == 0){
        apply_gamma(dpy, 1.0, 1.0, 1.0);
    }else{
        int kelvin = atoi(argv[1]);
        double r, g, b;
        kelvin_to_rgb(kelvin, &r, &g, &b);
        apply_gamma(dpy, r, g, b);
    }

    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
}