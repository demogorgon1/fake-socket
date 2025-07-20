#ifndef __FS_BASE_H__
#define __FS_BASE_H__

#if defined(WIN32)
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>	
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <errno.h>
	#include <pthread.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>

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

#endif