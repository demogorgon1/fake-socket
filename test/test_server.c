#include <fs/fake_socket.h>

#include "test.h"

#define NUM_SERVERS				1
#define NUM_CLIENTS				1
#define MAX_SERVER_CLIENTS		NUM_CLIENTS
#define MAX_PENDING_RESPONSES	10
#define MAX_PENDING_REQUESTS	10

#define MESSAGE_REQUEST_START	1
#define MESSAGE_REQUEST_END		0

typedef enum _message_parser_state
{
	MESSAGE_PARSER_STATE_INIT,
	MESSAGE_PARSER_STATE_REQUEST,
} message_parser_state;

typedef struct _message_parser
{
	message_parser_state	m_state;
	uint32_t				m_value;

	uint32_t				m_pendingResponses[MAX_PENDING_RESPONSES];
	size_t					m_numPendingResponses;
} message_parser;

typedef struct _server_client
{
	int						m_socket;
	message_parser			m_messageParser;
} server_client;

typedef struct _server
{
	uint16_t				m_listenPort;
	int						m_listenSocket;
	server_client*			m_clients[MAX_SERVER_CLIENTS];
	size_t					m_numClients;
} server;

typedef struct _client
{
	const uint16_t*			m_availableServerPorts;
	int						m_socket;
	uint32_t				m_pendingRequests[MAX_PENDING_REQUESTS];
	size_t					m_numPendingRequests;
	size_t					m_nextPendingRequest;
	uint8_t					m_recvBuffer[sizeof(uint32_t)];
	size_t					m_recvBufferBytes;
} client;	

void
message_parser_update(
	message_parser*	aParser,
	uint8_t			aInput)
{
	switch(aParser->m_state)
	{
	case MESSAGE_PARSER_STATE_INIT:
		if(aInput == MESSAGE_REQUEST_START)
			aParser->m_state = MESSAGE_PARSER_STATE_REQUEST;
		break;

	case MESSAGE_PARSER_STATE_REQUEST:
		if (aInput == MESSAGE_REQUEST_END)
		{
			if(aParser->m_numPendingResponses < MAX_PENDING_RESPONSES)
				aParser->m_pendingResponses[aParser->m_numPendingResponses++] = aParser->m_value;

			aParser->m_value = 0;
			aParser->m_state = MESSAGE_PARSER_STATE_INIT;
		}
		else
		{
			aParser->m_value += (uint32_t)aInput;
		}
		break;

	default:
		TEST_ASSERT(FS_FALSE);
		break;
	}
}

server*		
server_create(
	uint16_t		aListenPort)
{
	server* t = FS_NEW(server);

	t->m_listenPort = aListenPort;

	{
		t->m_listenSocket = fs_socket(AF_INET, SOCK_STREAM, 0);
		TEST_ASSERT(t->m_listenSocket != -1);
	}

	{
		struct sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(aListenPort);
		int result = fs_bind(t->m_listenSocket, (const struct sockaddr*)&sin, sizeof(sin));
		TEST_ASSERT(result != -1);
	}

	{
		int result = fs_listen(t->m_listenSocket, SOMAXCONN);
		TEST_ASSERT(result != -1);
	}

	return t;
}

void
server_destroy(
	server*			aServer)
{
	for (size_t i = 0; i < aServer->m_numClients; i++)
	{
		server_client* c = aServer->m_clients[i];

		fs_close(c->m_socket);
		free(c);
	}

	fs_close(aServer->m_listenSocket);

	free(aServer);
}

void
server_update(
	server*			aServer)
{
	while(aServer->m_numClients < MAX_SERVER_CLIENTS)
	{
		struct sockaddr addr;
		socklen_t addrSize = sizeof(addr);
		int clientSocket = fs_accept(aServer->m_listenSocket, &addr, &addrSize, 0);
		if(clientSocket == -1)
			break;

		server_client* c = FS_NEW(server_client);
		c->m_socket = clientSocket;
		
		aServer->m_clients[aServer->m_numClients++] = c;
	}

	for(size_t i = 0; i < aServer->m_numClients; i++)
	{
		server_client* c = aServer->m_clients[i];

		fs_bool closed = fs_is_closed_socket(c->m_socket);

		if(!closed)
		{
			uint8_t recvBuffer[256];
			size_t recvBytes = fs_recv(c->m_socket, recvBuffer, sizeof(recvBuffer), 0);
			if(recvBytes == 0 || (recvBytes == SIZE_MAX && errno != EAGAIN))
			{
				closed = FS_TRUE;
			}
			else if(recvBytes != SIZE_MAX)
			{
				for(size_t j = 0; j < recvBytes; j++)
					message_parser_update(&c->m_messageParser, recvBuffer[j]);

				if(c->m_messageParser.m_numPendingResponses > 0)
				{
					for (size_t j = 0; j < c->m_messageParser.m_numPendingResponses && !closed; j++)
					{
						size_t sentBytes = fs_send(c->m_socket, &c->m_messageParser.m_pendingResponses[j], sizeof(uint32_t), 0);
						if(sentBytes != sizeof(uint32_t))
							closed = FS_TRUE;
					}

					c->m_messageParser.m_numPendingResponses = 0;
				}
			}
		}

		if(closed)
		{
			fs_close(c->m_socket);
			free(c);

			if(i < aServer->m_numClients - 1)
				aServer->m_clients[i] = aServer->m_clients[aServer->m_numClients - 1];

			aServer->m_numClients--;

			i--;
		}

	}
}

