/*
RASTERIZER_MEMORY_POOL.C
*/

/* ---------- headers */

#include "cseries.h"
#include "errors.h"
#include "real_math.h"
#include "rasterizer.h"
/* Empty reconstructed header removed; no declarations to include. */

/* ---------- globals */

/* swapped? */
static void *global_memory_pool;
static long global_memory_index;

/* ---------- public code */

boolean rasterizer_memory_pool_initialize(
	void)
{
	boolean success = TRUE;
	global_memory_pool = match_malloc("c:\\halo\\SOURCE\\rasterizer\\rasterizer_memory_pool.c", 19, RASTERIZER_MEMORY_POOL_SIZE);

	if (!global_memory_pool)
	{
		error(_error_silent, "### ERROR rasterizer failed to allocate global memory pool");
		success = FALSE;
	}

	return success;
}

void rasterizer_memory_pool_begin(
	void)
{
	global_memory_index = 0;

	return;
}

void *rasterizer_memory_alloc(
	const void *src,
	unsigned long size)
{
	void *pointer = NULL;

	if (global_memory_index+size <= RASTERIZER_MEMORY_POOL_SIZE)
	{
		pointer = (byte *)global_memory_pool+global_memory_index;
		global_memory_index+= size;

		if (src)
		{
			memcpy(pointer, src, size);
		}
	}
	else
	{
		error(_error_silent, "### ERROR rasterizer memory pool exceeded");
	}

	return pointer;
}

const void *rasterizer_memory_alloc_const(
	const void *src,
	unsigned long size)
{
	match_assert("c:\\halo\\SOURCE\\rasterizer\\rasterizer_memory_pool.c", 66, src);
	return rasterizer_memory_alloc(src, size);
}

void rasterizer_memory_pool_end(
	void)
{
	return;
}

void rasterizer_memory_pool_dispose(
	void)
{
	if (global_memory_pool!=NULL)
	{
		match_free("c:\\halo\\SOURCE\\rasterizer\\rasterizer_memory_pool.c", 80, global_memory_pool);
	}

	global_memory_pool = NULL;
	global_memory_index = 0;

	return;
}
