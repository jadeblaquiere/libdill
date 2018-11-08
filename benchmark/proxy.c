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
    if(argc != 2) {fprintf(stderr,"usage: proxy <rsize>\n"); return 1;}
    int ri;
    int c = sscanf(argv[1], "%d", &ri);
    if(c != 1) {fprintf(stderr,"usage: proxy <rsize>\n"); return 1;}
    size_t rsz = ri;
    int xfer = NCHUNKS * CHUNKSZ;
    struct ipaddr addr;
    int err = ipaddr_remote(&addr, "127.0.0.1", 5555, IPADDR_PREF_IPV4, -1);
    if(err) {fprintf(stderr, "proxy: nslookup localhost failed\n"); return 1;}
    int so = tcp_connect(&addr, now()+5000);
    if(so < 0) {fprintf(stderr, "proxy: connect failed\n"); return 1;}
    err = ipaddr_local(&addr, NULL, 5550, IPADDR_PREF_IPV6);
    if(err) {fprintf(stderr, "proxy: local lookup NULL failed\n"); return 1;}
    int l = tcp_listen(&addr, 10);
    if(l < 0) {fprintf(stderr, "proxy: listen failed\n"); return 1;}
    int si = tcp_accept(l, NULL, -1);
    if(si < 0) {fprintf(stderr, "sink: accept failed\n"); return 1;}
    int blocks = NCHUNKS * (CHUNKSZ/rsz);
    printf("proxy: %d blksz, ", ri);
    printf("%d blocks, ", blocks);
    int64_t bnow = now();
    clock_t bclk = clock();
    for(int i = 0; i < blocks; i++) {
        err = brecv(si, chunk, rsz, -1);
        if(err) {fprintf(stderr, "proxy: read error\n"); return 1;}
        err = bsend(so, chunk, rsz, -1);
        if(err) {fprintf(stderr, "proxy: write error\n"); return 1;}
    }
    int64_t anow = now();
    clock_t aclk = clock();
    double walltm = ((double)(anow - bnow))/(1000.0);
    double cputm = ((double)(aclk - bclk))/((double)CLOCKS_PER_SEC);
    printf("elapsed time %f, ", walltm);
    printf("cpu time %f\n", cputm);
    return 0;
}
