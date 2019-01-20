#!/bin/sh

find                                    \
    "${MESON_SOURCE_ROOT}/src"          \
    "${MESON_SOURCE_ROOT}/include/unet" \
    "${MESON_SOURCE_ROOT}/test"         \
    "${MESON_SOURCE_ROOT}/bench"        \
    "${MESON_SOURCE_ROOT}/examples"     \
    -iname '*.cpp' -o -iname '*.hpp' | xargs clang-format -i -style=file

