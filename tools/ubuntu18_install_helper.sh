#!/bin/env bash

# 失败时退出
set -e
# 打印执行的命令
set -v

cd ../src

# Since Qt 5.13
has_swapItemsAt=(  'contest.cpp'
                   'contestant.cpp'
                   )

for i in "${has_swapItemsAt[@]}"; do
    sed -i 's/swapItemsAt/swap/g' "$i"
done


# Since Qt 5.14
has_Qt_SkipEmptyParts=(	'compiler.cpp'
						'detaildialog.cpp'
						'settings.cpp'
						)

for i in "${has_Qt_SkipEmptyParts[@]}"; do
    sed -i 's/Qt::SkipEmptyParts/QString::SkipEmptyParts/g' "$i"
done

cd ..

sudo apt install qt5-default build-essential
g++ watcher_unix.cpp -o watcher_unix -O2
cmake .
make
