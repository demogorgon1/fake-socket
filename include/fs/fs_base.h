#ifndef __FS_BASE_H__
#define __FS_BASE_H__

#if defined(WIN32)
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>	
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <malloc.h>

#define FS_TRUE							1
#define FS_FALSE						0

typedef int fs_bool;

#ifndef FS_MAX_SOCKETS
#define FS_MAX_SOCKETS					1024
#endif 

#ifndef FS_MAX_LISTEN_SOCKETS
#define FS_MAX_LISTEN_SOCKETS			256
#endif

#ifndef FS_MAX_BACKLOG
#define FS_MAX_BACKLOG					32
#endif

#ifndef FS_EPHEMERAL_PORT_RANGE_MIN
#define FS_EPHEMERAL_PORT_RANGE_MIN		45000
#endif

#ifndef FS_EPHEMERAL_PORT_RANGE_MAX
#define FS_EPHEMERAL_PORT_RANGE_MAX		65535
#endif

#ifndef FS_STREAM_BUFFER_SIZE
#define FS_STREAM_BUFFER_SIZE			2048
#endif

#define FS_NEW(_Type) (_Type*)fs_base_zalloc(sizeof(_Type))
#define FS_NEW_ARRAY(_Type, _Count) (_Type*)fs_base_zalloc(sizeof(_Type) * _Count)

static void* 
fs_base_zalloc(	
	size_t					aSize)
{
	void* p = malloc(aSize);
	if(p == NULL)
		abort(); // Don't want to bother with handling malloc() fails, just making everything more complex for no reason
	
	memset(p, 0, aSize);
	return p;
}

#endif