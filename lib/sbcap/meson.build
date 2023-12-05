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

libsbcap_sources = files('''
    ogs-sbcap.h

    conv.h
    message.h 
    build.h

    conv.c 
    message.c
    build.c
'''.split())

libsbcap_inc = include_directories('.')

libsbcap = library('ogssbcap',
    sources : libsbcap_sources,
    version : libogslib_version,
    c_args : '-DOGS_S1AP_COMPILATION',
    include_directories : [libsbcap_inc, libinc],
    dependencies : libasn1c_sbcap_dep,
    install : true)

libsbcap_dep = declare_dependency(
    link_with : libsbcap,
    include_directories : [libsbcap_inc, libinc],
    dependencies : libasn1c_sbcap_dep)
