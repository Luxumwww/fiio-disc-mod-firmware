/* Opaque native shade hides logical rows [0,height). Keep the frozen XY
 * pixels on the DPU plane; change only the newly covered/uncovered strip. */
static void shade_mask_strip(unsigned *dst,const unsigned *frozen,unsigned rotation,unsigned old_height,unsigned height){
 if(old_height==height)return;
 unsigned begin=old_height<height?old_height:height,end=old_height>height?old_height:height;
 unsigned x0=0,x1=360,y0=0,y1=360;
 switch(rotation&3u){case 0:y0=begin;y1=end;break;case 1:x0=begin;x1=end;break;case 2:y0=360-end;y1=360-begin;break;default:x0=360-end;x1=360-begin;break;}
 for(unsigned y=y0;y<y1;y++){
  unsigned offset=y*360+x0,bytes=(x1-x0)*4;
  if(height>old_height)memset(dst+offset,0,bytes);else memcpy(dst+offset,frozen+offset,bytes);
 }
}
