/* A known-length PCM stream has stopped near its end, while AVTransport
 * still claims PLAYING. Debounce; unknown-length streams stay untouched. */
static int dlna_eof_candidate(unsigned state,unsigned transport,unsigned duration_seconds,unsigned position_ms,unsigned recent,unsigned now){
 if(state!=3||transport!=1||!duration_seconds||duration_seconds>86400||now-recent<3)return 0;
 unsigned end=duration_seconds*1000;
 return position_ms>=end-(end>=1000?1000:0);
}