client*
client_create(
	const uint16_t*		aAvailableServerPorts)
{
	client* t = FS_NEW(client);

	t->m_availableServerPorts = aAvailableServerPorts;
	t->m_socket = -1;

	return t;
}

void
client_destroy(
	client*			aClient)
{
	if(aClient->m_socket != -1)
		fs_close(aClient->m_socket);

	free(aClient);
}

void
client_update(
	client*			aClient)
{
	if(aClient->m_socket == -1)
	{
		uint16_t serverPort = aClient->m_availableServerPorts[rand() % NUM_SERVERS];

		{
			aClient->m_socket = fs_socket(AF_INET, SOCK_STREAM, 0);
			TEST_ASSERT(aClient->m_socket != -1);
		}

		{
			struct sockaddr_in sin;
			memset(&sin, 0, sizeof(sin));
			sin.sin_family = AF_INET;
			sin.sin_port = htons(serverPort);
			int result = fs_connect(aClient->m_socket, (const struct sockaddr*)&sin, sizeof(sin));
			TEST_ASSERT(result != -1);
		}
	}
	else if(fs_is_closed_socket(aClient->m_socket))
	{
		fs_close(aClient->m_socket);

		aClient->m_socket = -1;
	}
	else if(fs_is_connected_socket(aClient->m_socket))
	{
		fs_bool closed = FS_FALSE;

		if(aClient->m_numPendingRequests == 0)
		{
			for(size_t i = 0; i < 1; i++)
			{
				uint8_t message[4];
				message[0] = MESSAGE_REQUEST_START;
				message[1] = 1;
				message[2] = 2;
				message[3] = MESSAGE_REQUEST_END;

				aClient->m_pendingRequests[aClient->m_numPendingRequests++] = 3;

				size_t sentBytes = fs_send(aClient->m_socket, message, sizeof(message), 0);
				if(sentBytes == SIZE_MAX && errno != EAGAIN)
				{
					closed = FS_TRUE;
					break;
				}

				TEST_ASSERT(sentBytes == sizeof(message));
			}

			aClient->m_recvBufferBytes = 0;
			aClient->m_nextPendingRequest = 0;
		}
		else
		{
			TEST_ASSERT(aClient->m_nextPendingRequest < aClient->m_numPendingRequests);

			uint8_t recvValue;
			size_t recvBytes = fs_recv(aClient->m_socket, &recvValue, 1, 0);
			if(recvBytes == 0 || (recvBytes == SIZE_MAX && recvBytes != EAGAIN))
			{
				closed = FS_TRUE;
			}
			else if(recvBytes != SIZE_MAX)
			{
				TEST_ASSERT(recvBytes == 1);
				TEST_ASSERT(aClient->m_recvBufferBytes < sizeof(uint32_t));
				aClient->m_recvBuffer[aClient->m_recvBufferBytes++] = recvValue;

				if(aClient->m_recvBufferBytes == sizeof(uint32_t))
				{
					uint32_t value = *((uint32_t*)aClient->m_recvBuffer);

					TEST_ASSERT(value == aClient->m_pendingRequests[aClient->m_nextPendingRequest]);

					aClient->m_nextPendingRequest++;

					if(aClient->m_nextPendingRequest == aClient->m_numPendingRequests)
						aClient->m_numPendingRequests = 0;
				}
			}
		}

		if(closed)
		{
			fs_close(aClient->m_socket);

			aClient->m_socket = -1;
		}
	}	
}

void
test_server()
{
	fs_init();

	uint16_t availableServerPorts[NUM_SERVERS];

	server* servers[NUM_SERVERS];
	client* clients[NUM_CLIENTS];

	for(size_t i = 0; i < NUM_SERVERS; i++)
	{
		servers[i] = server_create((uint16_t)(10000 + i));

		availableServerPorts[i] = servers[i]->m_listenPort;
	}

	for (size_t i = 0; i < NUM_CLIENTS; i++)
		clients[i] = client_create(availableServerPorts);

	for(;;)
	{
		for (size_t i = 0; i < NUM_SERVERS; i++)
			server_update(servers[i]);

		for (size_t i = 0; i < NUM_CLIENTS; i++)
			client_update(clients[i]);

		Sleep(1);
	}

	for (size_t i = 0; i < NUM_SERVERS; i++)
		server_destroy(servers[i]);

	for (size_t i = 0; i < NUM_CLIENTS; i++)
		client_destroy(clients[i]);

	fs_uninit();
}