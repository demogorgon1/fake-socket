#ifndef __FS_SOCKET_OBJECT_H__
#define __FS_SOCKET_OBJECT_H__

#include <fs/fs_base.h>

#include "fs_accept_backlog.h"
#include "fs_stream.h"

typedef struct _fs_socket_object
{
	fs_bool				m_closed;
	int					m_remoteSocket;

	uint16_t			m_localPort;
	uint16_t			m_remotePort;

	fs_accept_backlog*	m_acceptBacklog;
	fs_stream*			m_stream;
} fs_socket_object;

fs_socket_object*		fs_socket_object_create();
void					fs_socket_object_destroy(
							fs_socket_object*		aSocketObject);
void					fs_socket_object_init_accept_backlog(
							fs_socket_object*		aSocketObject,
							size_t					aSize);
void					fs_socket_object_init_stream(
							fs_socket_object*		aSocketObject);

#endif 
