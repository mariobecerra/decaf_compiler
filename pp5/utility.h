

#ifndef _H_utility
#define _H_utility

#include <stdlib.h>
#include <stdio.h>


void Failure(const char *format, ...);


#define Assert(expr)  \
  ((expr) ? (void)0 : Failure("Assertion failed: %s, line %d:\n    %s", \
      __FILE__, __LINE__, #expr))


void PrintDebug(const char *key, const char *format, ...);


void SetDebugForKey(const char *key, bool val);


bool IsDebugOn(const char *key);


void ParseCommandLine(int argc, char *argv[]);

#endif

