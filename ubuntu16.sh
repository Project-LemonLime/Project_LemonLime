#!/bin/env bash

# 失败时退出
set -e
# 打印执行的命令
set -v

has_qAsConst=(  'testcase.cpp'
                'exttestcaseupdaterdialog.cpp'
                'exttestcasetable.cpp'
                'exttestcasemodifier.cpp'
                'addtestcaseswizard.cpp'
                )

for i in "${has_qAsConst[@]}"; do
    sed -i 's/qAsConst\(([a-zA-Z]*)\)/\1/g' "$i"
done

sudo apt install qt5-default build-essential
g++ watcher_unix.cpp -o watcher_unix -O2
qmake lemon.pro
make
