//#include "stepper.h"
//#include "config.h"
//#include "settings.h"
#include "Wire.h"
#include "print.h"
#include "enco.h"
#include "nuts_bolts.h"

void syspos(int32_t *x,int32_t *y,int32_t *z)
{
    int32_t dt1,dt2;
    int32_t mx,my,mz,xx,yy,zz;
    xx=0;yy=0;zz=0;
    char oo=0;
    
    Wire.beginTransmission(8); // transmit to device #8
    
    int available = Wire.requestFrom(8, 6); //   Wire.write((xx>>8));        // sends five bytes
    
    
    if (available==6)   // slave may send less than requested
    {

        zz=Wire.read() << 8 | Wire.read();
        yy=Wire.read() << 8 | Wire.read();
        xx=Wire.read() << 8 | Wire.read();

        
      //  if((dt1==0xff)&&(dt2==0xff))
      //      printInteger(dt1);
      //  printString("a\n");
            oo=1;

        }
    else{
        oo=0;
        //printInteger(dt1);
    //    printInteger(available);
     //   printString("b\n");
        
    }
    Wire.endTransmission();    // stop transmitting
    
    if(oo==1)
    {
        mx=xx*8/5;
        my=yy*8/5;
        mz=zz;
        
        *x=mx;
        *y=my;
        *z=mz;
        
        //plan_set_current_position(sys.position[X_AXIS],sys.position[Y_AXIS],sys.position[Z_AXIS]);
        //       printInteger(xx);
        //      printPgmString(PSTR("aaa"));
        //      printInteger(yy);
        //     printPgmString(PSTR("\n"));
    }
    else
    {
        *x=sys.position[X_AXIS]+ofst_x;
        *y=sys.position[Y_AXIS]+ofst_y;
        *z=sys.position[Z_AXIS];
    }
}