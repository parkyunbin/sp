//------------------------------------------------------------------------------
//
// memtrace
//
// trace calls to the dynamic memory manager
//
#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memlog.h>
#include <memlist.h>
#include "callinfo.h"

//
// function pointers to stdlib's memory management functions
//
static void *(*mallocp)(size_t size) = NULL;
static void (*freep)(void *ptr) = NULL;
static void *(*callocp)(size_t nmemb, size_t size);
static void *(*reallocp)(void *ptr, size_t size);



//
// statistics & other global variables
//
static unsigned long n_malloc  = 0;
static unsigned long n_calloc  = 0;
static unsigned long n_realloc = 0;
static unsigned long n_allocb  = 0;
static unsigned long n_freeb   = 0;
static item *list = NULL;

//
// init - this function is called once when the shared library is loaded
//
__attribute__((constructor))
void init(void)
{
  char *error;

  LOG_START();

  // initialize a new list to keep track of all memory (de-)allocations
  // (not needed for part 1)
  list = new_list();

  // ...
}


//malloc wrapper function
void *malloc(size_t size){
  mallocp = dlsym(RTLD_NEXT, "malloc"); /*get address of libc malloc*/
  void *res = mallocp(size); /*call libc malloc*/
  LOG_MALLOC((int)size, res); 
  //printf("malloc(%d) = %p\n", (int)size, res);
  alloc(list, res, size); //existing block -> update size & reference counter
  n_malloc++;
  n_allocb += size;
  return res;
}

//free wrapper function
void free(void *ptr){
  if(!ptr) return;
  freep = dlsym(RTLD_NEXT, "free"); /*get address of libc free*/
  freep(ptr); /*call libc free*/
  
  item *temp = find(list, ptr);
  if(temp!= NULL){
    dealloc(list, ptr);
    n_freeb += (*temp).size;
  }
  LOG_FREE(ptr);
  //printf("free(%p)\n", ptr); 
}

//calloc wrapper function
void *calloc(size_t nmemb, size_t size){
  callocp = dlsym(RTLD_NEXT, "calloc");
  void *res = callocp(nmemb, size);
  LOG_CALLOC((int)nmemb, (int)size, res);
  //printf("calloc(%d, %d) = %p\n", (int)nmemb, (int)size, res);
  alloc(list, res, nmemb*size); //existing block -> update size & reference counter
  n_calloc++;
  n_allocb += ((int)size * nmemb);
  return res;
}

//realloc wrapper function
void *realloc(void *ptr, size_t size){
  reallocp = dlsym(RTLD_NEXT, "realloc");
  int origSize = -1;
  item *prev = list;
  while(prev != NULL){
    if((*prev).ptr == ptr){
      origSize = (*prev).size;
      break;
    }
  prev = prev->next;
  }
  void *res;
  if(origSize == -1){
    res = reallocp(NULL, size);
    alloc(list, res, size);
  }
  else if((origSize - (int)size) >= 0){
    n_freeb += (origSize - (int)size);
    res = reallocp(ptr, size);
    dealloc(list, ptr);
    alloc(list, res, size);
  }
  else{
    res = reallocp(ptr, size);
    dealloc(list, ptr);
    alloc(list, res, size);
  }
  LOG_REALLOC(ptr, size, res);
  //printf("realloc(%p, %d) = %p\n", ptr, (int)size, res);
  n_realloc++;
  n_allocb += size;
  return res;
}

//
// fini - this function is called once when the shared library is unloaded
//
__attribute__((destructor))
void fini(void)
{
  // ...

  LOG_STATISTICS(n_allocb, n_allocb/(n_malloc + n_calloc + n_realloc), n_freeb);

  int nonfreed_start = 1;
  item *prev = list;
  while(prev != NULL){
    if((*prev).cnt != 0){
      if(nonfreed_start){
        nonfreed_start = 0;
        LOG_NONFREED_START();
      }
      LOG_BLOCK((*prev).ptr, (*prev).size, (*prev).cnt, (*prev).fname, (*prev).ofs);
    }
    prev = (*prev).next;
  }

  LOG_STOP();

  // free list (not needed for part 1)
  free_list(list);
}

// ...
