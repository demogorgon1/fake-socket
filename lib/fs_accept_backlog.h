#ifndef __FS_ACCEPT_BACKLOG_H__
#define __FS_ACCEPT_BACKLOG_H__

#include <fs/fs_base.h>

typedef struct _fs_accept_backlog
{
	int*	m_remoteSockets;
	size_t	m_size;
	size_t	m_count;
	size_t	m_read;
	size_t	m_write;
} fs_accept_backlog;

fs_accept_backlog*	fs_accept_backlog_create(
						size_t				aSize);
void				fs_accept_backlog_destroy(
						fs_accept_backlog*	aAcceptBacklog);
fs_bool				fs_accept_backlog_add(
						fs_accept_backlog*	aAcceptBacklog,
						int					aRemoteSocket);
int					fs_accept_backlog_get_next(
						fs_accept_backlog*	aAcceptBacklog);
void				fs_accept_backlog_remove(
						fs_accept_backlog*	aAcceptBacklog,
						int					aRemoteSocket);

#endif