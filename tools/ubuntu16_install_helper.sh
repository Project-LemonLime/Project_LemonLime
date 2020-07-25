#!/bin/env bash

# 失败时退出
set -e
# 打印执行的命令
set -v

cd ..

# Since Qt 5.7
has_qAsConst=(  'testcase.cpp'
                'exttestcaseupdaterdialog.cpp'
                'exttestcasetable.cpp'
                'exttestcasemodifier.cpp'
                'addtestcaseswizard.cpp'
                )

for i in "${has_qAsConst[@]}"; do
    sed -i 's/qAsConst\(([a-zA-Z]*)\)/\1/g' "$i"
done

source ./ubuntu18_install_helper.sh
