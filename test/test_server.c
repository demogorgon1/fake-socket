#include <fs/fake_socket.h>

#include "../lib/fs_core.h"
#include "../lib/fs_mem.h"

#include "test.h"

#define NUM_SERVERS				5
#define NUM_CLIENTS				20
#define MAX_SERVER_CLIENTS		NUM_CLIENTS
#define MAX_PENDING_RESPONSES	10
#define MAX_PENDING_REQUESTS	10

#define MESSAGE_REQUEST_START	1
#define MESSAGE_REQUEST_END		0
#define MAX_MESSAGE_SIZE		3000

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
	size_t					m_remainingRoundsBeforeDisconnect;
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
			uint8_t recvBuffer[32768];
			size_t readSize = (size_t)(rand() % 32767) + 1;

			size_t recvBytes = fs_recv(c->m_socket, recvBuffer, readSize, 0);
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
	client*			aClient,
	size_t			aClientNumber)
{
	(void)aClientNumber;

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

		aClient->m_remainingRoundsBeforeDisconnect = 1 + (size_t)(rand() % 40);
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
			if(aClient->m_remainingRoundsBeforeDisconnect == 0)
			{
				closed = FS_TRUE;
			}
			else
			{
				aClient->m_remainingRoundsBeforeDisconnect--;

				size_t requestsToSend = (size_t)(rand() % 10) + 1;

				for(size_t i = 0; i < requestsToSend; i++)
				{
					size_t count = (rand() % (MAX_MESSAGE_SIZE - 1)) + 1;

					uint32_t sum = 0;

					uint8_t message[MAX_MESSAGE_SIZE + 2];
					message[0] = MESSAGE_REQUEST_START;

					for(size_t j = 0; j < count; j++)
					{
						uint8_t v = 1 + (uint8_t)(rand() % 100);
						message[1 + j] = v;
						sum += (uint32_t)v;
					}

					message[count + 1] = MESSAGE_REQUEST_END;

					aClient->m_pendingRequests[aClient->m_numPendingRequests++] = sum;

					size_t sentBytes = fs_send(aClient->m_socket, message, count + 2, 0);
					if(sentBytes == SIZE_MAX && errno != EAGAIN)
					{
						closed = FS_TRUE;
						break;
					}

					TEST_ASSERT(sentBytes == count + 2);
				}

				aClient->m_recvBufferBytes = 0;
				aClient->m_nextPendingRequest = 0;
			}
		}
		else
		{
			TEST_ASSERT(aClient->m_nextPendingRequest < aClient->m_numPendingRequests);

			uint8_t recvValue;
			size_t recvBytes = fs_recv(aClient->m_socket, &recvValue, 1, 0);
			if(recvBytes == 0 || (recvBytes == SIZE_MAX && errno != EAGAIN))
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

					aClient->m_recvBufferBytes = 0;
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

	time_t startTime = time(NULL);

	for(;;)
	{
		time_t currentTime = time(NULL);
		
		uint32_t secondsPassed = currentTime > startTime ? (uint32_t)(currentTime - startTime) : 0;
		if(secondsPassed > 6)
			break;

		for (size_t i = 0; i < NUM_SERVERS; i++)
			server_update(servers[i]);

		for (size_t i = 0; i < NUM_CLIENTS; i++)
			client_update(clients[i], i);		
	}

	for (size_t i = 0; i < NUM_SERVERS; i++)
		server_destroy(servers[i]);

	for (size_t i = 0; i < NUM_CLIENTS; i++)
		client_destroy(clients[i]);

	fs_uninit();
}