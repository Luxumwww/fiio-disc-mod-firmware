/* Bounded URI history; a phone's private queue is never inferred. */
#define DLNA_HISTORY 8
#define DLNA_URI_CAP 4096
#define DLNA_META_CAP 8192
typedef struct {char uri[DLNA_URI_CAP],meta[DLNA_META_CAP];} DiscDlnaTrack;
typedef struct {DiscDlnaTrack tracks[DLNA_HISTORY];unsigned count,index;} DiscDlnaHistory;
static int dlna_copy_text(char *out,unsigned cap,const char *s){
 unsigned n=0;if(!s)return 0;while(s[n]&&n<cap)n++;if(n==cap)return 0;
 for(unsigned i=0;i<=n;i++)out[i]=s[i];return 1;
}
static int dlna_history_add(DiscDlnaHistory *h,const char *uri,const char *meta){
 if(!uri||!uri[0]){h->count=h->index=0;return 1;}
 DiscDlnaTrack track;if(!dlna_copy_text(track.uri,sizeof(track.uri),uri)||!dlna_copy_text(track.meta,sizeof(track.meta),meta))return 0;
 for(unsigned i=0;i<h->count;i++)if(!strcmp(h->tracks[i].uri,uri)){h->index=i;h->tracks[i]=track;return 1;}
 if(h->count)h->count=h->index+1;
 if(h->count==DLNA_HISTORY){for(unsigned i=1;i<h->count;i++)h->tracks[i-1]=h->tracks[i];h->count--;}
 h->index=h->count;h->tracks[h->count++]=track;return 1;
}
static int dlna_history_target(const DiscDlnaHistory *h,unsigned action,DiscDlnaTrack *out){
 if(!h->count)return 0;
 if(action==2&&h->index){*out=h->tracks[h->index-1];return 1;}
 if(action==1&&h->index+1<h->count){*out=h->tracks[h->index+1];return 1;}
 return 0;
}
static int dlna_xml_escape(char *out,unsigned cap,const char *s){
 unsigned n=0;if(!s)return -1;
 while(*s){const char *part=0;char c=*s++;
  if(c=='&')part="&amp;";else if(c=='<')part="&lt;";else if(c=='>')part="&gt;";else if(c=='\"')part="&quot;";else if(c=='\'')part="&apos;";
  if(part){while(*part){if(n+1>=cap)return -1;out[n++]=*part++;}}
  else{if(n+1>=cap)return -1;out[n++]=c;}
 }if(n>=cap)return -1;out[n]=0;return (int)n;
}
