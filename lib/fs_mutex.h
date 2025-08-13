#ifndef __FS_MUTEX_H__
#define __FS_MUTEX_H__

#include <fs/fs_base.h>

typedef struct _fs_mutex
{
	#if defined(WIN32)
		CRITICAL_SECTION		m_cs;
	#else
		pthread_mutex_t			m_mutex;
	#endif
} fs_mutex;

void		fs_mutex_init(
				fs_mutex*		aMutex);
void		fs_mutex_uninit(
				fs_mutex*		aMutex);
void		fs_mutex_lock(
				fs_mutex*		aMutex);
void		fs_mutex_unlock(
				fs_mutex*		aMutex);

#endif
