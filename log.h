#ifndef LOG_H
#define LOG_H

#ifdef DEBUG
#define DEBUG_LOG(...) fprintf(stdout, "(DEBUG) " __VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

#ifdef RELEASE
#define INFO_LOG(...)
#define ERROR_LOG(...)
#else
#define INFO_LOG(...)  fprintf(stdout, "" __VA_ARGS__)
#define ERROR_LOG(...) fprintf(stderr, "(ERROR) " __VA_ARGS__)
#endif

#endif
