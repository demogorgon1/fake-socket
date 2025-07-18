#include "../lib/fs_stream.h"

#include "test.h"

#define LARGE_BUFFER_SIZE 10000

uint8_t*
create_random_buffer(
	size_t					aSize)
{
	uint8_t* buffer = FS_NEW_ARRAY(uint8_t, aSize);
	for(size_t i = 0; i < aSize; i++)
		buffer[i] = (uint8_t)rand();		

	return buffer;
}

void
test_stream()
{
	srand(12345678);

	uint8_t* largeBuffer = create_random_buffer(LARGE_BUFFER_SIZE);
	uint8_t* readBuffer = create_random_buffer(LARGE_BUFFER_SIZE);

	{
		fs_stream* stream = fs_stream_create();
		TEST_ASSERT(stream->m_bytes == 0);
		TEST_ASSERT(stream->m_headReadOffset == 0);
		TEST_ASSERT(stream->m_tailWriteOffset == 0);

		uint64_t value1 = 123;
		uint64_t value2 = 456;
		uint64_t value3 = 789;

		fs_stream_write(stream, &value1, sizeof(value1));
		TEST_ASSERT(stream->m_bytes == sizeof(value1));
		TEST_ASSERT(stream->m_headReadOffset == 0);
		TEST_ASSERT(stream->m_tailWriteOffset == sizeof(value1));

		fs_stream_write(stream, &value2, sizeof(value2));
		TEST_ASSERT(stream->m_bytes == sizeof(value1) + sizeof(value2));
		TEST_ASSERT(stream->m_headReadOffset == 0);
		TEST_ASSERT(stream->m_tailWriteOffset == sizeof(value1) + sizeof(value2));

		{
			int64_t value;
			TEST_ASSERT(fs_stream_read(stream, &value, sizeof(value)) == sizeof(value));
			TEST_ASSERT(value == value1);
			TEST_ASSERT(stream->m_bytes == sizeof(value2));
			TEST_ASSERT(stream->m_headReadOffset == sizeof(value1));
			TEST_ASSERT(stream->m_tailWriteOffset == sizeof(value1) + sizeof(value2));
		}

		fs_stream_write(stream, &value3, sizeof(value3));
		TEST_ASSERT(stream->m_bytes == sizeof(value2) + sizeof(value3));
		TEST_ASSERT(stream->m_headReadOffset == sizeof(value1));
		TEST_ASSERT(stream->m_tailWriteOffset == sizeof(value1) + sizeof(value2) + sizeof(value3));

		{
			int64_t value;
			TEST_ASSERT(fs_stream_read(stream, &value, sizeof(value)) == sizeof(value));
			TEST_ASSERT(value == value2);
			TEST_ASSERT(stream->m_bytes == sizeof(value3));
			TEST_ASSERT(stream->m_headReadOffset == sizeof(value1) + sizeof(value2));
			TEST_ASSERT(stream->m_tailWriteOffset == sizeof(value1) + sizeof(value2) + sizeof(value3));
		}

		{
			int64_t value;
			TEST_ASSERT(fs_stream_read(stream, &value, sizeof(value)) == sizeof(value));
			TEST_ASSERT(value == value3);
			TEST_ASSERT(stream->m_bytes == 0);
			TEST_ASSERT(stream->m_headReadOffset == 0);
			TEST_ASSERT(stream->m_tailWriteOffset == 0);
			TEST_ASSERT(stream->m_head == NULL);
			TEST_ASSERT(stream->m_tail == NULL);
		}

		fs_stream_destroy(stream);
	}

	{
		fs_stream* stream = fs_stream_create();
		fs_stream_write(stream, largeBuffer, LARGE_BUFFER_SIZE);
		TEST_ASSERT(stream->m_bytes == LARGE_BUFFER_SIZE);

		TEST_ASSERT(fs_stream_read(stream, readBuffer, LARGE_BUFFER_SIZE) == LARGE_BUFFER_SIZE);
		TEST_ASSERT(memcmp(readBuffer, largeBuffer, LARGE_BUFFER_SIZE) == 0);
		TEST_ASSERT(stream->m_bytes == 0);
		TEST_ASSERT(stream->m_headReadOffset == 0);
		TEST_ASSERT(stream->m_tailWriteOffset == 0);
		TEST_ASSERT(stream->m_head == NULL);
		TEST_ASSERT(stream->m_tail == NULL);

		fs_stream_destroy(stream);
	}

	{
		fs_stream* stream = fs_stream_create();
		fs_stream_write(stream, largeBuffer, LARGE_BUFFER_SIZE);

		for(size_t offset = 0; offset < LARGE_BUFFER_SIZE; offset += FS_STREAM_BUFFER_SIZE)
		{
			size_t remaining = LARGE_BUFFER_SIZE - offset;
			if(remaining > FS_STREAM_BUFFER_SIZE)
				remaining = FS_STREAM_BUFFER_SIZE;

			TEST_ASSERT(fs_stream_read(stream, readBuffer, remaining) == remaining);
			TEST_ASSERT(memcmp(readBuffer, largeBuffer + offset, remaining) == 0);
		}

		TEST_ASSERT(stream->m_bytes == 0);
		TEST_ASSERT(stream->m_headReadOffset == 0);
		TEST_ASSERT(stream->m_tailWriteOffset == 0);
		TEST_ASSERT(stream->m_head == NULL);
		TEST_ASSERT(stream->m_tail == NULL);

		fs_stream_destroy(stream);
	}

	{
		fs_stream* stream = fs_stream_create();
		fs_stream_write(stream, largeBuffer, LARGE_BUFFER_SIZE);

		size_t offset = 0;
		size_t steps[4] = { 3, 43, 87, 3025 };
		size_t stepIndex = 0;

		for(;;)
		{
			size_t toRead = steps[stepIndex];
			size_t remaining = LARGE_BUFFER_SIZE - offset;

			if(toRead > remaining)
			{
				TEST_ASSERT(fs_stream_read(stream, readBuffer, remaining) == remaining);
				TEST_ASSERT(memcmp(readBuffer, largeBuffer + offset, remaining) == 0);
				break;
			}
			else
			{
				TEST_ASSERT(fs_stream_read(stream, readBuffer, toRead) == toRead);
				TEST_ASSERT(memcmp(readBuffer, largeBuffer + offset, toRead) == 0);
				offset += toRead;
			}

			stepIndex = (stepIndex + 1) % 4;
		}

		TEST_ASSERT(stream->m_bytes == 0);
		TEST_ASSERT(stream->m_headReadOffset == 0);
		TEST_ASSERT(stream->m_tailWriteOffset == 0);
		TEST_ASSERT(stream->m_head == NULL);
		TEST_ASSERT(stream->m_tail == NULL);

		fs_stream_destroy(stream);
	}

	free(largeBuffer);
	free(readBuffer);
}
