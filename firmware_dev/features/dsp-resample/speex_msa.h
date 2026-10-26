/* Speex Q10 keeps four float products and four independent double sums.
 * MSA implements the same order, with no FMA or reduced precision. */
#if defined(__mips_msa)
#include <msa.h>
#define OVERRIDE_INNER_PRODUCT_DOUBLE 1
static inline double inner_product_double(const float *coeff,const float *samples,unsigned length){
 v2f64 lo={0,0},hi={0,0};
 for(unsigned i=0;i<length;i+=4){
  v4f32 a=(v4f32)__msa_ld_w((void *)(coeff+i),0),b=(v4f32)__msa_ld_w((void *)(samples+i),0);
  v4f32 product=__msa_fmul_w(a,b);
  lo=__msa_fadd_d(lo,__msa_fexupr_d(product));hi=__msa_fadd_d(hi,__msa_fexupl_d(product));
 }
 return ((lo[0]+lo[1])+hi[0])+hi[1];
}
#endif
