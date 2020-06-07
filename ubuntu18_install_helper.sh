#!/bin/env bash

# 失败时退出
set -e
# 打印执行的命令
set -v

has_swapItemsAt=(  'contest.cpp'
                   'contestant.cpp'
                   )

for i in "${has_swapItemsAt[@]}"; do
    sed -i 's/swapItemsAt/swap/g' "$i"
done

sudo apt install qt5-default build-essential
g++ watcher_unix.cpp -o watcher_unix -O2
qmake lemon.pro
make
