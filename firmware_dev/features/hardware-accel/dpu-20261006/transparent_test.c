/* Bounded transparent-layer test. Uses only driver-owned fb1 RAM. */
extern int open(const char *,int,...),close(int),ioctl(int,unsigned,...),printf(const char *,...),usleep(unsigned),memcmp(const void *,const void *,unsigned);
extern void *mmap(void *,unsigned,int,int,int,long),*memcpy(void *,const void *,unsigned),*memset(void *,int,unsigned);
extern int munmap(void *,unsigned);
static unsigned original[72],trial[72],current[72],fix[32];
static unsigned char saved[1555200];
int main(void){
 int fd=open("/dev/fb0",2),layer=-1,result=1,armed=0;void *pixels=(void*)-1;
 if(fd<0)return 2;
 if(ioctl(fd,0xc0044702u,original))goto end;
 if(original[0]!=0x003fc119||original[18]!=0x003fc110||original[36]!=0x003fc108||original[54]!=0x003fc100){printf("unrecognized configuration; skipped\n");goto end;}
 layer=open("/dev/fb1",2);if(layer<0)goto end;
 if(ioctl(layer,0x4602,fix)||fix[4]!=0x079fa400||fix[5]!=sizeof saved||fix[11]!=1440)goto end;
 pixels=mmap(0,sizeof saved,3,1,layer,0);if(pixels==(void*)-1)goto end;
 memcpy(saved,pixels,sizeof saved);memset(pixels,0,sizeof saved);__sync_synchronize();
 memcpy(trial,original,sizeof trial);trial[0]=(trial[0]&~(7u<<3))|(2u<<3);
 trial[18]=(trial[18]&~((7u<<3)|(15u<<6)))|(3u<<3)|(5u<<6)|1u;
 if(ioctl(fd,0xc0044701u,trial))goto end;armed=1;
 usleep(400000);
 if(ioctl(fd,0xc0044702u,current)==0&&memcmp(current,trial,sizeof trial)==0){printf("transparent_layer_config=accepted stable=1\n");result=0;}else printf("configuration changed during test\n");
end:
 if(armed){if(ioctl(fd,0xc0044702u,current)==0&&memcmp(current,trial,sizeof trial)==0){if(ioctl(fd,0xc0044701u,original)){printf("cleanup_failed\n");return 8;}usleep(100000);printf("original_configuration_restored=1\n");}else{printf("external_config_change: buffer retained, no overwrite\n");return 9;}}
 if(pixels!=(void*)-1){memcpy(pixels,saved,sizeof saved);munmap(pixels,sizeof saved);}if(layer>=0)close(layer);close(fd);return result;
}
__asm__(".text\n.set noreorder\n.globl _start\n_start:\naddiu $sp,$sp,-32\njal main\nnop\nmove $a0,$v0\nli $v0,4001\nsyscall\n");

