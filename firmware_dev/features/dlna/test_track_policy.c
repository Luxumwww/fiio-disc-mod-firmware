#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "track_policy.h"
int main(void){
 DiscDlnaHistory h={0};DiscDlnaTrack out;assert(!dlna_history_target(&h,1,&out));
 assert(dlna_history_add(&h,"http://one?a=1&b=2","<title>一</title>"));assert(!dlna_history_target(&h,1,&out));assert(!dlna_history_target(&h,2,&out));
 assert(dlna_history_add(&h,"http://two",""));assert(dlna_history_target(&h,2,&out)&&!strcmp(out.uri,"http://one?a=1&b=2"));
 assert(dlna_history_add(&h,out.uri,out.meta));assert(dlna_history_target(&h,1,&out)&&!strcmp(out.uri,"http://two"));
 assert(dlna_history_add(&h,"http://three",""));assert(!dlna_history_target(&h,1,&out));
 for(unsigned i=0;i<20;i++){char uri[32];snprintf(uri,sizeof(uri),"http://item/%u",i);assert(dlna_history_add(&h,uri,""));}assert(h.count==8&&h.index==7);
 char xml[128];assert(dlna_xml_escape(xml,sizeof(xml),"<&\"'>")>0&&!strcmp(xml,"&lt;&amp;&quot;&apos;&gt;"));char tiny[4];assert(dlna_xml_escape(tiny,sizeof(tiny),"&")<0);
 char large[DLNA_URI_CAP+1];memset(large,'x',sizeof(large));large[sizeof(large)-1]=0;unsigned count=h.count;assert(!dlna_history_add(&h,large,"")&&h.count==count);
 assert(dlna_history_add(&h,"","")&&!h.count);
 puts("PASS: URI history, back/forward, branch truncation, bounded rollover, oversized input rejection and XML escaping");
}
