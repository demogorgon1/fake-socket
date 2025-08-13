#include <fs/fs_base.h>

#include <stdarg.h>

#include "fs_trace.h"

#if FS_TRACE_ENABLED

void	
fs_trace(
	const char* aFormat,
	...)
{
	char buffer[2048];
	va_list list;
	va_start(list, aFormat);
	int n;

	#if defined(WIN32)
		n = vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, aFormat, list);		
	#else
		n = vsnprintf(buffer, sizeof(buffer), aFormat, list);
	#endif

	if (n < 0)
		buffer[0] = '\0';
	va_end(list);

	fprintf(stdout, "*** %s\n", buffer);
}

#endif
