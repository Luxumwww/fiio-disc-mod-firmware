/* Aggregate Linux CPU capacity and reclaim-aware RAM. No worker or timer. */
typedef struct {unsigned total,idle;int ready;} DiscCpuSample;
static int disc_cpu_percent(DiscCpuSample *old,const unsigned v[8]){
 unsigned total=0;for(unsigned i=0;i<8;i++)total+=v[i];unsigned idle=v[3]+v[4];
 unsigned elapsed=total-old->total,rest=idle-old->idle;int result=-1;
 if(old->ready&&elapsed&&rest<=elapsed)result=(int)(100.0*(elapsed-rest)/elapsed+0.5);
 *old=(DiscCpuSample){total,idle,1};return result;
}
static unsigned disc_cpu_mhz(unsigned control,unsigned pll){
 if(control>>30!=2||((control>>28)&3)!=1)return 0;
 double mhz=24.0*((pll>>20&511)+1)*2/((pll>>14&63)+1)/(1u<<(pll>>11&7))/((control&15)+1);
 return mhz>=100&&mhz<=2000?(unsigned)(mhz+0.5):0;
}
static unsigned disc_memory_used(unsigned total,unsigned available){return available<=total?total-available:0;}
