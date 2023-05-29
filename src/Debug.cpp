#include "Debug.h"
#include <bits/stdc++.h>
const int debug_level = -1;

void Debug(int level, const char* format, ...){
    if(level >= debug_level && debug_level != -1 || level == -1){
        va_list args;
        va_start(args, format);
        printf("%d#", level);
        vprintf(format, args);
        va_end(args);
    }
}


bool isShowableChar(char c){
    if(c >= 32 && c <= 126 && c != '\n') return true;
    return false;
}
void DebugHex(int level, unsigned char *buf, int size)
{
    if(level >= debug_level && debug_level != -1 || level == -1)
        for(int i = 0; i < size; i+=16)
            printf("%02x %02x %02x %02x %02x %02x %02x %02x - %02x %02x %02x %02x %02x %02x %02x %02x %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", (i<size?buf[i]:0), (i+1<size?buf[i+1]:0), (i + 2 < size ? buf[i + 2] : 0), (i + 3 < size ? buf[i + 3] : 0), (i + 4 < size ? buf[i + 4] : 0), (i + 5 < size ? buf[i + 5] : 0), (i + 6 < size ? buf[i + 6] : 0), (i + 7 < size ? buf[i + 7] : 0), (i + 8 < size ? buf[i + 8] : 0), (i + 9 < size ? buf[i + 9] : 0), (i + 10 < size ? buf[i + 10] : 0), (i + 11 < size ? buf[i + 11] : 0), (i + 12 < size ? buf[i + 12] : 0), (i + 13 < size ? buf[i + 13] : 0), (i + 14 < size ? buf[i + 14] : 0), (i + 15 < size ? buf[i + 15] : 0), (i < size && isShowableChar(buf[i]) ? buf[i] : 0), 
            (i + 1 < size && isShowableChar(buf[i + 1]) ? buf[i + 1] : ' '), 
            (i + 2 < size && isShowableChar(buf[i + 2]) ? buf[i + 2] : ' '), 
            (i + 3 < size && isShowableChar(buf[i + 3]) ? buf[i + 3] : ' '), 
            (i + 4 < size && isShowableChar(buf[i + 4]) ? buf[i + 4] : ' '), 
            (i + 5 < size && isShowableChar(buf[i + 5]) ? buf[i + 5] : ' '), 
            (i + 6 < size && isShowableChar(buf[i + 6]) ? buf[i + 6] : ' '), 
            (i + 7 < size && isShowableChar(buf[i + 7]) ? buf[i + 7] : ' '), 
            (i + 8 < size && isShowableChar(buf[i + 8]) ? buf[i + 8] : ' '), 
            (i + 9 < size && isShowableChar(buf[i + 9]) ? buf[i + 9] : ' '), 
            (i + 10 < size && isShowableChar(buf[i + 10]) ? buf[i + 10] : ' '), 
            (i + 11 < size && isShowableChar(buf[i + 11]) ? buf[i + 11] : ' '), 
            (i + 12 < size && isShowableChar(buf[i + 12]) ? buf[i + 12] : ' '), 
            (i + 13 < size && isShowableChar(buf[i + 13]) ? buf[i + 13] : ' '), 
            (i + 14 < size && isShowableChar(buf[i + 14]) ? buf[i + 14] : ' '), 
            (i + 15 < size && isShowableChar(buf[i + 15]) ? buf[i + 15] : ' '));
}
