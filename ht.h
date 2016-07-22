/*
    Copyright (C) 2016 cacahuatl < cacahuatl at autistici dot org >

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _HT_H
#define _HT_H

#include <stdbool.h>

typedef struct {
	void *data;
	size_t sz;
} HT_VALUE;

void *
ht_create(size_t buckets, const unsigned char *key);

bool
ht_destroy(void *_h);

bool
ht_insert(void *_h, const void *key, size_t key_sz, const void *value, size_t value_sz);

HT_VALUE *
ht_retrieve(void *_h, const void *key, size_t key_sz);

bool
ht_delete(void *_h, const void *key, size_t key_sz);

bool
ht_update(void *_h, const void *key, size_t key_sz, const void *value, size_t value_sz);

#endif
