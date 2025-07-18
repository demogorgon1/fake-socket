#include "fs_core.h"

#define FS_CORE_MAGIC 0xF5C012E

void				
fs_core_init(
	fs_core*				aCore)
{
	assert(!fs_core_is_init(aCore));
	memset(aCore, 0, sizeof(fs_core));

	aCore->m_numSockets = 1; // Socket 0 isn't used
	aCore->m_listenTable = fs_listen_table_create();

	fs_mutex_init(&aCore->m_lock);

	aCore->m_magic = FS_CORE_MAGIC;
}

void				
fs_core_uninit(
	fs_core*				aCore)
{
	assert(fs_core_is_init(aCore));

	for(uint32_t i = 0; i < aCore->m_numSockets; i++)
	{
		if(aCore->m_sockets[i] != NULL)
			fs_socket_object_destroy(aCore->m_sockets[i]);
	}

	fs_listen_table_destroy(aCore->m_listenTable);

	fs_mutex_uninit(&aCore->m_lock);

	aCore->m_magic = 0;
}

fs_bool				
fs_core_is_init(
	fs_core*				aCore)
{
	return aCore->m_magic == FS_CORE_MAGIC;
}

void				
fs_core_lock(
	fs_core*				aCore)
{
	fs_mutex_lock(&aCore->m_lock);
}

void				
fs_core_unlock(
	fs_core*				aCore)
{
	fs_mutex_unlock(&aCore->m_lock);
}

int					
fs_core_create_socket(
	fs_core*				aCore,
	int*					aOutError)
{
	*aOutError = 0;

	int s = -1;

	if(aCore->m_numFreeSockets > 0)
	{
		s = aCore->m_freeSockets[aCore->m_numFreeSockets];
		aCore->m_numFreeSockets--;
	}
	else if(aCore->m_numSockets < FS_MAX_SOCKETS)
	{
		s = (int)aCore->m_numSockets;
		aCore->m_numSockets++;
	}
	else
	{
		*aOutError = EMFILE;
		return -1;
	}

	assert(s > 0);
	assert(s < FS_MAX_SOCKETS);

	assert(aCore->m_sockets[s] == NULL);
	aCore->m_sockets[s] = fs_socket_object_create();

	return s;
}

void				
fs_core_destroy_socket(
	fs_core*				aCore,
	int						aSocket)
{
	assert(aSocket > 0);
	assert((uint32_t)aSocket < aCore->m_numSockets);
	assert(aCore->m_sockets[aSocket] != NULL);

	fs_socket_object_destroy(aCore->m_sockets[aSocket]);
	aCore->m_sockets[aSocket] = NULL;

	assert(aCore->m_numSockets > 0);

	if ((uint32_t)aSocket == aCore->m_numSockets - 1)
	{
		aCore->m_numSockets--;
	}
	else
	{
		assert(aCore->m_numFreeSockets < FS_MAX_SOCKETS);
		aCore->m_freeSockets[aCore->m_numFreeSockets] = aSocket;
		aCore->m_numFreeSockets++;
	}
}

fs_bool				
fs_core_is_valid_socket(
	fs_core*				aCore,
	int						aSocket)
{
	return fs_core_get_socket(aCore, aSocket) != NULL;
}

fs_socket_object* 
fs_core_get_socket(
	fs_core*				aCore,
	int						aSocket)
{
	return aSocket <= 0 || (uint32_t)aSocket >= aCore->m_numSockets ? NULL : aCore->m_sockets[aSocket];
}

fs_bool				
fs_core_bind(
	fs_core*				aCore,
	int						aSocket,
	uint16_t				aPort,
	int*					aOutError)
{
	*aOutError = 0;

	fs_socket_object* socketObject = fs_core_get_socket(aCore, aSocket);
	if(socketObject == NULL)
	{
		*aOutError = ENOTSOCK;
		return FS_FALSE;
	}

	if(socketObject->m_localPort != 0)
	{
		*aOutError = EINVAL;
		return FS_FALSE;
	}

	socketObject->m_localPort = aPort;

	return FS_TRUE;
}

fs_bool				
fs_core_connect(
	fs_core*				aCore,
	int						aSocket,
	uint16_t				aPort,
	int*					aOutError)
{
	*aOutError = 0;

	fs_socket_object* socketObject = fs_core_get_socket(aCore, aSocket);
	if (socketObject == NULL)
	{
		*aOutError = ENOTSOCK;
		return FS_FALSE;
	}

	if(socketObject->m_remoteSocket != -1 || socketObject->m_stream != NULL)
	{
		*aOutError = EISCONN;
		return FS_FALSE;
	}

	socketObject->m_remotePort = aPort;

	int listenSocket = fs_listen_table_get(aCore->m_listenTable, aPort);
	
	if (listenSocket == -1)
	{
		socketObject->m_closed = FS_TRUE;		
	}
	else
	{
		socketObject->m_localPort = 10000; // FIXME: ephemeral port

		fs_socket_object* listenSocketObject = fs_core_get_socket(aCore, listenSocket);
		assert(listenSocketObject != NULL);
		assert(listenSocketObject->m_acceptBacklog != NULL);

		if(!fs_accept_backlog_add(listenSocketObject->m_acceptBacklog, aSocket))
		{
			*aOutError = ECONNREFUSED;
			return FS_FALSE;
		}
	}

	return FS_TRUE;
}

