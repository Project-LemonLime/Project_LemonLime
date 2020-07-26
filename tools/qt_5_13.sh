#!/bin/env bash

# 失败时退出
set -e
# 打印执行的命令
set -v


# Since Qt 5.13
has_swapItemsAt=(  'contest.cpp'
                   'contestant.cpp'
                   )

for i in "${has_swapItemsAt[@]}"; do
    sed -i 's/swapItemsAt/swap/g' "$1"/src/"$i"
    sed -i 's/Qt::endl/swap/g' "$1"/src/"$i"
done

# Since Qt 5.13
sed -i 's/Qt::endl/swap/g' "$1"/src/exportutil.cpp

