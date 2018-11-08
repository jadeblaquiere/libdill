/*

  Copyright (c) 2018 Joseph deBlaquiere

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#include <libdill.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define CHUNKSZ (4096)
#define NCHUNKS (4096)

int main(int argc, char **argv) {
    uint8_t chunk[CHUNKSZ];
    FILE *fptr = fopen("/dev/urandom", "rb");
    if(!fptr) {fprintf(stderr,"source: can't open /dev/urandom\n"); return 1;}
    size_t rsz = fread(chunk, sizeof(uint8_t), CHUNKSZ, fptr);
    if(rsz != CHUNKSZ) {fprintf(stderr,"source: short read from urandom\n"); return 1;}
    struct ipaddr addr;
    int err = ipaddr_remote(&addr, "127.0.0.1", 5550, IPADDR_PREF_IPV6, now() + 5000);
    if(err) {fprintf(stderr, "source: nslookup localhost failed\n"); return 1;}
    int s = tcp_connect(&addr, now()+5000);
    if(s < 0) {fprintf(stderr, "source: connect failed\n"); return 1;}
    int64_t bnow = now();
    clock_t bclk = clock();
    for(int i = 0; i < NCHUNKS; i++) {
        err = bsend(s, chunk, rsz, -1);
        if(err) {fprintf(stderr, "source: write error\n"); return 1;}
    }
    int64_t anow = now();
    clock_t aclk = clock();
    double walltm = ((double)(anow - bnow))/(1000.0);
    double cputm = ((double)(aclk - bclk))/((double)CLOCKS_PER_SEC);
    printf("elapsed time %f\n", walltm);
    printf("cpu time %f\n", cputm);
    return 0;
}