fs_bool				
fs_core_listen(
	fs_core*				aCore,
	int						aSocket,
	size_t					aBacklog,
	int*					aOutError)
{
	*aOutError = 0;

	fs_socket_object* socketObject = fs_core_get_socket(aCore, aSocket);
	if (socketObject == NULL)
	{
		*aOutError = ENOTSOCK;
		return FS_FALSE;
	}

	if (socketObject->m_localPort == 0 || socketObject->m_acceptBacklog != NULL)
	{
		*aOutError = EINVAL;
		return FS_FALSE;
	}

	if (fs_listen_table_get(aCore->m_listenTable, socketObject->m_localPort) != -1)
	{
		*aOutError = EADDRINUSE;
		return FS_FALSE;
	}

	fs_socket_object_init_accept_backlog(socketObject, aBacklog < FS_MAX_BACKLOG ? aBacklog : FS_MAX_BACKLOG);
	
	aCore->m_listenTable->m_sockets[socketObject->m_localPort] = aSocket;

	return FS_TRUE;
}

int					
fs_core_accept(
	fs_core*				aCore,
	int						aSocket,
	uint16_t*				aOutRemotePort,
	int*					aOutError)
{
	*aOutError = 0;

	fs_socket_object* listenSocketObject = fs_core_get_socket(aCore, aSocket);
	if(listenSocketObject == NULL)
	{
		*aOutError = ENOTSOCK;
		return -1;
	}

	if(listenSocketObject->m_acceptBacklog == NULL)
	{
		*aOutError = EINVAL;
		return -1;
	}

	int remoteSocket = fs_accept_backlog_get_next(listenSocketObject->m_acceptBacklog);
	if(remoteSocket == -1)
	{
		*aOutError = EAGAIN;
		return -1;
	}

	fs_socket_object* remoteSocketObject = fs_core_get_socket(aCore, remoteSocket);
	assert(remoteSocketObject != NULL);

	int clientSocket = fs_core_create_socket(aCore, aOutError);
	if(clientSocket == -1)
	{
		remoteSocketObject->m_closed = FS_TRUE;
		return -1;
	}
	
	fs_socket_object* clientSocketObject = fs_core_get_socket(aCore, clientSocket);
	assert(clientSocketObject != NULL);

	clientSocketObject->m_remoteSocket = remoteSocket;
	clientSocketObject->m_remotePort = remoteSocketObject->m_localPort;
	clientSocketObject->m_localPort = listenSocketObject->m_localPort;

	remoteSocketObject->m_remoteSocket = clientSocket;
	remoteSocketObject->m_remotePort = clientSocketObject->m_localPort;

	fs_socket_object_init_stream(clientSocketObject);
	fs_socket_object_init_stream(remoteSocketObject);

	*aOutRemotePort = remoteSocketObject->m_localPort;

	return clientSocket;
}

size_t				
fs_core_send(
	fs_core*				aCore,
	int						aSocket,
	const void*				aBuffer,
	size_t					aBufferSize,
	int*					aOutError)
{
	*aOutError = 0;

	fs_socket_object* socketObject = fs_core_get_socket(aCore, aSocket);
	if(socketObject == NULL)
	{
		*aOutError = ENOTSOCK;
		return SIZE_MAX;
	}

	if(socketObject->m_closed)
	{
		*aOutError = ECONNRESET;
		return SIZE_MAX;
	}

	if (socketObject->m_remotePort == 0)
	{
		*aOutError = ENOTCONN;
		return SIZE_MAX;
	}

	if(socketObject->m_remoteSocket == -1)
	{
		*aOutError = EAGAIN;
		return SIZE_MAX;
	}

	fs_socket_object* remoteSocketObject = fs_core_get_socket(aCore, socketObject->m_remoteSocket);
	if(remoteSocketObject == NULL)
	{
		*aOutError = ECONNRESET;
		return SIZE_MAX;
	}

	if(remoteSocketObject->m_stream == NULL)
	{
		*aOutError = EAGAIN;
		return SIZE_MAX;
	}

	fs_stream_write(remoteSocketObject->m_stream, aBuffer, aBufferSize);

	return aBufferSize;
}

size_t				
fs_core_recv(
	fs_core*				aCore,
	int						aSocket,
	void*					aBuffer,
	size_t					aBufferSize,
	int*					aOutError)
{
	*aOutError = 0;

	fs_socket_object* socketObject = fs_core_get_socket(aCore, aSocket);
	if (socketObject == NULL)
	{
		*aOutError = ENOTSOCK;
		return SIZE_MAX;
	}

	if(socketObject->m_stream == NULL)
	{
		*aOutError = ENOTCONN;
		return SIZE_MAX;
	}

	if (socketObject->m_closed)
		return 0;

	size_t bytes = fs_stream_read(socketObject->m_stream, aBuffer, aBufferSize);
	if(bytes == 0)
	{
		*aOutError = EAGAIN;
		return SIZE_MAX;
	}

	return bytes;
}

void				
fs_core_close(
	fs_core*				aCore,
	int						aSocket)
{
	fs_socket_object* socketObject = fs_core_get_socket(aCore, aSocket);
	if (socketObject != NULL)
		socketObject->m_closed = FS_TRUE;
}

