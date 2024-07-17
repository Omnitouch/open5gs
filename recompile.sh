#!/bin/bash

# Ensure we can get core dumps
ulimit -c unlimited
export ASAN_OPTIONS=abort_on_error=1:disable_coredump=0:unmap_shadow_on_exit=1

meson build --prefix=`pwd`/install
ninja -C build install

# After building we store a copy of these executables so that when we get a
# coredump we can use it after future recompiles
exe_folder="/var/log/open5gs/exe_$(date +%Y_%m_%d_%H%M%S)"
mkdir $exe_folder
cp /usr/src/open5gs/build/src/mme/open5gs-mmed $exe_folder
cp /usr/src/open5gs/build/src/sgwc/open5gs-sgwcd $exe_folder
cp /usr/src/open5gs/build/src/sgwu/open5gs-sgwud $exe_folder
cp /usr/src/open5gs/build/src/smf/open5gs-smfd $exe_folder
cp /usr/src/open5gs/build/src/upf/open5gs-upfd $exe_folder

systemctl restart 'open5gs*'
