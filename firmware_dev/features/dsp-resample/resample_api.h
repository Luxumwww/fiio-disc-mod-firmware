#ifndef DISC_RESAMPLE_API_H
#define DISC_RESAMPLE_API_H
typedef int (*DiscResampleProcessFloat)(void *,float *,unsigned);
typedef struct {
 unsigned abi;
 void *(*create)(unsigned,unsigned);
 void (*destroy)(void *);
 int (*process)(void *,int *,unsigned,DiscResampleProcessFloat,void *);
 void (*stats)(void *,unsigned *,unsigned *,unsigned *,unsigned *);
 void (*diagnostics)(void *,unsigned *,float *,float *);
} DiscResampleApi;
#endif
