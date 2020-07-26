#!/bin/env bash

# 失败时退出
set -e
# 打印执行的命令
set -v


# Since Qt 5.4
sed -i '/<property name="tabBarAutoHide">/,+2d' "$1"/src/forms/lemon.ui
