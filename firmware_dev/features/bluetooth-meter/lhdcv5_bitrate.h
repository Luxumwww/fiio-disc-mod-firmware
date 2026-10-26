/* Passive V5 lossy stereo 5ms frame budgets. The frame header carries the
 * byte budget, not the phone's Auto/fixed preference. Match exact encoder
 * rounding (floor(kbps*5/16) bytes per channel); never map an upper limit. */
static unsigned v5_nominal_kbps,v5_candidate,v5_matches,v5_remaining,v5_header_used;
static unsigned v5_ssrc,v5_sequence,v5_synced,v5_last_frame,v5_packet_at;
static unsigned v5_restarts,v5_gaps,v5_invalid_headers,v5_ignored;
static unsigned char v5_header[2];
static void v5_bitrate_reset(void){
 v5_nominal_kbps=v5_candidate=v5_matches=v5_remaining=v5_header_used=0;
 v5_ssrc=v5_sequence=v5_synced=v5_last_frame=v5_packet_at=0;
}
static unsigned v5_budget_kbps(unsigned payload){
 static const unsigned modes[]={64,128,160,192,200,240,256,260,320,390,400,480,500,520,600,700,800,900,1000};
 for(unsigned i=0;i<sizeof(modes)/sizeof(modes[0]);i++)if(2*(modes[i]*5/16)==payload)return modes[i];
 return 0;
}
static unsigned v5_be32(const unsigned char *p){return (unsigned)p[0]<<24|(unsigned)p[1]<<16|(unsigned)p[2]<<8|p[3];}
static void v5_bitrate_packet(const unsigned char *rtp,const unsigned char *payload,unsigned n,unsigned t){
 /* Pause/restart can reuse SSRC but restart the RTP sequence. Only a fresh
  * accepted packet advances this timer: rejecting every packet must not keep
  * an old sequence baseline alive forever. Reconfirm actual frame budgets. */
 if(v5_synced&&t-v5_packet_at>=1500){v5_bitrate_reset();v5_restarts++;}
 unsigned seq=(unsigned)rtp[2]<<8|rtp[3],ssrc=v5_be32(rtp+8);
 if(v5_synced&&ssrc==v5_ssrc){
  unsigned delta=(seq-v5_sequence)&65535;if(!delta||delta>=32768){v5_ignored++;return;}
  if(delta!=1){v5_bitrate_reset();v5_gaps++;}
 }else if(v5_synced)v5_bitrate_reset();
 v5_synced=1;v5_sequence=seq;v5_ssrc=ssrc;v5_packet_at=t;
 while(n){
  if(!v5_remaining){
   while(n&&v5_header_used<2){v5_header[v5_header_used++]=*payload++;n--;}
   if(v5_header_used<2)break;
   unsigned h=(unsigned)v5_header[0]|(unsigned)v5_header[1]<<8;
   unsigned bytes=2*(h&1023);v5_header_used=0;
   /* Published receiver accepts only stereo lossy 5ms. A zero/reserved
    * budget or unknown mode cannot produce a plausible preset label. */
   unsigned mode=v5_budget_kbps(bytes);
   if(!mode||bytes>1536||(h>>10)!=1){
    /* Known lossy 5ms stereo captures use flags=1. Search bytewise after
     * lost framing instead of throwing away the rest of every packet. Two
     * completed, matching exact budgets are still required for a label. */
    v5_nominal_kbps=v5_candidate=v5_matches=v5_last_frame=0;
    v5_header[0]=v5_header[1];v5_header_used=1;v5_invalid_headers++;continue;
   }
   if(mode!=v5_candidate){v5_candidate=mode;v5_matches=0;}
   v5_remaining=bytes;
  }
  unsigned take=n<v5_remaining?n:v5_remaining;payload+=take;n-=take;v5_remaining-=take;
  if(!v5_remaining){
   if(v5_matches<2)v5_matches++;
   if(v5_matches==2){v5_nominal_kbps=v5_candidate;v5_last_frame=t;}
  }
 }
}
static unsigned v5_bitrate_current(unsigned t){return v5_last_frame&&t-v5_last_frame<1500?v5_nominal_kbps:0;}
