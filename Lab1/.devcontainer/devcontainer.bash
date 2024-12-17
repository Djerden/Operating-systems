#!/bin/bash

set -e

cd "$(dirname "$0")"

apt update
apt install -y \
    wget \
    cmake \
    git \
    clang \
    clangd \
    clang-tidy \
    clang-format \
    lldb

wget https://github.com/Kitware/CMake/releases/download/v3.28.0/cmake-3.28.0-linux-x86_64.tar.gz &&
    tar -zxvf cmake-3.28.0-linux-x86_64.tar.gz --strip-components=1 -C /usr/local &&
    rm cmake-3.28.0-linux-x86_64.tar.gz