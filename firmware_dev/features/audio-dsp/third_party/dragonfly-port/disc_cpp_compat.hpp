#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void *disc_fv_alloc(size_t);
void disc_fv_free(void *);
[[noreturn]] void disc_fv_oom();
namespace std {
inline long double floor(long double x){return ::floor((double)x);}
inline long double atan(long double x){return ::atan((double)x);}
inline long double cos(long double x){return ::cos((double)x);}
inline long double exp(long double x){return ::exp((double)x);}
inline long double log(long double x){return ::log((double)x);}
inline long double sin(long double x){return ::sin((double)x);}
inline long double sinh(long double x){return ::sinh((double)x);}
inline long double sqrt(long double x){return ::sqrt((double)x);}
inline long double tan(long double x){return ::tan((double)x);}
inline long double pow(long double x,long double y){return ::pow((double)x,(double)y);}
inline long double abs(long double x){return x<0?-x:x;}

using ::fprintf;using ::memcpy;using ::memset;using ::rand;
using ::floor;using ::atan;using ::cos;using ::exp;using ::log;
using ::pow;using ::sin;using ::sinh;using ::sqrt;using ::tan;
inline float floor(float v){return floorf(v);} inline float atan(float v){return atanf(v);}
inline float cos(float v){return cosf(v);} inline float exp(float v){return expf(v);}
inline float log(float v){return logf(v);} inline float pow(float a,float b){return powf(a,b);}
inline float sin(float v){return sinf(v);} inline float sinh(float v){return sinhf(v);}
inline float sqrt(float v){return sqrtf(v);} inline float tan(float v){return tanf(v);}
inline int abs(int v){return v<0?-v:v;} inline float abs(float v){return fabsf(v);}
inline double abs(double v){return fabs(v);}
}
void *operator new(__SIZE_TYPE__);
void *operator new[](__SIZE_TYPE__);
void operator delete(void *) noexcept;
void operator delete[](void *) noexcept;
