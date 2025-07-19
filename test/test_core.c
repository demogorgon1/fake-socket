#include "../lib/fs_core.h"

#include "test.h"

size_t
get_num_valid_sockets(
	fs_core*						aCore)
{
	size_t count = 0;
	for(size_t i = 0; i < FS_MAX_SOCKETS; i++)
	{
		if(fs_core_is_valid_socket(aCore, i))
			count++;
	}
	return count;
}

typedef enum _simple_connection_test_mode
{
	SIMPLE_CONNECTION_TEST_MODE_CLIENT_CLOSE,
	SIMPLE_CONNECTION_TEST_MODE_SERVER_CLOSE
} simple_connection_test_mode;

void
test_1()
{
	fs_core* core = fs_core_create();
	TEST_ASSERT(get_num_valid_sockets(core) == 0);

	int error = 0;

	{
		int s = fs_core_create_socket(core, &error);
		TEST_ASSERT(s != -1);
		TEST_ASSERT(error == 0);
		TEST_ASSERT(get_num_valid_sockets(core) == 1);
		TEST_ASSERT(fs_core_is_valid_socket(core, s));
		fs_socket_object* socketObject = fs_core_get_socket(core, s);
		TEST_ASSERT(socketObject != NULL);
		TEST_ASSERT(!socketObject->m_closed);
		TEST_ASSERT(socketObject->m_localPort == 0);
		TEST_ASSERT(socketObject->m_remotePort == 0);
		fs_core_destroy_socket(core, s);
		TEST_ASSERT(get_num_valid_sockets(core) == 0);
	}	

	fs_core_destroy(core);
}

void
test_2(
	simple_connection_test_mode		aSimpleConnectionTestMode)
{
	fs_core* core = fs_core_create();
	TEST_ASSERT(get_num_valid_sockets(core) == 0);

	int error = 0;
	uint16_t remotePort = 0;

	char recvBuffer[256];

	{
		int listenSocket = fs_core_create_socket(core, &error);
		TEST_ASSERT(listenSocket != -1);
		TEST_ASSERT(error == 0);
		TEST_ASSERT(get_num_valid_sockets(core) == 1);
		TEST_ASSERT(fs_core_bind(core, listenSocket, 12345, &error));
		TEST_ASSERT(error == 0);

		TEST_ASSERT(fs_core_listen(core, listenSocket, 32, &error));
		TEST_ASSERT(error == 0);
		TEST_ASSERT(fs_core_accept(core, listenSocket, &remotePort, &error) == -1);
		TEST_ASSERT(error == EAGAIN);

		int connectSocket = fs_core_create_socket(core, &error);
		TEST_ASSERT(error == 0);
		TEST_ASSERT(connectSocket != -1);
		TEST_ASSERT(get_num_valid_sockets(core) == 2);
		TEST_ASSERT(fs_core_connect(core, connectSocket, 12345, &error));
		TEST_ASSERT(error == 0);
		TEST_ASSERT(!fs_core_is_connected_socket(core, connectSocket));
		TEST_ASSERT(!fs_core_is_closed_socket(core, connectSocket));

		{
			size_t bytes = fs_core_send(core, connectSocket, "Hello", 6, &error);
			TEST_ASSERT(bytes == SIZE_MAX);
			TEST_ASSERT(error == EAGAIN);
		}

		int clientSocket = fs_core_accept(core, listenSocket, &remotePort, &error);
		TEST_ASSERT(error == 0);
		TEST_ASSERT(clientSocket != -1);
		TEST_ASSERT(get_num_valid_sockets(core) == 3);

		TEST_ASSERT(fs_core_is_connected_socket(core, connectSocket));
		TEST_ASSERT(!fs_core_is_closed_socket(core, connectSocket));

		TEST_ASSERT(fs_core_is_connected_socket(core, clientSocket));
		TEST_ASSERT(!fs_core_is_closed_socket(core, clientSocket));

		{
			size_t bytes = fs_core_recv(core, connectSocket, recvBuffer, sizeof(recvBuffer), &error);
			TEST_ASSERT(bytes == SIZE_MAX);
			TEST_ASSERT(error == EAGAIN);
		}

		{
			size_t bytes = fs_core_recv(core, clientSocket, recvBuffer, sizeof(recvBuffer), &error);
			TEST_ASSERT(bytes == SIZE_MAX);
			TEST_ASSERT(error == EAGAIN);
		}

		{
			size_t bytes = fs_core_send(core, connectSocket, "Hello1", 6, &error);
			TEST_ASSERT(bytes == 6);
			TEST_ASSERT(error == 0);
		}

		{
			size_t bytes = fs_core_send(core, clientSocket, "Hello2", 6, &error);
			TEST_ASSERT(bytes == 6);
			TEST_ASSERT(error == 0);
		}

		{
			size_t bytes = fs_core_recv(core, connectSocket, recvBuffer, sizeof(recvBuffer), &error);
			TEST_ASSERT(bytes == 6);
			TEST_ASSERT(error == 0);
			TEST_ASSERT(memcmp(recvBuffer, "Hello2", 6) == 0);
		}

		{
			size_t bytes = fs_core_recv(core, clientSocket, recvBuffer, sizeof(recvBuffer), &error);
			TEST_ASSERT(bytes == 6);
			TEST_ASSERT(error == 0);
			TEST_ASSERT(memcmp(recvBuffer, "Hello1", 6) == 0);
		}

		{
			switch (aSimpleConnectionTestMode)
			{
			case SIMPLE_CONNECTION_TEST_MODE_CLIENT_CLOSE:	fs_core_close(core, connectSocket); break;
			case SIMPLE_CONNECTION_TEST_MODE_SERVER_CLOSE:  fs_core_close(core, clientSocket); break;
			default:
				break;
			}

			TEST_ASSERT(!fs_core_is_connected_socket(core, connectSocket));
			TEST_ASSERT(fs_core_is_closed_socket(core, connectSocket));

			TEST_ASSERT(!fs_core_is_connected_socket(core, clientSocket));
			TEST_ASSERT(fs_core_is_closed_socket(core, clientSocket));
		}
	}

	fs_core_destroy(core);

}

void
test_core()
{	
	test_1();
	test_2(SIMPLE_CONNECTION_TEST_MODE_CLIENT_CLOSE);
	test_2(SIMPLE_CONNECTION_TEST_MODE_SERVER_CLOSE);
}