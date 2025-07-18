#ifndef __TEST_H__
#define __TEST_H__

#include <fs/fs_base.h>

#define TEST_ASSERT(_X)															\
	do																			\
	{																			\
		if(!(_X))																\
		{																		\
			fprintf(stderr, "FAILED: %s:%u: %s\n", __FILE__, __LINE__, #_X);	\
			abort();															\
		}																		\
	} while(0)

#endif