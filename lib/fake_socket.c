#include "fs_core.h"

fs_core*		fs_core_global = NULL;

void
fs_init()
{
	assert(fs_core_global == NULL);
	
	fs_core_global = fs_core_create();
}

void
fs_uninit()
{
	assert(fs_core_global != NULL);

	fs_core_destroy(fs_core_global);
}

fs_bool		
fs_is_valid_socket(
	int				aSocket)
{
	fs_core_lock(fs_core_global);
	
	fs_bool isValid = fs_core_is_valid_socket(fs_core_global, aSocket);
	
	fs_core_unlock(fs_core_global);

	return isValid;
}

fs_bool		
fs_is_closed_socket(
	int				aSocket)
{
	fs_core_lock(fs_core_global);
	
	fs_bool isClosed = fs_core_is_closed_socket(fs_core_global, aSocket);
	
	fs_core_unlock(fs_core_global);

	return isClosed;
}

fs_bool
fs_is_connected_socket(
	int				aSocket)
{
	fs_core_lock(fs_core_global);

	fs_bool isConnected = fs_core_is_connected_socket(fs_core_global, aSocket);

	fs_core_unlock(fs_core_global);

	return isConnected;
}

int			
fs_socket(
	int				aDomain,
	int				aType,
	int				aProtocol)
{
	if(aDomain != AF_INET || aType != SOCK_STREAM || aProtocol != 0)
	{
		errno = EINVAL;
		return -1;
	}

	fs_core_lock(fs_core_global);

	int error = 0;
	int s = fs_core_create_socket(fs_core_global, &error);

	if(s == -1)
	{
		assert(error != 0);
		errno = error;
	}

	fs_core_unlock(fs_core_global);

	return s;
}

int			
fs_close(
	int				aSocket)
{
	fs_core_lock(fs_core_global);
	fs_core_destroy_socket(fs_core_global, aSocket);
	fs_core_unlock(fs_core_global);

	return 0;
}

size_t		
fs_send(
	int				aSocket,
	const void*		aBuffer,
	size_t			aBufferSize,
	int				aFlags)
{
	(void)aFlags; // Unused

	fs_core_lock(fs_core_global);

	int error = 0;
	size_t result = fs_core_send(fs_core_global, aSocket, aBuffer, aBufferSize, &error);

	fs_core_unlock(fs_core_global);

	if(result == SIZE_MAX)
	{
		assert(error != 0);
		errno = error;
	}

	return result;
}

size_t		
fs_recv(
	int				aSocket,
	void*			aBuffer,
	size_t			aBufferSize,
	int				aFlags)
{
	(void)aFlags; // Unused

	fs_core_lock(fs_core_global);

	int error = 0;
	size_t result = fs_core_recv(fs_core_global, aSocket, aBuffer, aBufferSize, &error);

	fs_core_unlock(fs_core_global);

	if(result == SIZE_MAX)
	{
		assert(error != 0);
		errno = error;
	}

	return result;
}

int			
fs_listen(
	int				aSocket,
	int				aBacklog)
{
	fs_core_lock(fs_core_global);

	int error = 0;
	fs_bool ok = fs_core_listen(fs_core_global, aSocket, (size_t)aBacklog, &error);

	fs_core_unlock(fs_core_global);

	if (!ok)
	{
		assert(error != 0);
		errno = error;
		return -1;
	}

	return 0;
}

int			
fs_accept(
	int					aSocket,
	struct sockaddr*	aOutAddr,
	socklen_t*			aOutAddrLen,
	int					aFlags)
{
	(void)aFlags; // Unused

	if(aOutAddr == NULL || aOutAddrLen == NULL || *aOutAddrLen < sizeof(struct sockaddr_in))
	{
		errno = EINVAL;
		return -1;
	}

	fs_core_lock(fs_core_global);

	int error = 0;
	uint16_t remotePort = 0;
	int s = fs_core_accept(fs_core_global, aSocket, &remotePort, &error);

	fs_core_unlock(fs_core_global);

	if(s == -1)
	{
		assert(error != 0);
		errno = error;
	}
	else
	{
		struct sockaddr_in* sin = (struct sockaddr_in*)aOutAddr;
		memset(sin, 0, sizeof(struct sockaddr_in));
		sin->sin_family = AF_INET;
		sin->sin_addr.S_un.S_addr = INADDR_LOOPBACK;
		sin->sin_port = htons(remotePort);
		*aOutAddrLen = sizeof(struct sockaddr_in);
	}

	return s;
}

int			
fs_bind(
	int						aSocket,
	const struct sockaddr*	aAddr,
	socklen_t				aAddrLen)
{
	if (aAddr == NULL || aAddrLen != sizeof(struct sockaddr_in))
	{
		errno = EINVAL;
		return -1;
	}

	const struct sockaddr_in* sin = (const struct sockaddr_in*)aAddr;
	if(sin->sin_family != AF_INET)
	{
		errno = EINVAL;
		return -1;
	}

	uint16_t remotePort = ntohs(sin->sin_port);

	fs_core_lock(fs_core_global);

	int error = 0;
	fs_bool ok = fs_core_bind(fs_core_global, aSocket, remotePort, &error);

	fs_core_unlock(fs_core_global);

	if(!ok)
	{
		assert(error != 0);
		errno = error;
		return -1;
	}

	return 0;
}

int			
fs_connect(
	int						aSocket,
	const struct sockaddr*	aAddr,
	socklen_t				aAddrLen)
{
	if (aAddr == NULL || aAddrLen != sizeof(struct sockaddr_in))
	{
		errno = EINVAL;
		return -1;
	}

	const struct sockaddr_in* sin = (const struct sockaddr_in*)aAddr;
	if (sin->sin_family != AF_INET)
	{
		errno = EINVAL;
		return -1;
	}

	uint16_t remotePort = ntohs(sin->sin_port);

	fs_core_lock(fs_core_global);

	int error = 0;
	fs_bool ok = fs_core_connect(fs_core_global, aSocket, remotePort, &error);

	fs_core_unlock(fs_core_global);

	if (!ok)
	{
		assert(error != 0);
		errno = error;
		return -1;
	}

	return 0;
}

