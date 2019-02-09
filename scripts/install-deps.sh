#!/bin/sh

sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
    arping                                             \
    build-essential                                    \
    clang-format                                       \
    cmake                                              \
    ninja-build                                        \
    python3                                            \
    python3-pip                                        \
    python3-setuptools                                 \
    tshark                                             \
    tmux                                               \
    unzip                                              \
    wget

sudo pip3 install meson

cd /tmp &&                                                                         \
    wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.zip && \
    unzip -q boost_1_69_0.zip &&                                                   \
    cd boost_1_69_0 &&                                                             \
    ./bootstrap.sh --with-libraries=system &&                                      \
    sudo ./b2 install -d0 -j $(nproc)

cd /tmp &&                                                                 \
    wget https://github.com/gflags/gflags/archive/v2.2.2.zip &&            \
    unzip -q v2.2.2.zip &&                                                 \
    cd gflags-2.2.2 &&                                                     \
    mkdir build &&                                                         \
    cd build &&                                                            \
    cmake -DCMAKE_BUILD_TYPE=RELEASE .. &&                                 \
    sudo make -j $(nproc) install

cd /tmp &&                                                                 \
    wget https://github.com/abseil/googletest/archive/release-1.8.1.zip && \
    unzip -q release-1.8.1.zip &&                                          \
    cd googletest-release-1.8.1 &&                                         \
    mkdir build &&                                                         \
    cd build &&                                                            \
    cmake -DCMAKE_BUILD_TYPE=RELEASE .. &&                                 \
    sudo make -j $(nproc) install

cd /tmp &&                                                                 \
    wget https://github.com/google/benchmark/archive/v1.4.1.zip &&         \
    unzip -q v1.4.1.zip &&                                                 \
    cd benchmark-1.4.1 &&                                                  \
    mkdir build &&                                                         \
    cd build &&                                                            \
    cmake -DCMAKE_BUILD_TYPE=RELEASE .. &&                                 \
    sudo make -j $(nproc) install
