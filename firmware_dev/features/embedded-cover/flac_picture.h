/* Bounded metadata-only read: native hints can be zero for large pictures.
 * Skip payloads with seek; no image/audio allocation or music-file writes. */
extern long lseek(int,long,int);
static unsigned cover_flac_picture_bytes(const char *path){
 int fd=open(path,0);if(fd<0)return 0;unsigned char b[8];unsigned result=0,position=4;
 if(read(fd,b,4)!=4||b[0]!='f'||b[1]!='L'||b[2]!='a'||b[3]!='C')goto done;
 for(unsigned blocks=0;blocks<256;blocks++){
  if(read(fd,b,4)!=4)break;
  unsigned last=b[0]&128,type=b[0]&127,n=(unsigned)b[1]<<16|(unsigned)b[2]<<8|b[3];position+=4;
  if(position>33554432u||n>33554432u-position)break;unsigned end=position+n;
  if(type==6&&n>=32){
   if(read(fd,b,8)!=8)break;unsigned mime=cover_be32(b+4);
   if(mime<=n-32&&lseek(fd,(long)mime,1)>=0&&read(fd,b,4)==4){
    unsigned description=cover_be32(b);
    if(description<=n-32-mime&&lseek(fd,(long)description+16,1)>=0&&read(fd,b,4)==4){
     unsigned bytes=cover_be32(b);
     if(bytes<=n-32-mime-description&&cover_large_hint(bytes)){result=bytes;break;}
    }
   }
  }
  if(last||lseek(fd,(long)end,0)!=(long)end)break;position=end;
 }
done:close(fd);return result;
}
