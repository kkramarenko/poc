#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define UNLIKELY(x) (__builtin_expect(!!(x), 0))

#define BASE7_MASK ((1<<7) - 1)
#define BASE7_SHIFT 7
#define BASE7_CONT_FLAG (1<<7)

int pack_size(uint64_t size, uint8_t out_buf[9])
{
    uint64_t tmp = size;
    int idx = 0;
    do {
        uint8_t val = tmp & BASE7_MASK;
        tmp >>= BASE7_SHIFT;
        if( UNLIKELY(tmp) ) {
            val |= BASE7_CONT_FLAG;
        }
        out_buf[idx++] = val;
    } while(tmp && idx < 8);

    /* If we have leftover (VERY unlikely) */
    if (UNLIKELY(8 == idx && tmp)) {
        out_buf[idx++] = tmp;
    }
    return idx;
}

int unpack_size(uint8_t in_buf[], uint64_t *out_size)
{
    uint64_t size = 0, shift = 0;
    int idx = 0;
    uint8_t val = 0;
    do {
        val = in_buf[idx++];
        size = size + (((uint64_t)val & BASE7_MASK) << shift);
        shift += BASE7_SHIFT;
    } while( UNLIKELY((val & BASE7_CONT_FLAG) && (idx < 8)) );

    /* If we have leftover (VERY unlikely) */
    if (UNLIKELY(8 == idx && (val & BASE7_CONT_FLAG))) {
        val = in_buf[idx++];
        size = size + ((uint64_t)val << shift);
    }
    *out_size = size;
    return idx;
}


int main()
{
    int i;
    uint64_t size;
    size_t buffer_sizes[] = { 16, 256, 4096, 16*1024, 1024*1024, 128*1024*1024, 1024*1024*1024 };
    int cnt = sizeof(buffer_sizes) / sizeof(buffer_sizes[0]);


    printf("Flexible storage allocation for different numbers:\n");
    for(i = 0; i < cnt; i++) {
        uint8_t buf[9];
        size = buffer_sizes[i];
        int s = pack_size(size, buf);
        printf("%zu: %d\n", size, s);
    }

    for(size = 0; size < 100000000000; size++) {
        uint8_t buf[9];
        uint64_t nsize;
        int ps = pack_size(size, buf);
        int us = unpack_size(buf, &nsize);
        if( ps != us){
            abort();
        }
        if( nsize != size){
            abort();
        }
        if( 0 == (size%100000) ){
            printf("1: size = %lu\n", size);
        }
    }

    for(size = (uint64_t)(-1) - 100000000000; size < (uint64_t)(-1); size++) {
        uint8_t buf[9];
        uint64_t nsize;
        int ps = pack_size(size, buf);
        int us = unpack_size(buf, &nsize);
        if (ps != us){
            abort();
        }
        if( nsize != size){
            abort();
        }

        if( 0 == (size%100000) ){
            printf("1: size = %lu\n", size);
        }
    }

    printf("OK\n");
    return 0;
}
