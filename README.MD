# MWM - A Tiling Window Manager

## BUILD
```make j$(nproc)```<br />
```./mwm```

## Testing On Xephyr
I'm hoping it'll work on WSL without any extra hassle. I could test it myself but yk.. it's too much of a hassle to set up WSL on windows and then develop everything on windows when I already have arch installed on my laptop. Lmk how it goes though if you test it. <br />
```Xephyr -br -ac -noreset -screen 1280x720 :2 &```<br />
```DIPLAY=:2 ./mwm &```<br />