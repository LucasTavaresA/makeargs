#ifndef LOG_C
#define LOG_C

#ifndef LOG_PRINTF
#define LOG_PRINTF(format, ...) printf(format, ##__VA_ARGS__);
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(format, ...) fprintf(stderr, format, ##__VA_ARGS__);
#endif

#ifndef LOG_ERROR_CODE
#define LOG_ERROR_CODE 1
#endif

#ifndef LOG_EXIT
#define LOG_EXIT(status) exit(status);
#endif

#define LOG_HALT(status, format, ...)    \
	LOG_ERROR(format "\n", ##__VA_ARGS__); \
	LOG_EXIT(status);

#define LOG_ASSERT(cond, format, ...)                \
	if (!(cond))                                       \
	{                                                  \
		LOG_HALT(LOG_ERROR_CODE, format, ##__VA_ARGS__); \
	}

#endif	// LOG_C
