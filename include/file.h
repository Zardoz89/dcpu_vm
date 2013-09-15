#ifndef __FILE__H_
#define __FILE__H_ 1

#include <stdio.h>
#include <stdlib.h>
#include "config.hpp"

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

#endif // __FILE__H_
