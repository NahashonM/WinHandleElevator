
#include <iostream>
#include <sstream>
#include <string>


#include "WinHandleEvelevator.h"


template <typename T>
void LogMessageRaw(WHE_LOG_LEVEL loglevel, T format, ...) {

	std::ostringstream buffer;

	buffer << "[";
	
	switch (loglevel)
	{
	case WHE_LOG_INFO:
		buffer << "   INFO";
		break;
	case WHE_LOG_WARNING:
		buffer << "WARNING";
		break;
	default:
		buffer << "  ERROR";
		break;
	}

	buffer << "]: ";
	buffer << format;

	va_list args;
	__crt_va_start(args, format);
	vprintf(buffer.str().c_str(), args );
	__crt_va_end(args);

}



template void LogMessageRaw<const char*>(WHE_LOG_LEVEL, const char*, ...);
template void LogMessageRaw<int>(WHE_LOG_LEVEL, int, ...);
template void LogMessageRaw<DWORD>(WHE_LOG_LEVEL, DWORD, ...);
