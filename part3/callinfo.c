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
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

 char proc[256];
 while (unw_step(&cursor) > 0) {
        long long unsigned int pri = *ofs;
        unw_get_proc_name(&cursor, proc, 256, (unw_word_t *) ofs);
        if(strcmp(proc, "main") == 0)
        {
            strcpy(fname, proc);
            *ofs -= 5;
            return 0;
        }
  }
  return -1;
}
