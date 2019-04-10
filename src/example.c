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

#include <stdio.h>

#include "xmpool.h"

int main()
{
	xmpool_t *mpool = xmpool_new(XMPOOL_DEFAULT_SIZE);
	if (mpool == NULL) {
		fprintf(stderr, "error\n");
		return 1;
	}

	char *msg = (char *)xmpool_palloc(mpool, 1024);

	memset(msg, '\0', 1024);

	memcpy(msg, "Hello", 5);

	fprintf(stdout, "%s\n", msg);

	xmpool_free(mpool, msg);

	xmpool_delete(mpool);

	return 0;
}