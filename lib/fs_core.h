#ifndef __FS_CORE_H__
#define __FS_CORE_H__

#include <fs/fs_base.h>

#include "fs_port_table.h"
#include "fs_mutex.h"
#include "fs_socket_object.h"

typedef struct _fs_core
{
	fs_socket_object*	m_sockets[FS_MAX_SOCKETS];
	uint32_t			m_numSockets;
	int					m_freeSockets[FS_MAX_SOCKETS];
	uint32_t			m_numFreeSockets;	
	fs_port_table*		m_portTable;			
	fs_mutex			m_lock;
} fs_core;

fs_core*			fs_core_create();
void				fs_core_destroy(
						fs_core*				aCore);
void				fs_core_lock(
						fs_core*				aCore);
void				fs_core_unlock(
						fs_core*				aCore);
int					fs_core_create_socket(
						fs_core*				aCore,
						int*					aOutError);
void				fs_core_destroy_socket(	
						fs_core*				aCore,
						int						aSocket);
fs_bool				fs_core_is_valid_socket(
						fs_core*				aCore,
						int						aSocket);
fs_bool				fs_core_is_closed_socket(
						fs_core*				aCore,
						int						aSocket);
fs_bool				fs_core_is_connected_socket(
						fs_core*				aCore,
						int						aSocket);
fs_socket_object*	fs_core_get_socket(
						fs_core*				aCore,
						int						aSocket);
fs_bool				fs_core_bind(
						fs_core*				aCore,
						int						aSocket,
						uint16_t				aPort,
						int*					aOutError);
fs_bool				fs_core_connect(
						fs_core*				aCore,
						int						aSocket,
						uint16_t				aPort,
						int*					aOutError);
fs_bool				fs_core_listen(
						fs_core*				aCore,
						int						aSocket,
						size_t					aBacklog,
						int*					aOutError);
int					fs_core_accept(
						fs_core*				aCore,
						int						aSocket,
						uint16_t*				aOutRemotePort,
						int*					aOutError);
size_t				fs_core_send(
						fs_core*				aCore,
						int						aSocket,
						const void*				aBuffer,
						size_t					aBufferSize,
						int*					aOutError);
size_t				fs_core_recv(
						fs_core*				aCore,
						int						aSocket,
						void*					aBuffer,
						size_t					aBufferSize,
						int*					aOutError);
void				fs_core_close(
						fs_core*				aCore,
						int						aSocket);

#endif
