#ifndef DEBUG_H
#define DEBUG_H
void Debug(int level, const char* format, ...);
void DebugHex(int level, unsigned char *buf, int size);
#endif