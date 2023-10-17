# Copyright (C) 2023 by Ryan Dimsey <ryan@omnitouch.com.au>

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

redis_sources = files('''
    ogs-redis.h

    ogs-redis.c
'''.split())

libhiredis_dep = cc.find_library('hiredis', dirs: '/usr/include/hiredis', required: true)
libhiredis_inc = include_directories('/usr/include/hiredis')

redis_inc = include_directories('.')

libredis = library('ogsredis',
    sources : redis_sources,
    version : libogslib_version,
    c_args : '-DOGS_S1AP_COMPILATION',
    include_directories : [redis_inc, libhiredis_inc, libinc],
    dependencies : [libhiredis_dep, libapp_dep],
    install : true)

libredis_dep = declare_dependency(
    link_with : libredis,
    include_directories : [redis_inc, libhiredis_inc, libinc],
    dependencies : [libhiredis_dep, libapp_dep])
