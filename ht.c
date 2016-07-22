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
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <error.h>
#include <string.h>
#include "siphash.h" /* https://github.com/floodyberry/siphash */
#include "ht.h"

struct HT_ENTRY_PROTO {
	uint64_t hkey;
	void *key;
	size_t key_sz;
	HT_VALUE *value;
	struct HT_ENTRY_PROTO *next;
	struct HT_ENTRY_PROTO *prev;
};

typedef struct HT_ENTRY_PROTO HT_ENTRY;

typedef struct {
	size_t num_buckets;
	unsigned char key[16];
	HT_ENTRY **bucket;
} HT;

static void
ht_free_e(HT_ENTRY *e)
{
	if (e) {
		if (e->value) {
			if (e->value->data) { free(e->value->data); e->value->data = NULL; }
			free(e->value); e->value = NULL;
		}
		if (e->key) { free(e->key); e->key = NULL; }
		free(e); e = NULL;
	}
}

static HT_ENTRY *
ht_retrieve_e(void *_h, const void *key, size_t key_sz)
{
	HT *h = (HT *)_h;
	HT_ENTRY *root = NULL;
	uint64_t hkey = siphash(h->key, key, key_sz) % h->num_buckets;
	for (root = h->bucket[hkey]; root; root = root->next) {
		if (root->key_sz == key_sz) {
			if (!memcmp(root->key, key, root->key_sz))
				return root;
		}
	}
	return NULL;
}

void *
ht_create(size_t buckets, const unsigned char *key)
{
	if (!key || !buckets) {
		warnx("ht_create: invalid input");
		goto fail;
	}
	HT *h = calloc(1, sizeof(HT));
	if (!h) {
		warn("ht_create: calloc: %lu bytes", sizeof(HT));
		goto fail;
	}
	h->num_buckets = buckets;
	h->bucket = calloc(h->num_buckets, sizeof(HT_ENTRY *));
	if (!h->bucket) {
		warn("ht_create: calloc: %lu bytes", h->num_buckets * sizeof(HT_ENTRY *));
		goto fail;
	}
	return (void *)h;
fail:
	if (h) {
		if (h->bucket) { free(h->bucket); h->bucket = NULL; }
		free(h); h = NULL;
	}
	return NULL;
}

bool
ht_destroy(void *_h)
{
	HT *h = (HT *)_h;
	if (!h) {
		warnx("ht_destroy: invalid input");
		goto fail;
	}
	for (size_t b = 0; b < h->num_buckets; b++) {
		HT_ENTRY *e = h->bucket[b], *t = NULL;
		while (e) {
			t = e->next;
			e->next = NULL;
			e->prev = NULL;
			ht_free_e(e);
			e = t;
		}
	}
	if (h->bucket) { free(h->bucket); h->bucket = NULL; }
	if (h) { free(h); h = NULL; }
	return true;
fail:
	return false;
}

bool
ht_insert(void *_h, const void *key, size_t key_sz, const void *value, size_t value_sz)
{
	HT *h = (HT *)_h;
	HT_ENTRY *e = NULL, *root = NULL;
	uint64_t hkey;
	if (!h || !key || key_sz == 0 || !value || value_sz == 0) {
		warnx("ht_insert: invalid input");
		goto fail;
	}
	if (ht_retrieve_e(_h, key, key_sz)) {
		warnx("ht_insert: duplicate entry");
		goto fail;
	}
	e = calloc(1, sizeof(HT_ENTRY));
	if (!e) {
		warn("ht_insert: calloc: %lu bytes", sizeof(HT_ENTRY));
		goto fail;
	}
	e->value = calloc(1, sizeof(HT_VALUE));
	if (!e->value) {
		warn("ht_insert: calloc: %lu bytes", sizeof(HT_VALUE));
		goto fail;
	}
	e->value->data = calloc(1, value_sz);
	if (!e->value->data) {
		warn("ht_insert: calloc: %lu bytes", value_sz);
		goto fail;
	}
	e->key = calloc(1, key_sz);
	if (!e->key) {
		warn("ht_insert: calloc: %lu bytes", key_sz);
		goto fail;
	}
	hkey = siphash(h->key, key, key_sz) % h->num_buckets;
	e->hkey = hkey;
	memcpy(e->value->data, value, value_sz);
	e->value->sz = value_sz;
	memcpy(e->key, key, key_sz);
	e->key_sz = key_sz;
	root = h->bucket[hkey];
	if (root) {
		while (root->next) root = root->next;
		root->next = e;
		e->prev = root;
	} else h->bucket[hkey] = e;
	return true;
fail:
	if (e) {
		if (e->value) {
			if (e->value->data) { free(e->value->data); e->value->data = NULL; }
			free(e->value); e->value = NULL;
		}
		if (e->key) { free(e->key); e->key = NULL; }
		free(e); e = NULL;
	}
	return false;
}

HT_VALUE *
ht_retrieve(void *_h, const void *key, size_t key_sz)
{
	HT_ENTRY *e = NULL;
	if (!_h || !key || !key_sz) {
		warnx("ht_retrieve: invalid input");
		goto fail;
	}
	e = ht_retrieve_e(_h, key, key_sz);
	if (e) return e->value;
fail:
	return NULL;
}

bool
ht_delete(void *_h, const void *key, size_t key_sz)
{
	HT *h = (HT *)_h;
	HT_ENTRY *e = NULL, *t;
	if (!_h || !key || !key_sz) {
		warnx("ht_delete: invalid input");
		goto fail;
	}
	e = ht_retrieve_e(_h, key, key_sz);
	if (!e) goto fail;
	t = e;
	if (e->next) e->next->prev = e->prev;
	if (e->prev) e->prev->next = e->next;
	else {
		h->bucket[e->hkey] = e->next;
	}
	ht_free_e(t);
	return true;
fail:
	return false;
}

bool
ht_update(void *_h, const void *key, size_t key_sz, const void *value, size_t value_sz)
{
	HT *h = (HT *)_h;
	HT_ENTRY *e = NULL;
	if (!h || !key || !key_sz || !value || !value_sz) {
		warnx("ht_update: invalid input");
		goto fail;
	}
	e = ht_retrieve_e(h, key, key_sz);
	if (!e) {
		warnx("ht_update: no such key");
		goto fail;
	}
	if (e->value) {
		if (e->value->data) { free(e->value->data); e->value->data = NULL; }
		free(e->value); e->value = NULL;
	}
	e->value = calloc(1, sizeof(HT_VALUE));
	if (!e->value) {
		warn("ht_update: calloc: %lu bytes", sizeof(HT_VALUE));
		goto fail;
	}
	e->value->data = calloc(1, value_sz);
	if (!e->value->data) {
		warn("ht_update: calloc: %lu bytes", value_sz);
		goto fail;
	}
	e->value->sz = value_sz;
	memcpy(e->value->data, value, e->value->sz);
	return true;
fail:
	return false;
}
