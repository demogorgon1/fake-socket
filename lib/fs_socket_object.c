#include "fs_socket_object.h"

fs_socket_object* 
fs_socket_object_create()
{
	fs_socket_object* t = FS_NEW(fs_socket_object);
	t->m_remoteSocket = -1;
	return t;
}

void					
fs_socket_object_destroy(
	fs_socket_object*		aSocketObject)		
{
	if(aSocketObject != NULL)
	{
		fs_stream_destroy(aSocketObject->m_stream);
		fs_accept_backlog_destroy(aSocketObject->m_acceptBacklog);

		free(aSocketObject);
	}
}

void
fs_socket_object_init_accept_backlog(
	fs_socket_object*		aSocketObject,
	size_t					aSize)
{
	assert(aSocketObject != NULL);
	assert(aSocketObject->m_acceptBacklog == NULL);

	aSocketObject->m_acceptBacklog = fs_accept_backlog_create(aSize);
}

void
fs_socket_object_init_stream(
	fs_socket_object*		aSocketObject)
{
	assert(aSocketObject != NULL);
	assert(aSocketObject->m_stream == NULL);

	aSocketObject->m_stream = fs_stream_create();
}

