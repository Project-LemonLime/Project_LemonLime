#!/bin/env bash

# 失败时退出
set -e
# 打印执行的命令
set -v


# Since Qt 5.14
has_Qt_SkipEmptyParts=(	'compiler.cpp'
						'detaildialog.cpp'
						'settings.cpp'
						)

for i in "${has_Qt_SkipEmptyParts[@]}"; do
    sed -i 's/Qt::SkipEmptyParts/QString::SkipEmptyParts/g' "$1"/src/"$i"
done
