#ifndef __FS_MEM_H__
#define __FS_MEM_H__

#include <stddef.h>

#define FS_NEW(_Type) (_Type*)fs_mem_zalloc(sizeof(_Type))
#define FS_NEW_ARRAY(_Type, _Count) (_Type*)fs_mem_zalloc(sizeof(_Type) * _Count)

void*		fs_mem_zalloc(
				size_t					aSize);

#endif 