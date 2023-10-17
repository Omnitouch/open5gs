/*
 * Copyright (C) 2023 by Ryan Dimsey <ryan@omnitouch.com.au>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef REGEX_EXTENSION_H
#define REGEX_EXTENSION_H


#include <regex.h>
#include <stdbool.h>


/*
 * Checks if pattern will match against string.
 * Returns true on match, false otherwise.
 */
bool reg_match(char const *pattern, char const *string);


/*
 * Match pattern against string and apply `replacement`
 * on `string` if match succeeds. Returns 1 on success.
 */
int reg_replace(char *pattern, char *replacement, char *string, char *buf, size_t buf_sz);


#endif /* REGEX_EXTENSION_H */