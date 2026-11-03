#!/bin/bash
make -j$(nproc) || exit 1
Xephyr -br -ac -noreset -screen 1920x1080 :2 &
XEPHYR_PID=$!
sleep 1
DISPLAY=:2 ./mwm