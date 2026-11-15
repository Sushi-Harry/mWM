# MWM - A Tiling Window Manager

## DEPENDENCIES
### Arch Linux
```sudo pacman -S --needed base-devel libx11 libxrandr libxss sqlite raylib picom xorg-xinit``` <br />

### Ubuntu / Debian
```sudo apt update && sudo apt install -y build-essential libx11-dev libxrandr-dev libxss-dev libsqlite3-dev libraylib-dev picom xinit```


## BUILD

```
# 1. Compile the window manager and companion tools
make

# 2. Install binaries and Xsession entry system-wide
sudo make install

# 3. Copy GLSL colorblind shaders into your user config directory
mkdir -p ~/.config/mwm/shaders
cp shaders/*.glsl ~/.config/mwm/shaders/
```
<br />

## Testing On Xephyr
I'm hoping it'll work on WSL without any extra hassle. I could test it myself but yk.. it's too much of a hassle to set up WSL on windows and then develop everything on windows when I already have arch installed on my laptop. Lmk how it goes though if you test it. <br />
```Xephyr -br -ac -noreset -screen 1280x720 :2 &```<br />
```DIPLAY=:2 ./mwm &```<br />