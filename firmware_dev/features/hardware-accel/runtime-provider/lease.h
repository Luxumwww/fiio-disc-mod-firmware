/* Main-thread exclusive ownership; no implicit preemption or nesting. */
typedef struct {unsigned token,sequence;} DiscLease;
static unsigned lease_claim(DiscLease *l){if(l->token)return 0;if(!++l->sequence)++l->sequence;return l->token=l->sequence;}
static int lease_valid(DiscLease *l,unsigned token){return token&&l->token==token;}
static int lease_release(DiscLease *l,unsigned token){if(!lease_valid(l,token))return 0;l->token=0;return 1;}
