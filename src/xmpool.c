/**
 * Nginx
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 *
 * The Nginx License
*/

/**
 * X-Memory-Pool
 *
 * Copyright (C) Ali Bakhtiar
 *
 * The Nginx License
 *
 * @modified : 21 January 2019
 * @created  : 03 July 2018
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "xmpool.h"
#include <stdlib.h>

static void *xmpool_palloc_block(xmpool_t *pool, size_t size);
static void *xmpool_palloc_large(xmpool_t *pool, size_t size);


/**
 * New
*/
xmpool_t *xmpool_new(size_t size)
{
	xmpool_t  *p;

	p = (xmpool_t  *)xmpool_memalign(XMPOOL_ALIGNMENT_SIZE, size);
	if (p == NULL)
		return NULL;

	p->d.last = (u_char *) p + sizeof(xmpool_t);
	p->d.end  = (u_char *) p + size;
	p->d.next = NULL;
	p->d.failed = 0;

	size = size - sizeof(xmpool_t);
	p->max = (size < XMPOOL_MAX_ALLOC_FROM_POOL) ? size : XMPOOL_MAX_ALLOC_FROM_POOL;
	p->size = size;

	p->current = p;
	p->chain = NULL;
	p->large = NULL;
	p->cleanup = NULL;

	return p;
}


/**
 * Delete (destroy)
*/
void xmpool_delete(xmpool_t *pool)
{
	xmpool_t          *p, *n;
	xmpool_large_t    *l;
	xmpool_cleanup_t  *c;

	for (c = pool->cleanup; c; c = c->next) {
		if (c->handler)
			c->handler(c->data);
	}

	for (l = pool->large; l; l = l->next) {
		if (l->alloc)
			free(l->alloc);
	}

	for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
		free(p);

		if (n == NULL)
			break;
	}

	return;
}


/**
 * Free
*/
int xmpool_free(xmpool_t *pool, void *p)
{
	xmpool_large_t *l;

	for (l = pool->large; l; l = l->next) {
		if (p == l->alloc) {
			free(l->alloc);
			l->alloc = NULL;
			return 1;
		}
	}

	return 0;
}


/**
 * Calloc
*/
void *xmpool_calloc(size_t size)
{
	void  *p;
	p = xmpool_malloc(size);
	if (p)
		xmpool_memzero(p, size);

	return p;
}


/**
 * Palloc
*/
void *xmpool_palloc(xmpool_t *pool, size_t size)
{
	u_char    *m;
	xmpool_t  *p;

	if (size <= pool->max) {
		p = pool->current;

		do {
			m = xmpool_align_ptr(p->d.last, XMPOOL_ALIGNMENT);

			if ((size_t) (p->d.end - m) >= size) {
				p->d.last = m + size;
				return m;
			}

			p = p->d.next;

		} while (p);

		return xmpool_palloc_block(pool, size);
	}

	return xmpool_palloc_large(pool, size);
}


/**
 * Palloc block
*/
static void *xmpool_palloc_block(xmpool_t *pool, size_t size)
{
	u_char    *m;
	size_t    psize;
	xmpool_t *p, *newp, *current;

	psize = (size_t) (pool->d.end - (u_char *) pool);

	m = (u_char  *)xmpool_memalign(XMPOOL_ALIGNMENT_SIZE, psize);
	if (m == NULL)
		return NULL;

	newp = (xmpool_t *) m;

	newp->d.end = m + psize;
	newp->d.next = NULL;
	newp->d.failed = 0;

	m += sizeof(xmpool_data_t);
	m = xmpool_align_ptr(m, XMPOOL_ALIGNMENT);
	newp->d.last = m + size;

	current = pool->current;

	for (p = current; p->d.next; p = p->d.next) {
		if (p->d.failed++ > 4)
			current = p->d.next;
	}

	p->d.next = newp;

	pool->current = current ? current : newp;

	return m;
}


/**
 * Palloc large
*/
static void *xmpool_palloc_large(xmpool_t *pool, size_t size)
{
	void            *p;
	xmpool_uint_t    n;
	xmpool_large_t  *large;

	p = xmpool_malloc(size);
	if (p == NULL)
		return NULL;

	n = 0;

	for (large = pool->large; large; large = large->next) {
		if (large->alloc == NULL) {
			large->alloc = p;
			return p;
		}

		if (n++ > 3)
			break;
	}

	large = (xmpool_large_t *)xmpool_palloc(pool, sizeof(xmpool_large_t));
	if (large == NULL) {
		free(p);
		return NULL;
	}

	large->alloc = p;
	large->next = pool->large;
	pool->large = large;

	return p;
}


/**
 * Cleanup
*/
xmpool_cleanup_t *xmpool_cleanup_add(xmpool_t *p, size_t size)
{
	xmpool_cleanup_t *c;

	c = (xmpool_cleanup_t *)xmpool_palloc(p, sizeof(xmpool_cleanup_t));
	if (c == NULL)
		return NULL;

	if (size) {
		c->data = xmpool_palloc(p, size);
		if (c->data == NULL)
			return NULL;
	}
	else {
		c->data = NULL;
	}

	c->handler = NULL;
	c->next = p->cleanup;

	p->cleanup = c;

	return c;
}


#ifdef __cplusplus
}
#endif