/* LVGL-free libgooglepinyin facade from VanillaNahida/lvgl_pinyin_ime. */
extern void *dlopen(const char *,int);
extern void *dlsym(void *,const char *);
static void *online_ime_lib;
static int online_ime_ready,online_input_mode;
static unsigned online_candidate_page,online_candidate_count;
static char online_compose[65],online_candidates[3][193];
static int (*online_ime_open)(const char *,const char *);
static unsigned (*online_ime_search)(const char *,unsigned),(*online_ime_candidate)(unsigned,char *,unsigned),(*online_ime_choose)(unsigned);
static const char *(*online_ime_remaining)(unsigned *);
static void (*online_ime_reset)(void);
static unsigned online_length(const char *p){unsigned n=0;while(p[n])n++;return n;}
static int online_ime_init(void){
 if(online_ime_ready)return 1;
 if(!online_ime_lib)online_ime_lib=dlopen("/usr/data/disc-runtime/components/disc-online-music/current/libdisc-ime.so",2);
 if(!online_ime_lib)return 0;
 online_ime_open=dlsym(online_ime_lib,"ime_engine_open");online_ime_search=dlsym(online_ime_lib,"ime_engine_search");online_ime_candidate=dlsym(online_ime_lib,"ime_engine_candidate");online_ime_choose=dlsym(online_ime_lib,"ime_engine_choose");online_ime_remaining=dlsym(online_ime_lib,"ime_engine_remaining_pinyin");online_ime_reset=dlsym(online_ime_lib,"ime_engine_reset");
 if(!online_ime_open||!online_ime_search||!online_ime_candidate||!online_ime_choose||!online_ime_remaining||!online_ime_reset)return 0;
 online_ime_ready=online_ime_open("/usr/data/disc-runtime/components/disc-online-music/current/dict_pinyin.dat","");return online_ime_ready;
}
static void online_ime_update(void){online_candidate_count=0;if(online_compose[0]&&online_ime_init())online_candidate_count=online_ime_search(online_compose,online_length(online_compose));if(online_candidate_page*3>=online_candidate_count)online_candidate_page=0;for(unsigned i=0;i<3;i++){online_candidates[i][0]=0;if(online_candidate_page*3+i<online_candidate_count)online_ime_candidate(online_candidate_page*3+i,online_candidates[i],sizeof(online_candidates[i]));}}
static int online_append(const char *text){unsigned n=online_length(online_query),k=online_length(text);if(n+k>=sizeof(online_query))return 0;for(unsigned i=0;i<=k;i++)online_query[n+i]=text[i];return 1;}
static void online_ime_pick(unsigned item){if(item>=3||!online_candidates[item][0])return;unsigned n=online_length(online_query),k=online_length(online_candidates[item]);if(n+k>=sizeof(online_query))return;online_ime_choose(online_candidate_page*3+item);unsigned remain=0;const char *p=online_ime_remaining(&remain);char next[65];snprintf(next,sizeof(next),"%s",p?p:"");online_append(online_candidates[item]);snprintf(online_compose,sizeof(online_compose),"%s",next);online_ime_reset();online_candidate_page=0;online_ime_update();}
static void online_erase(void){unsigned n=online_length(online_compose);if(n){online_compose[n-1]=0;online_candidate_page=0;online_ime_update();return;}n=online_length(online_query);if(n){n--;while(n&&((unsigned char)online_query[n]&0xc0)==0x80)n--;online_query[n]=0;}}
