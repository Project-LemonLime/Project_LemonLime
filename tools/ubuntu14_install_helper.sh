#!/bin/env bash

# 失败时退出
set -e
# 打印执行的命令
set -v

cd ..

# Since Qt 5.5
has_asprintf=(  'detaildialog.cpp'
                'exportutil.cpp'
                )

for i in "${has_asprintf[@]}"; do
    sed -i 's/asprintf/sprintf/g' "$i"
done

# Since Qt 5.4
sed -i '/<property name="tabBarAutoHide">/,+2d' forms/lemon.ui
# Since GCC 6.x compiled with option "--enable-default-pie"
sed -i '/unix:QMAKE_LFLAGS += -no-pie/d' lemon.pro

source ./ubuntu16_install_helper.sh
