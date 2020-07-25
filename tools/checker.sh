#!/bin/env bash

# 失败时退出
set -e
# 打印执行的命令
set -v

qt_ver=$(pkg-config --modversion Qt5Core)

function version_gt() { test "$(echo "$@" | tr " " "\n" | sort -V | head -n 1)" != "$1"; }
function version_le() { test "$(echo "$@" | tr " " "\n" | sort -V | head -n 1)" == "$1"; }
function version_lt() { test "$(echo "$@" | tr " " "\n" | sort -rV | head -n 1)" != "$1"; }
function version_ge() { test "$(echo "$@" | tr " " "\n" | sort -rV | head -n 1)" == "$1"; }

version_lt "$qt_ver" 5.4 && ./qt_5_4.sh ..
version_lt "$qt_ver" 5.5 && ./qt_5_5.sh ..
version_lt "$qt_ver" 5.7 && ./qt_5_7.sh ..
version_lt "$qt_ver" 5.13 && ./qt_5_13.sh ..
version_lt "$qt_ver" 5.14 && ./qt_5_14.sh ..
