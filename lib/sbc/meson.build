# Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>

# This file is part of Open5GS.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

libsbc_sources = files('''
    ogs-sbc.h

    sbc-types.h
    message.h 
    build.h 

    message.c
'''.split())

libsbc_inc = include_directories('.')

libsbc = library('ogssbc',
    sources : libsbc_sources,
    version : libogslib_version,
    c_args : '-DOGS_SBC_COMPILATION',
    include_directories : [libsbc_inc, libinc],
    dependencies : libasn1c_s1ap_dep,
    install : true)

libsbc_dep = declare_dependency(
    link_with : libsbc,
    include_directories : [libsbc_inc, libinc],
    dependencies : libasn1c_s1ap_dep)
