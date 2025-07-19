#ifndef __FS_STREAM_H__
#define __FS_STREAM_H__

#include <fs/fs_base.h>

typedef struct _fs_stream_buffer
{
	uint8_t						m_data[FS_STREAM_BUFFER_SIZE];
	struct _fs_stream_buffer*	m_next;
} fs_stream_buffer;

typedef struct _fs_stream
{
	fs_stream_buffer*			m_head;
	fs_stream_buffer*			m_tail;
	size_t						m_bytes;
	size_t						m_tailWriteOffset;
	size_t						m_headReadOffset;
} fs_stream;

fs_stream*			fs_stream_create();
void				fs_stream_destroy(
						fs_stream*		aStream);
void				fs_stream_write(
						fs_stream*		aStream,
						const void*		aBuffer,
						size_t			aBufferSize);
size_t				fs_stream_read(
						fs_stream*		aStream,
						void*			aBuffer,
						size_t			aBufferSize);
void				fs_stream_clear(
						fs_stream*      aStream);

#endif
