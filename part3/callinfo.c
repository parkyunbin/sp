#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<string.h>
#include <errno.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

int get_callinfo(char *fname, size_t fnlen, unsigned long long *ofs)
{
  unw_cursor_t cursor;
  unw_context_t context;
  
  return -1;
}
