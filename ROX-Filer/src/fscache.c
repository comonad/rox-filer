/*
 * $Id$
 *
 * FSCache - a glib-style object for caching files
 * Copyright (C) 1999, Thomas Leonard, <tal197@ecs.soton.ac.uk>.
 *
 * A cache object holds stat details about files in a hash table, along
 * with user-specified data. When you want to read in a file try to
 * get the data via the cache - if the file is cached AND has not been
 * modified since it was last loaded the cached copy is returned, else the
 * file is reloaded.
 *
 * The actual data need not be the raw file contents - a user specified
 * function loads the file and associates data with the file in the cache.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "fscache.h"


/* Static prototypes */

static guint hash_key(gconstpointer key);
static gint cmp_stats(gconstpointer a, gconstpointer b);
static void destroy_hash_entry(gpointer key, gpointer data, gpointer user_data);


/****************************************************************
 *			EXTERNAL INTERFACE			*
 ****************************************************************/


/* Create a new GFSCache object and return a pointer to it.
 *
 * When someone tries to lookup a file which is not in the cache or
 * which is out-of-date, load() is called.
 * It should load the file and return a pointer to an object for the file.
 * The object should have a ref count of 1.
 *
 * ref() and unref() should modify the reference counter of the object.
 * When the counter reaches zero, destroy the object.
 *
 * 'user_data' will be passed to all of the above functions.
 */
GFSCache *g_fscache_new(GFSLoadFunc load,
			GFSFunc ref,
			GFSFunc unref,
			gpointer user_data)
{
	GFSCache *cache;

	cache = g_new(GFSCache, 1);
	cache->inode_to_stats = g_hash_table_new(hash_key, cmp_stats);
	cache->load = load;
	cache->ref = ref;
	cache->unref = unref;
	cache->user_data = user_data;

	return cache;
}

void g_fscache_destroy(GFSCache *cache)
{
	g_return_if_fail(cache != NULL);

	g_hash_table_foreach(cache->inode_to_stats, destroy_hash_entry, cache);
	g_hash_table_destroy(cache->inode_to_stats);

	g_free(cache);
}

void g_fscache_data_ref(GFSCache *cache, gpointer data)
{
	g_return_if_fail(cache != NULL);

	if (cache->ref)
		cache->ref(data, cache->user_data);
}

void g_fscache_data_unref(GFSCache *cache, gpointer data)
{
	g_return_if_fail(cache != NULL);

	if (cache->unref)
		cache->unref(data, cache->user_data);
}

/* Find the data for this file in the cache, loading it into
 * the cache if it isn't there already.
 *
 * Remember to g_fscache_data_unref() the returned value when
 * you're done with it.
 *
 * Returns NULL on failure.
 */
gpointer g_fscache_lookup(GFSCache *cache, char *pathname)
{
	struct stat 	info;
	GFSCacheKey	key;
	GFSCacheData	*data;

	g_return_val_if_fail(cache != NULL, NULL);
	g_return_val_if_fail(pathname != NULL, NULL);

	if (stat(pathname, &info))
		return NULL;

	key.device = info.st_dev;
	key.inode = info.st_ino;

	data = g_hash_table_lookup(cache->inode_to_stats, &key);

	if (data)
	{
		/* We've cached this file already - is it up-to-date? */

		if (data->m_time == info.st_mtime
		 && data->length == info.st_size
		 && data->mode == info.st_mode)
		{
			if (cache->ref)
				cache->ref(data->data, cache->user_data);
			return data->data;
		}

		/* Out-of-date */
		if (cache->unref)
			cache->unref(data->data, cache->user_data);
		data->data = NULL;
	}
	else
	{
		GFSCacheKey *new_key;
		
		new_key = g_memdup(&key, sizeof(key));

		data = g_new(GFSCacheData, 1);
		data->data = NULL;

		g_hash_table_insert(cache->inode_to_stats, new_key, data);
	}

	data->m_time = info.st_mtime;
	data->length = info.st_size;
	data->mode = info.st_mode;

	/* data->data is NULL - create the object for the file */

	if (cache->load)
		data->data = cache->load(pathname, cache->user_data);

	if (cache->ref)
		cache->ref(data->data, cache->user_data);

	return data->data;
}


/****************************************************************
 *			INTERNAL FUNCTIONS			*
 ****************************************************************/


/* Generate a hash number for some stats */
static guint hash_key(gconstpointer key)
{
	GFSCacheKey *stats = (GFSCacheKey *) key;
	
	return stats->inode;
}

/* See if two stats blocks represent the same file */
static gint cmp_stats(gconstpointer a, gconstpointer b)
{
	GFSCacheKey *c = (GFSCacheKey *) a;
	GFSCacheKey *d = (GFSCacheKey *) b;

	return c->device == d->device && c->inode == d->inode;
}

static void destroy_hash_entry(gpointer key, gpointer data, gpointer user_data)
{
	GFSCache *cache = (GFSCache *) user_data;
	GFSCacheData *cache_data = (GFSCacheData *) data;
	
	if (cache->unref)
		cache->unref(cache_data->data, cache->user_data);

	g_free(key);
	g_free(data);
}
