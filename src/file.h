#ifndef __FILE
#define __FILE
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

//From my personal collection :p

#ifdef __cplusplus
extern "C" {
#endif

int fsize(FILE *fp);
void fwritetext(const char* filename, const char* text);
void fswitchendian(uint16_t* f, unsigned int size);

#ifdef __cplusplus
}
#endif
#endif
