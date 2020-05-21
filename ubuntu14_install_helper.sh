#!/bin/env bash

# 失败时退出
set -e
# 打印执行的命令
set -v

has_asprintf=(  'detaildialog.cpp'
                'exportutil.cpp'
                )

for i in "${has_asprintf[@]}"; do
    sed -i 's/asprintf/sprintf/g' "$i"
done

sed -i '/<property name="tabBarAutoHide">/,+2d' forms/lemon.ui
sed -i '/unix:QMAKE_LFLAGS += -no-pie/d' lemon.pro

source ./ubuntu16.sh
