#include "fs_accept_backlog.h"
#include "fs_mem.h"

fs_accept_backlog* 
fs_accept_backlog_create(
	size_t				aSize)
{
	assert(aSize > 0);

	fs_accept_backlog* t = FS_NEW(fs_accept_backlog);

	t->m_size = aSize;
	t->m_remoteSockets = FS_NEW_ARRAY(int, aSize);

	return t;
}

void				
fs_accept_backlog_destroy(
	fs_accept_backlog*	aAcceptBacklog)
{
	if(aAcceptBacklog != NULL)
	{
		assert(aAcceptBacklog->m_remoteSockets != NULL);
			
		free(aAcceptBacklog->m_remoteSockets);
		free(aAcceptBacklog);
	}
}

fs_bool				
fs_accept_backlog_add(
	fs_accept_backlog*	aAcceptBacklog,
	int					aRemoteSocket)
{
	if(aAcceptBacklog->m_count == aAcceptBacklog->m_size)
		return FS_FALSE;

	assert(aAcceptBacklog->m_count < aAcceptBacklog->m_size);
	assert(aAcceptBacklog->m_write < aAcceptBacklog->m_size);

	aAcceptBacklog->m_remoteSockets[aAcceptBacklog->m_write] = aRemoteSocket;	
	aAcceptBacklog->m_write = (aAcceptBacklog->m_write + 1) % aAcceptBacklog->m_size;

	aAcceptBacklog->m_count++;

	return FS_TRUE;
}

int					
fs_accept_backlog_get_next(
	fs_accept_backlog*	aAcceptBacklog)
{
	if (aAcceptBacklog->m_count == 0)
		return -1;

	int s = -1;

	size_t loops = 0;

	while(s == -1)
	{
		assert(loops < aAcceptBacklog->m_size);
		assert(aAcceptBacklog->m_read < aAcceptBacklog->m_size);
		s = aAcceptBacklog->m_remoteSockets[aAcceptBacklog->m_read];
		aAcceptBacklog->m_read = (aAcceptBacklog->m_read + 1) % aAcceptBacklog->m_size;

		loops++;
	}

	aAcceptBacklog->m_count--;

	return s;
}

void				
fs_accept_backlog_remove(
	fs_accept_backlog*	aAcceptBacklog,
	int					aRemoteSocket)
{
	for(size_t i = 0; i < aAcceptBacklog->m_size; i++)
	{
		if(aAcceptBacklog->m_remoteSockets[i] == aRemoteSocket)
		{
			aAcceptBacklog->m_remoteSockets[i] = -1;
			aAcceptBacklog->m_count--;
			break;
		}
	}
}

