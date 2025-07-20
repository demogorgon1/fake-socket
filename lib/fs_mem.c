#include <fs/fs_base.h>

#include "fs_mem.h"

void* 
fs_mem_zalloc(
	size_t					aSize)
{
	void* p = malloc(aSize);
	if (p == NULL)
		abort(); // Don't want to bother with handling malloc() fails, just making everything more complex for no reason

	memset(p, 0, aSize);
	return p;
}
