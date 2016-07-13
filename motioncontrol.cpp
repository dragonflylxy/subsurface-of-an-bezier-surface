// mc_line( ){...
    int xx,yy;
   
    Wire.beginTransmission(8); 
    int available = Wire.requestFrom(8, 4); 
    if (available==4)   
    {
       yy=Wire.read() << 8 | Wire.read();
       xx=Wire.read() << 8 | Wire.read();
    }
    else{
        printPgmString(PSTR("rrr\n"));
    }
    Wire.endTransmission();   
    
//...}
