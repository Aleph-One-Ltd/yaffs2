/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "yaffs_cache.h"

/*------------------------ Short Operations Cache ------------------------------
 *   In many situations where there is no high level buffering  a lot of
 *   reads might be short sequential reads, and a lot of writes may be short
 *   sequential writes. eg. scanning/writing a jpeg file.
 *   In these cases, a short read/write cache can provide a huge perfomance
 *   benefit with dumb-as-a-rock code.
 *   In Linux, the page cache provides read buffering and the short op cache
 *   provides write buffering.
 *
 *   There are a small number (~10) of cache chunks per device so that we don't
 *   need a very intelligent search.
 */

int yaffs_obj_cache_dirty(struct yaffs_obj *obj)
{
	struct yaffs_dev *dev = obj->my_dev;
	int i;
	struct yaffs_cache *cache;
	int n_caches = obj->my_dev->param.n_caches;

	for (i = 0; i < n_caches; i++) {
		cache = &dev->cache[i];
		if (cache->object == obj && cache->dirty)
			return 1;
	}

	return 0;
}

void yaffs_flush_single_cache(struct yaffs_cache *cache, int discard)
{

	if (!cache || cache->locked)
		return;

	/* Write it out and free it up  if need be.*/
	if (cache->dirty) {
		yaffs_wr_data_obj(cache->object,
				  cache->chunk_id,
				  cache->data,
				  cache->n_bytes,
				  1);

		cache->dirty = 0;
	}

	if (discard)
		cache->object = NULL;
}

void yaffs_flush_file_cache(struct yaffs_obj *obj, int discard)
{
	struct yaffs_dev *dev = obj->my_dev;
	int i;
	struct yaffs_cache *cache;
	int n_caches = obj->my_dev->param.n_caches;

	if (n_caches < 1)
		return;


	/* Find the chunks for this object and flush them. */
	for (i = 0; i < n_caches; i++) {
		cache = &dev->cache[i];
		if (cache->object == obj)
			yaffs_flush_single_cache(cache, discard);
	}

}


void yaffs_flush_whole_cache(struct yaffs_dev *dev, int discard)
{
	struct yaffs_obj *obj;
	int n_caches = dev->param.n_caches;
	int i;

	/* Find a dirty object in the cache and flush it...
	 * until there are no further dirty objects.
	 */
	do {
		obj = NULL;
		for (i = 0; i < n_caches && !obj; i++) {
			if (dev->cache[i].object && dev->cache[i].dirty)
				obj = dev->cache[i].object;
		}
		if (obj)
			yaffs_flush_file_cache(obj, discard);
	} while (obj);

}

/* Grab us an unused cache chunk for use.
 * First look for an empty one.
 * Then look for the least recently used non-dirty one.
 * Then look for the least recently used dirty one...., flush and look again.
 */
static struct yaffs_cache *yaffs_grab_chunk_worker(struct yaffs_dev *dev)
{
	u32 i;

	if (dev->param.n_caches > 0) {
		for (i = 0; i < dev->param.n_caches; i++) {
			if (!dev->cache[i].object)
				return &dev->cache[i];
		}
	}

	return NULL;
}

struct yaffs_cache *yaffs_grab_chunk_cache(struct yaffs_dev *dev)
{
	struct yaffs_cache *cache;
	int usage;
	u32 i;

	if (dev->param.n_caches < 1)
		return NULL;

	/* First look for an unused cache */

	cache = yaffs_grab_chunk_worker(dev);

	if (cache)
		return cache;

	/*
	 * Thery were all in use.
	 * Find the LRU cache and flush it if it is dirty.
	 */

	usage = -1;
	cache = NULL;

	for (i = 0; i < dev->param.n_caches; i++) {
		if (dev->cache[i].object &&
		    !dev->cache[i].locked &&
		    (dev->cache[i].last_use < usage || !cache)) {
				usage = dev->cache[i].last_use;
				cache = &dev->cache[i];
		}
	}

#if 1
	yaffs_flush_single_cache(cache, 1);
#else
	yaffs_flush_file_cache(cache->object, 1);
	cache = yaffs_grab_chunk_worker(dev);
#endif

