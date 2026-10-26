#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "cover_policy.h"
#include "flac_picture.h"
static const char *path="/tmp/disc-cover-metadata-test.flac";
int main(int argc,char **argv){
 unsigned char b[40]={ 'f','L','a','C', 134,0,0,32 };
 /* Picture prefix: type3, empty MIME/description, width/height=1,
  * encoded payload length. The declared block must contain that payload. */
 b[11]=3;b[23]=1;b[27]=1;unsigned payload=3691554,total=32+payload;
 b[5]=total>>16;b[6]=total>>8;b[7]=total;
 b[36]=payload>>24;b[37]=payload>>16;b[38]=payload>>8;b[39]=payload;
 int fd=open(path,O_CREAT|O_TRUNC|O_WRONLY,0600);assert(fd>=0&&write(fd,b,40)==40);assert(!ftruncate(fd,8+total));close(fd);
 assert(cover_flac_picture_bytes(path)==payload);
 fd=open(path,O_WRONLY);assert(fd>=0);b[39]=0;assert(pwrite(fd,b+36,4,36)==4);close(fd);assert(cover_flac_picture_bytes(path)==3691520);
 fd=open(path,O_WRONLY);assert(fd>=0);b[12]=127;assert(pwrite(fd,b+12,4,12)==4);close(fd);assert(!cover_flac_picture_bytes(path));
 for(unsigned n=0;n<40;n++){fd=open(path,O_WRONLY|O_TRUNC);assert(fd>=0&&write(fd,b,n)==(int)n);close(fd);assert(!cover_flac_picture_bytes(path));}
 unlink(path);assert(!cover_flac_picture_bytes(path));
 if(argc>1)assert(cover_flac_picture_bytes(argv[1])==3691554);
 puts("PASS: production FLAC metadata scanner, actual large-picture length without reading payload, bad lengths/truncation/missing file and exact current song");
}
