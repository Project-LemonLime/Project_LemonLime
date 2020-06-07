#!/bin/env bash

# 失败时退出
set -e
# 打印执行的命令
set -v

cd "$1"

# Since Qt 5.5
has_asprintf=(  'detaildialog.cpp'
                'exportutil.cpp'
                )

for i in "${has_asprintf[@]}"; do
    sed -i 's/asprintf/sprintf/g' "$i"
done
