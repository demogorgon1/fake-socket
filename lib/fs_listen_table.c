#include "fs_listen_table.h"

fs_listen_table* 
fs_listen_table_create()
{
	return FS_NEW(fs_listen_table);
}

void					
fs_listen_table_destroy(
	fs_listen_table*			aListenTable)
{
	if(aListenTable != NULL)
		free(aListenTable);
}

int						
fs_listen_table_get(
	fs_listen_table*			aListenTable,
	uint16_t					aPort)
{
	int s = aListenTable->m_sockets[aPort];
	if(s == 0)
		return -1;
	return s;
}

