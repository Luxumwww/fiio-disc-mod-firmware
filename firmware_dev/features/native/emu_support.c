/* Computer-only: emulate successful scheduler priority requests. The private
 * namespace runner bounds CPU/lifetime; never include this in device firmware. */
int pthread_setschedparam(unsigned thread,int policy,const void *priority){
 (void)thread;(void)policy;(void)priority;return 0;
}