	return cache;
}

/* Find a cached chunk */
struct yaffs_cache *yaffs_find_chunk_cache(const struct yaffs_obj *obj,
						  int chunk_id)
{
	struct yaffs_dev *dev = obj->my_dev;
	u32 i;

	if (dev->param.n_caches < 1)
		return NULL;

	for (i = 0; i < dev->param.n_caches; i++) {
		if (dev->cache[i].object == obj &&
		    dev->cache[i].chunk_id == chunk_id) {
			dev->cache_hits++;

			return &dev->cache[i];
		}
	}
	return NULL;
}

/* Mark the chunk for the least recently used algorithym */
void yaffs_use_cache(struct yaffs_dev *dev, struct yaffs_cache *cache,
			    int is_write)
{
	u32 i;

	if (dev->param.n_caches < 1)
		return;

	if (dev->cache_last_use < 0 ||
		dev->cache_last_use > 100000000) {
		/* Reset the cache usages */
		for (i = 1; i < dev->param.n_caches; i++)
			dev->cache[i].last_use = 0;

		dev->cache_last_use = 0;
	}
	dev->cache_last_use++;
	cache->last_use = dev->cache_last_use;

	if (is_write)
		cache->dirty = 1;
}

/* Invalidate a single cache page.
 * Do this when a whole page gets written,
 * ie the short cache for this page is no longer valid.
 */
void yaffs_invalidate_chunk_cache(struct yaffs_obj *object, int chunk_id)
{
	struct yaffs_cache *cache;

	if (object->my_dev->param.n_caches > 0) {
		cache = yaffs_find_chunk_cache(object, chunk_id);

		if (cache)
			cache->object = NULL;
	}
}

/* Invalidate all the cache pages associated with this object
 * Do this whenever ther file is deleted or resized.
 */
void yaffs_invalidate_file_cache(struct yaffs_obj *in)
{
	u32 i;
	struct yaffs_dev *dev = in->my_dev;

	if (dev->param.n_caches > 0) {
		/* Invalidate it. */
		for (i = 0; i < dev->param.n_caches; i++) {
			if (dev->cache[i].object == in)
				dev->cache[i].object = NULL;
		}
	}
}


int yaffs_cache_init(struct yaffs_dev *dev)
{
	int init_failed;

	if (dev->param.n_caches > 0) {
		u32 i;
		void *buf;
		u32 cache_bytes =
		    dev->param.n_caches * sizeof(struct yaffs_cache);

		if (dev->param.n_caches > YAFFS_MAX_SHORT_OP_CACHES)
			dev->param.n_caches = YAFFS_MAX_SHORT_OP_CACHES;

		dev->cache = kmalloc(cache_bytes, GFP_NOFS);

		buf = (u8 *) dev->cache;

		if (dev->cache)
			memset(dev->cache, 0, cache_bytes);

		for (i = 0; i < dev->param.n_caches && buf; i++) {
			dev->cache[i].object = NULL;
			dev->cache[i].last_use = 0;
			dev->cache[i].dirty = 0;
			dev->cache[i].data = buf =
			    kmalloc(dev->param.total_bytes_per_chunk, GFP_NOFS);
		}
		if (!buf)
			init_failed = 1;

		dev->cache_last_use = 0;
	}

	return init_failed ? -1 : 0;
}

void yaffs_cache_deinit(struct yaffs_dev *dev)
{

	if (dev->param.n_caches > 0 && dev->cache) {
		u32 i;

		for (i = 0; i < dev->param.n_caches; i++) {
			kfree(dev->cache[i].data);
			dev->cache[i].data = NULL;
		}

		kfree(dev->cache);
		dev->cache = NULL;
	}
}
