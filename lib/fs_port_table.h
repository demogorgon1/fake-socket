#ifndef __fs_port_table_H__
#define __fs_port_table_H__

#include <fs/fs_base.h>

typedef struct _fs_port_table_entry
{
	int						m_socket;
	fs_bool					m_listening;
} fs_port_table_entry;

typedef struct _fs_port_table
{
	fs_port_table_entry		m_entries[(size_t)UINT16_MAX + 1];
	uint16_t				m_nextEphemeralPort;
} fs_port_table;

fs_port_table*					fs_port_table_create();
void							fs_port_table_destroy(				
									fs_port_table*						aPortTable);
fs_port_table_entry*			fs_port_table_get_entry(
									fs_port_table*						aPortTable,
									uint16_t							aPort);
int								fs_port_table_get_listening_socket(
									fs_port_table*						aPortTable,
									uint16_t							aPort);
int								fs_port_table_get_socket(
									fs_port_table*						aPortTable,
									uint16_t							aPort);
uint16_t						fs_port_table_next_ephemeral(
									fs_port_table*						aPortTable);

#endif
