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

#include <stdio.h>
#include <string.h>
#include "ht.h"

int
main(int argc, char **argv)
{
	const char key[16] = "0123456789abcdef"; /* secure as fuck */
	void *h = ht_create(argc, key);
	HT_VALUE *v = NULL;
	if (!h) return -1;
	for (int i = 1; i < argc; i++) {
		char *s = strchr(argv[i], '=');
		if (s) *s++ = 0;
		else continue;
		bool r = ht_insert(h, (void *)argv[i], strlen(argv[i])+1, (void *)s, strlen(s)+1);
		if (!r) fprintf(stderr, "insert failed %s=%s\n", argv[i], s);
		else fprintf(stderr, "insert worked %s=%s\n", argv[i], s);
	}
	for (int i = 1; i < argc; i++) {
		v = ht_retrieve(h, (void *)argv[i], strlen(argv[i])+1);
		if (v) fprintf(stderr, "retrieve worked %s=%s\n", argv[i], v->data);
		else fprintf(stderr, "retrieve failed %s\n", argv[i]);
	}
	if (ht_insert(h, "hello", 6, "world", 6))
		fprintf(stderr, "insert worked %s=%s\n", "hello", "world");
	v = ht_retrieve(h, "hello", 6);
	if (v) fprintf(stderr, "retrieve worked %s=%s\n", "hello", v->data);
	else fprintf(stderr, "retrieve failed %s\n", "hello");
	if (ht_update(h, "hello", 6, "dog", 4))
		fprintf(stderr, "update worked %s=%s\n", "hello", "dog");
	v = ht_retrieve(h, "hello", 6);
	if (v) fprintf(stderr, "retrieve worked %s=%s\n", "hello", v->data);
	else fprintf(stderr, "retrieve failed %s\n", "hello");
	if (ht_delete(h, "hello", 6)) fprintf(stderr, "delete worked %s\n", "hello");
	else fprintf(stderr, "delete failed %s\n", "hello");
	v = ht_retrieve(h, "hello", 6);
	if (v) fprintf(stderr, "retrieve worked %s=%s\n", "hello", v->data);
	else fprintf(stderr, "retrieve failed %s\n", "hello");

	return (ht_destroy(h) ? 0 : -1);
}
