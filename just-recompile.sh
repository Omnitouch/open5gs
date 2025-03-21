#!/bin/bash

# Ensure we can get core dumps
ulimit -c unlimited
export ASAN_OPTIONS=abort_on_error=1:disable_coredump=0:unmap_shadow_on_exit=1:detect_odr_violation=0

meson build --prefix=`pwd`/install
ninja -C build install
