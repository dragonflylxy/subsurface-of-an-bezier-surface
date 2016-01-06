

void plan_buffer_line(float x, float y, float z, float feed_rate, uint8_t invert_feed_rate) 
{
    int32_t ex,ey;
    int32_t dlx,dly,sx,sy;
    dlx=0;
    dly=0;
  // Prepare to set up new block
  block_t *block = &block_buffer[block_buffer_head];

    syspos(&ex,&ey);
    
    sx=ex-ofst_x;
    sy=ey-ofst_y;
    
    dlx=sys.position[X_AXIS]-sx;
    dly=sys.position[Y_AXIS]-sy;
    
    sys.position[X_AXIS]-=dlx;
    sys.position[Y_AXIS]-=dly;
    
    // Calculate target position in absolute steps
  int32_t target[3];
  int32_t targetP[3];
    
  targetP[X_AXIS] = lround(x*settings.steps_per_mm[X_AXIS]);
  targetP[Y_AXIS] = lround(y*settings.steps_per_mm[Y_AXIS]);
  targetP[Z_AXIS] = lround(z*settings.steps_per_mm[Z_AXIS]);
    
  memcpy(target, targetP, sizeof(targetP));
    
    target[X_AXIS]+=dlx;
    target[Y_AXIS]+=dly;
    
    ...
    
    memcpy(pl.position, targetP, sizeof(target));
