/* Existing blocking tinyalsa writer, stereo S32 frames. No copied queue,
 * marker regeneration, PCM reconfiguration or automatic recovery here. */
struct SplitWriteStats {
 unsigned blocks,calls,frames,partials,zero_results,errors,invalid_results;
 unsigned incomplete_frames,max_request_frames,fault;
 int last_result;
};
static struct SplitWriteStats split_stats;
static int split_write_block(void *pcm,const void *data,unsigned bytes,
                            int (*writer)(void *,const void *,unsigned)){
 const unsigned char *p=data;unsigned remaining=bytes/8,total=remaining;
 split_stats.blocks++;
 while(remaining){
  unsigned count=remaining>512?512:remaining;
  if(count>split_stats.max_request_frames)split_stats.max_request_frames=count;
  int rc=writer(pcm,p,count*8);split_stats.calls++;split_stats.last_result=rc;
  /* Stock pcm_writei can return 0 after prepare/recovery while its WRITEI
   * has already transferred data. Never interpret it as safe to retry.
   * Future calls use the original writer after an ambiguous/error result. */
  if(rc<=0||(unsigned)rc>count){
   if(rc<0)split_stats.errors++;
   else if(!rc)split_stats.zero_results++;
   else split_stats.invalid_results++;
   split_stats.incomplete_frames+=remaining;split_stats.fault=1;
   return rc>(int)count?-5:rc;
  }
  unsigned accepted=(unsigned)rc;
  if(accepted<count)split_stats.partials++;
  split_stats.frames+=accepted;p+=accepted*8;remaining-=accepted;
 }
 return (int)total;
}
