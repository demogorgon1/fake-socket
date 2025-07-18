#ifndef __FS_LISTEN_TABLE_H__
#define __FS_LISTEN_TABLE_H__

#include <fs/fs_base.h>

typedef struct _fs_listen_table
{
	int				m_sockets[(size_t)UINT16_MAX + 1];
} fs_listen_table;

fs_listen_table*		fs_listen_table_create();
void					fs_listen_table_destroy(				
							fs_listen_table*					aListenTable);
int						fs_listen_table_get(
							fs_listen_table*					aListenTable,
							uint16_t							aPort);

#endif