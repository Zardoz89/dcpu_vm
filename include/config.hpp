#ifndef __CONFIG
#define __CONFIG

#if defined(__MINGW32__) || defined(__MINGW64__)

#define __MINGW__
#define __NO_THREAD_11__ 
#define __NO_TOSTRING_11__
#include <SFML/System.hpp>
#include <cstdio>

#endif
#else

#include <thread> //not working with mingw 4.7 need to wait 4.8

#endif
