#include "fs_stream.h"

fs_stream* 
fs_stream_create()
{
	return FS_NEW(fs_stream);
}

void				
fs_stream_destroy(
	fs_stream*	aStream)
{
	if(aStream != NULL)
	{
		fs_stream_clear(aStream);

		free(aStream);
	}
}

void				
fs_stream_write(
	fs_stream*	aStream,
	const void* aBuffer,
	size_t		aBufferSize)
{
	size_t remaining = aBufferSize;
	const uint8_t* p = (const uint8_t*)aBuffer;

	while(remaining > 0)
	{
		if(aStream->m_tail == NULL)
		{		
			assert(aStream->m_head == NULL);
			assert(aStream->m_headReadOffset == 0);
			assert(aStream->m_tailWriteOffset == 0);

			aStream->m_tail = FS_NEW(fs_stream_buffer);
			aStream->m_head = aStream->m_tail;
		}
		else if(aStream->m_tailWriteOffset == FS_STREAM_BUFFER_SIZE)
		{
			aStream->m_tail->m_next = FS_NEW(fs_stream_buffer);
			aStream->m_tail = aStream->m_tail->m_next;

			aStream->m_tailWriteOffset = 0;
		}

		size_t spaceLeft = FS_STREAM_BUFFER_SIZE - aStream->m_tailWriteOffset;
		assert(spaceLeft > 0);
		assert(aStream->m_tail != NULL);

		size_t toCopy = remaining <= spaceLeft ? remaining : spaceLeft;

		memcpy(aStream->m_tail->m_data + aStream->m_tailWriteOffset, p, toCopy);

		aStream->m_tailWriteOffset += toCopy;
		aStream->m_bytes += toCopy;
		p += toCopy;
		remaining -= toCopy;
	}
}

size_t				
fs_stream_read(
	fs_stream*	aStream,
	void*		aBuffer,
	size_t		aBufferSize)
{
	size_t remaining = aBufferSize;
	uint8_t* p = (uint8_t*)aBuffer;
	size_t totalRead = 0;

	while(aStream->m_bytes > 0 && remaining > 0)
	{
		assert(aStream->m_head != NULL);
		assert(aStream->m_tail != NULL);

		assert(aStream->m_headReadOffset < FS_STREAM_BUFFER_SIZE);

		size_t headBytes = aStream->m_head == aStream->m_tail ? aStream->m_tailWriteOffset : FS_STREAM_BUFFER_SIZE;
		assert(headBytes <= FS_STREAM_BUFFER_SIZE);
		
		assert(aStream->m_headReadOffset < headBytes);
		size_t pendingBytes = headBytes - aStream->m_headReadOffset;

		size_t toCopy = remaining <= pendingBytes ? remaining : pendingBytes;
		assert(toCopy <= aStream->m_bytes);

		memcpy(p, aStream->m_head->m_data + aStream->m_headReadOffset, toCopy);

		aStream->m_headReadOffset += toCopy;
		aStream->m_bytes -= toCopy;
		p += toCopy;
		totalRead += toCopy;
		remaining -= toCopy;

		if(aStream->m_headReadOffset == FS_STREAM_BUFFER_SIZE)
		{
			fs_stream_buffer* next = aStream->m_head->m_next;

			free(aStream->m_head);

			aStream->m_head = next;
			aStream->m_headReadOffset = 0;

			if(aStream->m_head == NULL)
			{
				aStream->m_tail = NULL;
			}
		}
	}

	if(aStream->m_bytes == 0 && aStream->m_head != NULL)
	{
		assert(aStream->m_head == aStream->m_tail);

		free(aStream->m_head);

		aStream->m_head = NULL;
		aStream->m_tail = NULL;

		aStream->m_headReadOffset = 0;
		aStream->m_tailWriteOffset = 0;
	}

	return totalRead;
}

void
fs_stream_clear(
	fs_stream*	aStream)
{
	fs_stream_buffer* p = aStream->m_head;
	while(p != NULL)
	{
		fs_stream_buffer* next = p->m_next;
		free(p);
		p = next;
	}

	aStream->m_bytes = 0;
	aStream->m_headReadOffset = 0;
	aStream->m_tailWriteOffset = 0;
	aStream->m_head = NULL;
	aStream->m_tail = NULL;
}
