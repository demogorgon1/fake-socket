#include "fs_mutex.h"

void
fs_mutex_init(
	fs_mutex* aMutex)
{
	#if defined(WIN32)
		InitializeCriticalSection(&aMutex->m_cs);
	#else
		int result = pthread_mutex_init(&aMutex->m_mutex, NULL);
		assert(result == 0);
		(void)result;
	#endif
}

void		
fs_mutex_uninit(
	fs_mutex* aMutex)
{
	#if defined(WIN32)
		DeleteCriticalSection(&aMutex->m_cs);
	#else
		int result = pthread_mutex_destroy(&aMutex->m_mutex);
		assert(result == 0);
		(void)result;
	#endif	
}

void		
fs_mutex_lock(
	fs_mutex* aMutex)
{
	#if defined(WIN32)
		EnterCriticalSection(&aMutex->m_cs);
	#else
		int result = pthread_mutex_lock(&aMutex->m_mutex);
		assert(result == 0);
		(void)result;
	#endif	
}

void		
fs_mutex_unlock(
	fs_mutex* aMutex)
{
	#if defined(WIN32)
		LeaveCriticalSection(&aMutex->m_cs);
	#else
		int result = pthread_mutex_unlock(&aMutex->m_mutex);
		assert(result == 0);
		(void)result;
	#endif	
}

