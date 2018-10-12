#define _GNU_SOURCE
#include "cmap.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>

#define FUNC(name) (name) 
#define ORIGINAL(name) original_##name 
#define GET_ORIGINAL(ret, name, ...)                        \
    static ret (*ORIGINAL(name))(__VA_ARGS__) = NULL;       \
    if (!ORIGINAL(name)) {                                  \
      *(void**)(&ORIGINAL(name)) = dlsym(RTLD_NEXT, #name); \
    }                                                       \
    if (!ORIGINAL(name)) {                                  \
        errno = ENOSYS;                                     \
        abort();                                            \
    }                                                       \

#define BUFF_SIZE 256
#define MIN(a,b) (((a)<(b))?(a):(b))

void rep_strat(const void* buf, unsigned char* dat, size_t count, size_t* newcount){
    unsigned char* old_dat = (unsigned char*)buf;
    int j = 0;
    for (size_t i = 0; i < count; i++) {
        if (old_dat[i] != '\n' && old_dat[i] != '\0'){
            for(int k = 0; k < 5; j++,k++){
                dat[j] = CHAR_MAP[(int)old_dat[i]][k%5];
            }
        } else {
           dat[j] = old_dat[i]; 
           j++;
        }
    }
    *newcount = j;
}

ssize_t FUNC(write)(int fd, const void* buf, size_t count) {
    if (!count) return 0;

    GET_ORIGINAL(ssize_t, write, int, const void *, size_t);

    size_t newcount;
    ssize_t written;
    size_t write_amount;
    unsigned char* new_buf = (unsigned char *) malloc(BUFF_SIZE*5);
    if (new_buf == NULL){
        printf("Out of memory -- stopping");
    }
    for(unsigned int i = 0; i < count/BUFF_SIZE + 1; i++){
        write_amount = MIN(BUFF_SIZE, count - BUFF_SIZE*i - 1);
        rep_strat(buf + BUFF_SIZE*i, new_buf, write_amount, &newcount);
        written = ORIGINAL(write)(fd, new_buf, newcount);
    }

    free(new_buf);
    return written;
}

//ssize_t FUNC(fwrite)(const void *data, size_t size, size_t count, FILE* stream) {
//    if (size * count == 0) return 0;
//
//    size_t result;
//    int fd = fileno(stream);
//
//    GET_ORIGINAL(ssize_t, fwrite, const void*, size_t, size_t, FILE*); 
//
//    unsigned char* new_data = (unsigned char *) malloc(count*size); 
//    rep_strat(new_data, count*size);
//
//    result = ORIGINAL(fwrite)(new_data, size, count, stream);
//    free(new_data);
//    return result;
//}
