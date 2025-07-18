#ifndef __FAKE_SOCKET_H__
#define __FAKE_SOCKET_H__

#include "fs_base.h"

void		fs_init();
void		fs_uninit();
fs_bool		fs_is_valid_socket(
				int						aSocket);

int			fs_socket(	
				int						aDomain,
				int						aType,
				int						aProtocol);
int			fs_close(
				int						aSocket);
size_t		fs_send(
				int						aSocket,
				const void*				aBuffer,
				size_t					aBufferSize,
				int						aFlags);
size_t		fs_recv(
				int						aSocket,
				void*					aBuffer,
				size_t					aBufferSize,
				int						aFlags);
int			fs_listen(
				int						aSocket,
				int						aBacklog);
int			fs_accept(
				int						aSocket,
				struct sockaddr*		aOutAddr,
				socklen_t*				aOutAddrLen,
				int						aFlags);
int			fs_bind(
				int						aSocket,
				const struct sockaddr*	aAddr,
				socklen_t				aAddrLen);
int			fs_connect(
				int						aSocket,
				const struct sockaddr*	aAddr,
				socklen_t				aAddrLen);

#endif