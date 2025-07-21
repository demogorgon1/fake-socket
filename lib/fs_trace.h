#ifndef __FS_TRACE_H__
#define __FS_TRACE_H__

#ifndef FS_TRACE_ENABLED
#define FS_TRACE_ENABLED		0
#endif

#if FS_TRACE_ENABLED
	#define FS_TRACE(...) fs_trace(__VA_ARGS__);

	void	fs_trace(
				const char*		aFormat,
				...);
#else
	#define FS_TRACE(...) do { } while(0)
#endif

#endif