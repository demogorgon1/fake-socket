#include "fs_mem.h"
#include "fs_port_table.h"

fs_port_table* 
fs_port_table_create()
{	
	fs_port_table* t = FS_NEW(fs_port_table);

	t->m_nextEphemeralPort = FS_EPHEMERAL_PORT_RANGE_MIN;

	return t;
}

void					
fs_port_table_destroy(
	fs_port_table*				aPortTable)
{
	if(aPortTable != NULL)
		free(aPortTable);
}

fs_port_table_entry*
fs_port_table_get_entry(
	fs_port_table*				aPortTable,
	uint16_t					aPort)
{
	return &aPortTable->m_entries[aPort];
}

int								
fs_port_table_get_listening_socket(
	fs_port_table*				aPortTable,
	uint16_t					aPort)
{
	const fs_port_table_entry* t = &aPortTable->m_entries[aPort];
	if(t->m_socket == 0 || !t->m_listening)
		return -1;
	return t->m_socket;
}

uint16_t						
fs_port_table_next_ephemeral(
	fs_port_table*				aPortTable)
{
	uint16_t port = 0;

	for(uint32_t i = FS_EPHEMERAL_PORT_RANGE_MIN; i < FS_EPHEMERAL_PORT_RANGE_MAX && port == 0; i++)
	{
		fs_port_table_entry* t = &aPortTable->m_entries[aPortTable->m_nextEphemeralPort];

		if(t->m_socket == 0)
			port = aPortTable->m_nextEphemeralPort;
			
		if(aPortTable->m_nextEphemeralPort == FS_EPHEMERAL_PORT_RANGE_MAX)
			aPortTable->m_nextEphemeralPort = FS_EPHEMERAL_PORT_RANGE_MIN;
		else
			aPortTable->m_nextEphemeralPort++;
	}

	return port;
}

