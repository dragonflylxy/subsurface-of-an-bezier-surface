
#include <AlUniverse.h>

#include <AlCurve.h>
#include <AlCurveCV.h>
#include <AlCurveNode.h>
#include <AlSurfaceNode.h>
#include <AlSurface.h>
#include <AlSurfaceCV.h>

#include <AlLiveData.h>
#include <AlFunction.h>
#include <AlFunctionHandle.h>

#include <AlCommand.h>
#include <AlUserCommand.h>
#include <AlNotifyDagNode.h>
#include <AlPickList.h>

#include <AlAttributes.h>
#include <AlLineAttributes.h>

//#include <string.h>
#include <AlWindow.h>
#include <AlPickable.h>
#include <AlTM.h>
#include <AlLiveData.h>
#include <AlBlindData.h>
#include "omp.h"


//
// Prototypes
//

#define CMD_NAME "linkObject"	// Used by OpenAlias for creation and 
								// destruction of the command plug-in.
#define CMD_CLASS_ID	50		// A user defined type in case you have
								// more than 1 command. 

const int kIntId = 100;
const int kCharId = 101;
const int kDoubleId = 102;

	double *zt0;
	unsigned long ztsz0;

void zeros( double *ptr);
void ones( double *ptr);
void pascs( double *ptr);
double* cnstm(int u,int v,int ui,int vi);
unsigned long precsnt(double *a,double *b);

double cnp(double a,double b);
double factr(double a);

void clearmm( double *ptr);
void flwFM( double *a, double *b, double *c);
void setmmxx( double *a,int u,int v,int u1,int v1,double nbr);
double getmmxx( double *a,int u,int v,int u1,int v1);

unsigned long cnst_size(int u,int v,int ui,int vi);
void cn_init( double *a,int u,int v,int u1,int v1);

//
// A user defined data class.  This class contains two 
// constructor dag nodes and their targets.  firstDagObject
// and secondDagObject are the constructors.  firstLineDag and
// secondLineDag are the targets and are
// rebuilt based on the changes to the constructors. 
// 
class cmdData
{
public:
	cmdData*			cmdDataPtr()	{ return this; }
	AlCurveNode*		domain;
	AlSurfaceNode*		sf0;
	AlSurfaceNode*		sf1;
	AlSurfaceNode*		sf2;

	 double		 				*zt;
	 unsigned long  *cmt_sz;
	 double					**cmt;
	 int						   mmdf;
	 int dru,drv,fnu,fnv,ggu,ggv,dgu,dgv;
	 unsigned long ssu_sz,ssv_sz,wwuv_sz,xxs_sz,paa_sz,mtdru_sz,mtdrv_sz,rst_sz,ztsz;
	 AlSurface *suv,*pnbs,*nrbs;
	 AlCurve *curve ;
	 double *mtdru,*mtdrv,*wwuv,*ssu,*ssv,*xxs,*paa,*rst;

	int								 od;
	AlData *charData;
	AlData *intData;
	AlData *doubleData;
};

class genericCmd: public AlUserCommand, private cmdData
{
	enum error { kOkay = 0, kInvalid = 1, kDagNodeNotHandled };

public:
	genericCmd();
	~genericCmd();

	virtual int			isValid();
	virtual int			execute();
	virtual int			declareReferences();
	virtual int			instanceDag( AlDagNode *oldDag, AlDagNode *newDag );
	virtual int			undo();

	virtual int			geometryModified( AlDagNode *dag );
	virtual int			listModifiedDagNodes( const AlNotifyDagNode *dagMod, AlObject *obj );
	virtual int			debug( const char *prefix );
	virtual void *		asDerivedPtr();

	virtual statusCode	retrieveWire( AlInput *input );
	virtual statusCode	storeWire( AlOutput *output );

	virtual int			dagModified( AlDagNode *dag );

	// for your own use
	virtual int			type();

	// We didn't bother to implement these commands since they don't apply
	// to us (they would be similar to instanceDag() and geometryModified() )
	//
	// virtual int curveOnSurfaceModified( AlCurveOnSurface *surf );

	// The following command is not yet supported
	// virtual statusCode  storeSDL( ofstream &outputSDL );

public:
	// Methods that the UI commands can use to set our fields
	statusCode			set( AlDagNode *firstDag, AlDagNode *secondDag ,AlDagNode *thirdDag, AlDagNode *fourthDag, int d);
	
private:
	boolean requireStrongValidity;
};

class seldata
{
public:
	seldata*			seldataPtr()	{ return this; }
	AlCurveNode*		pt1;
	AlSurfaceNode*		sf1;
	AlSurfaceNode*		sf2;
	AlSurfaceNode*		sf3;
	int							   odu;
};

class selctfc: public AlContinuousFunction,private seldata
{
	public:
		static void init_func( void );
		static void down_func(int input, Screencoord x, Screencoord y );
		static int    go_button_pressed( void );
		static void move_func(int input, Screencoord x, Screencoord y );
		static void up_func(int input, Screencoord x, Screencoord y );
		static void cleanup_func();
};

//
//	Start command definition
//

genericCmd::genericCmd()
//
//	Initialize the structure
//
{
	domain = NULL;
	sf0 = NULL;
	sf1 = NULL;
	sf2 = NULL;
	od  = 0;	
	zt = NULL;
	cmt_sz = NULL;
	cmt = NULL;

	requireStrongValidity = TRUE;
	
	charData = NULL;
	intData = NULL;
	doubleData = NULL;
}

genericCmd::~genericCmd()
//
// Provide a safe cleanup.
//
{
	if ( domain != NULL )
		delete domain;
	if ( sf0 != NULL )
		delete sf0;
	if ( sf1 != NULL )
		delete sf1;
	if ( sf2 != NULL )
		delete sf2;
	if ( zt != NULL )
		delete[] zt;
	if ( cmt_sz != NULL )
		delete[] cmt_sz;
	if ( cmt != NULL )
		delete[] cmt;


	if ( charData )
		delete charData;
	if ( intData )
		delete intData;
	if ( doubleData )
		delete doubleData;
}

void *genericCmd::asDerivedPtr()
//
//	Provide safe down casting.
//
{
	return this;
}

int genericCmd::type()
//
//	User defined value so you can determine the class type
//	of the command.
//
{
	return CMD_CLASS_ID;
}

int genericCmd::isValid()
//
//	Since the construction history plug-in maintains its own data,
//	it is necessary for it to implement the isValid() method to
//	tell the command layer that it is ok to call this command.
//
{
	// Testing will involve NULL pointer checks, making sure you
	// have the correct kind of Dags etc.
	
	if( domain == NULL || sf0 == NULL || sf1 == NULL )
		return kInvalid;
		
	if ( requireStrongValidity )
		if ( sf2 == NULL)
			return kInvalid;

	int result1, result2, result3;
	
	switch( domain->type() )
	{
		case kCurveNodeType:
			result1 =  kOkay;
			break;
		default:
			result1 =  kDagNodeNotHandled;
			break;
	}
	switch( sf0->type() )
	{
		case kSurfaceNodeType:
			result2 =  kOkay;
			break;
		default:
			result2 =  kDagNodeNotHandled;
			break;
	}
	switch( sf1->type() )
	{
		case kSurfaceNodeType:
			result3 =  kOkay;
			break;
		default:
			result3 =  kDagNodeNotHandled;
			break;
	}

	if ( result1 == kOkay && result2 == kOkay && result3 == kOkay )
		return kOkay;
		
	return kDagNodeNotHandled;
}

int genericCmd::execute()
//
//	This method is called when the geometry needs to be updated.
//	This will occur if the constructor dag nodes are modified.	
//
{	
	int u1,v1,u2,v2,u11,v11,u22,v22,u,v,ua,va;
	double gtdu = 1,gtdv = 1,wgt = 1;
	int i,j,ii,jj,w,r=4,k,km;
	int fv,du,dv,md0,md1;
	double tmp,tmp0,tmpr[4],tmpxs,su1,sv1,ud,vd,tmph1,tmph2;
	double a[4],b[4],c[4],d[4];
	double *p= NULL;


	AlCurveCV *cv1 = NULL;
	AlCurveCV *cv2 = NULL;

	AlSurfaceCV *sv,*mmvv,*objj;
////////////////////////////////////////////////////

	dru = pnbs->uDegree();
	drv = pnbs->vDegree();
	ggu = suv->uDegree();
	ggv = suv->vDegree();
	fnu = ggu*(dru+drv)+1;
	fnv = ggv*(dru+drv)+1;
	dgu=nrbs->uDegree()+1;
	dgv=nrbs->vDegree()+1;

	double *cvs = new double [(dru+1) * (drv+1) * 4]; 
	int *uMult = new int [dru+1]; 
	int *vMult = new int [drv+1];
	pnbs->CVsWorldPosition( cvs, uMult, vMult ); 

	double *suv_cvs = new double [(ggu+1) * (ggv+1) * 4]; 
	int *suv_uMult = new int [ggu+1]; 
	int *suv_vMult = new int [ggv+1];
	suv->CVsWorldPosition( suv_cvs, suv_uMult, suv_vMult ); 


	ssu_sz = cnst_size(ggu+1,ggv+1,1,1);
	ssv_sz = cnst_size(ggu+1,ggv+1,1,1);
	wwuv_sz = cnst_size(ggu+1,ggv+1,1,1);
	xxs_sz = cnst_size(fnu,fnv,1,1);
	paa_sz = cnst_size(ggu+1,ggv+1,1,1);
	mtdru_sz = cnst_size(ggu+1,ggv+1,2,1);
	mtdrv_sz = cnst_size(ggu+1,ggv+1,1,2);
	rst_sz = cnst_size(fnu,fnv,1,r);
	ztsz = ssu_sz + ssv_sz + wwuv_sz + xxs_sz + paa_sz + mtdru_sz + mtdrv_sz + rst_sz;




	if (cmt_sz == NULL)
	{
		cmt_sz = new  unsigned long [dru+drv+1];
		cmt = new  double *[dru+drv+1];
	}
	else
	{
		if (mmdf == 333)
		{
			delete[] cmt_sz;
			cmt_sz = new  unsigned long [dru+drv+1];
			delete[] cmt;
			cmt = new  double *[dru+drv+1];
		}
	}



//	unsigned long *cmt_sz = (unsigned long *)malloc((dru+drv+1)*sizeof( unsigned long ));
	cmt_sz[0] = cnst_size(1,1,1,1);
	ztsz = ztsz + cmt_sz[0];
	u1=1;v1=1;u11=1;v11=1;
	u2=ggu+1;v2=ggv+1;u22=2;v22=1;
	for(i=0;i<dru;i++)
	{
		cmt_sz[i+1] = cnst_size(u1+u2-1,v1+v2-1,u11+u22-1,v11+v22-1);
		ztsz = ztsz + cmt_sz[i+1];
		u1=u1+u2-1;v1 = v1+v2-1;
		u11 = u11+u22-1;v11 = v11+v22-1;
	}
	u22=1;v22=2;
	for(j=dru;j<dru+drv;j++)
	{
		cmt_sz[j+1] = cnst_size(u1+u2-1,v1+v2-1,u11+u22-1,v11+v22-1);
		ztsz = ztsz + cmt_sz[j+1];
		u1=u1+u2-1;v1 = v1+v2-1;
		u11 = u11+u22-1;v11 = v11+v22-1;
	}



//	double *zt =   (double * const)malloc(ztsz*sizeof(double));
	if (zt == NULL)
	{
		zt = new    double [ztsz];
		p = zt;
		zt0 = p;
		ztsz0 = ztsz;
	}
	else
	{
		if (mmdf == 333)
		{
			delete[] zt;
			zt = new    double [ztsz];
			p = zt;
			zt0 = p;
			ztsz0 = ztsz;
			mmdf = 0;
		}
		if (mmdf == 222)
		{
			p = zt;
			zt0 = p;
			ztsz0 = ztsz;
			mmdf = 0;
		}
		if (mmdf == 555)
		{
			p = zt;
			zt0 = p;
			ztsz0 = ztsz;
			mmdf = 0;
			goto begindo2;
		}
	}
	
	
	ssu = p;
	p = p + ssu_sz;
	ssv = p;
	p = p + ssv_sz;
	wwuv = p;
	p = p + wwuv_sz;
	xxs = p;
	p = p + xxs_sz;
	paa = p;
	p = p + paa_sz;
	mtdru = p;
	p = p + mtdru_sz;
	mtdrv = p;
	p = p + mtdrv_sz;
	rst = p;
	p = p + rst_sz;





	cn_init(ssu,ggu+1,ggv+1,1,1);
	cn_init(ssv,ggu+1,ggv+1,1,1);
	cn_init(wwuv,ggu+1,ggv+1,1,1);
	cn_init(xxs,fnu,fnv,1,1);
	cn_init(paa,ggu+1,ggv+1,1,1);
	cn_init(mtdru,ggu+1,ggv+1,2,1);
	cn_init(mtdrv,ggu+1,ggv+1,1,2);
	cn_init(rst,fnu,fnv,1,r);

	zeros(ssu);
	zeros(ssv);
	ones(wwuv);
	pascs(xxs);
	pascs(paa);


//	double **cmt =  (double **)malloc((dru+drv+1)*sizeof(double*));


	cmt[0] = p;
	cn_init(cmt[0],1,1,1,1); /////////////////////////////////////////
	p = p + cmt_sz[0];
	u1=1;v1=1;u11=1;v11=1;
	u2=ggu+1;v2=ggv+1;u22=2;v22=1;
	for(i=0;i<dru;i++)
	{
		cmt[i+1] = p;
		p = p + cmt_sz[i+1];
		cn_init(cmt[i+1],u1+u2-1,v1+v2-1,u11+u22-1,v11+v22-1);
		clearmm(cmt[i+1]);
		u1=u1+u2-1;v1 = v1+v2-1;
		u11 = u11+u22-1;v11 = v11+v22-1;
	}
	u22=1;v22=2;
	for(j=dru;j<dru+drv;j++)
	{
		cmt[j+1] = p;
		p = p + cmt_sz[j+1];
		cn_init(cmt[j+1],u1+u2-1,v1+v2-1,u11+u22-1,v11+v22-1);
		clearmm(cmt[j+1]);
		u1=u1+u2-1;v1 = v1+v2-1;
		u11 = u11+u22-1;v11 = v11+v22-1;
	}



	setmmxx(cmt[0],0,0,0,0,1.0);



	curve->getCV(0)->worldPosition(a[0],a[1],a[2],a[3]);
	curve->getCV(1)->worldPosition(b[0],b[1],b[2],b[3]);

	ud = b[1] - a[1];
	vd = b[0] - a[0];
	su1 = 0.0;	sv1 = 0.0;



	for(i=0;i<=ggu;i++)
	{
		for(j=0;j<=ggv;j++)
		{
			for(w=0;w<r;w++)
			{
				km = (i*(ggv+1)+j)*4+w;
				c[w] = suv_cvs[km];
			}
			su1 = (c[1]-a[1])/ud;
			sv1 = (c[0]-a[0])/vd;
			setmmxx(wwuv,i,j,0,0,c[3]);
			setmmxx(ssu,i,j,0,0,su1);
			setmmxx(ssv,i,j,0,0,sv1);
		}
	}


//	begindo:

	for(i=0;i<=ggu;i++)
	{
		for(j=0;j<=ggv;j++)
		{
			tmp0 = getmmxx(paa,i,j,0,0);
			su1 = getmmxx(ssu,i,j,0,0);
			sv1 = getmmxx(ssv,i,j,0,0);
			wgt = tmp0*getmmxx(wwuv,i,j,0,0);

			setmmxx(mtdru,i,j,0,0,(1 - su1)*wgt);
			setmmxx(mtdru,i,j,1,0,su1*wgt);
			setmmxx(mtdrv,i,j,0,0,(1 - sv1)*wgt);
			setmmxx(mtdrv,i,j,0,1,sv1*wgt);
		}
	}



	for(i=0;i<dru;i++)
	{
		 flwFM(cmt[i],mtdru,cmt[i+1]);
	}
	for(j=dru;j<dru+drv;j++)
	{
		 flwFM(cmt[j],mtdrv,cmt[j+1]);
	}



	begindo2:

	for(i=0;i<fnu;i++)
	{
		for(j=0;j<fnv;j++)
		{			
				for(w=0;w<r;w++)
					tmpr[w] = 0;

				for(ii=0;ii<dru+1;ii++)
				{
					for(jj=0;jj<drv+1;jj++)
					{
						for(w=0;w<r;w++)
						{
							km = (ii*(drv+1)+jj)*4+w;
							d[w] = cvs[km];
						}
						tmp = getmmxx(cmt[dru+drv],i,j,ii,jj);
						for(w=0;w<r;w++)
						{
							tmpr[w] = tmpr[w] + (d[w])*tmp;
						}
					}
				}
				for(w=0;w<r;w++)
				{
					setmmxx(rst,i,j,0,w,tmpr[w]);
				}
		}
	}


//jj:	
	
	md0 = dgu/2;
	md1 = dgv/2;


	for(k=0;k<(fnu-dgu);k++)
	{
		for(i=1;i<md0;i++)
		{
			for(j=0;j<fnv;j++)
			{
				for(w=0;w<r;w++)
				{
					tmph1 = getmmxx(rst,i -1,j,0,w);
					tmph2 = getmmxx(rst,i,j,0,w);
					setmmxx(rst,i,j,0,w,tmph2 - tmph1);
				}
			}
		}
	}

	for(k=0;k<(dgu-fnu);k++)
	{
		for(i=1;i<fnu;i++)
		{
			for(j=0;j<fnv;j++)
			{
				for(w=0;w<r;w++)
				{
					tmph1 = getmmxx(rst,fnu - i -1,j,0,w);
					tmph2 = getmmxx(rst,fnu - i,j,0,w);
					setmmxx(rst,fnu - i,j,0,w,tmph2 + tmph1);
				}
			}
		}
	}

	for(k=0;k<(fnv-dgv);k++)
	{
		for(i=0;i<md0;i++)
		{
			for(j=1;j<md1;j++)
			{
				for(w=0;w<r;w++)
				{
					tmph1 = getmmxx(rst,i,j-1,0,w);
					tmph2 = getmmxx(rst,i,j,0,w);
					setmmxx(rst,i,j,0,w,tmph2 - tmph1);

					tmph1 = getmmxx(rst,i,fnv-j,0,w);
					tmph2 = getmmxx(rst,i,fnv-j-1,0,w);
					setmmxx(rst,i,fnv-j-1,0,w,tmph2 - tmph1);
				}
			}
		}
	}

dv= dgv-1;
du= dgu-1;
//return kOkay;

if (od == 0)
	for(i=0;i<md0;i++)
	{
		for(j=0;j<md1;j++)
		{
			tmpxs = cnp(du,i)*cnp(dv,j);
			objj = nrbs->getCV(i,j);
			for(w=0;w<r;w++)
			{
				tmpr[w] = getmmxx(rst,i,j,0,w);
			}
			objj->setWorldPosition(tmpr[0]/tmpxs,tmpr[1]/tmpxs,tmpr[2]/tmpxs,tmpr[3]/tmpxs,1);

			objj = nrbs->getCV(i,dgv-j-1);
			for(w=0;w<r;w++)
			{
				tmpr[w] = getmmxx(rst,i,fnv-j-1,0,w);
			}
			objj->setWorldPosition(tmpr[0]/tmpxs,tmpr[1]/tmpxs,tmpr[2]/tmpxs,tmpr[3]/tmpxs,1);
		}
	}
else 
	for(i=0;i<md0;i++)
	{
		for(j=0;j<md1;j++)
		{
			tmpxs = cnp(du,i)*cnp(dv,j);
			objj = nrbs->getCV(dgu - i - 1,j);
			for(w=0;w<r;w++)
			{
				tmpr[w] = getmmxx(rst,i,j,0,w);
			}
			objj->setWorldPosition(tmpr[0]/tmpxs,tmpr[1]/tmpxs,tmpr[2]/tmpxs,tmpr[3]/tmpxs,1);

			objj = nrbs->getCV(dgu - i - 1,dgv-j-1);
			for(w=0;w<r;w++)
			{
				tmpr[w] = getmmxx(rst,i,fnv-j-1,0,w);
			}
			objj->setWorldPosition(tmpr[0]/tmpxs,tmpr[1]/tmpxs,tmpr[2]/tmpxs,tmpr[3]/tmpxs,1);

		}
	}

	delete[] cvs;
	delete[] uMult;
	delete[] vMult;
	delete[] suv_cvs;
	delete[] suv_uMult;
	delete[] suv_vMult;

//	free(zt);
//	free(cmt);
//	free(cmt_sz);
//	delete[] zt;
//	delete[] cmt;
//	delete[] cmt_sz;

/*	zt = NULL;
	cmt = NULL;
	cmt_sz = NULL;

	mtdru = NULL;
	mtdrv = NULL;
	xxs = NULL;
	paa = NULL;
	ssu = NULL;
	ssv = NULL;
	wwuv = NULL;
	rst = NULL;

*/

	AlUniverse::doUpdates(FALSE);	
	sf2->asDagNodePtr()->updateDrawInfo();


	// Force the redrawing of the screen

	//AlUniverse::redrawScreen( kRedrawAll );
	
	return kOkay;
}

int genericCmd::instanceDag( AlDagNode *oldDag, AlDagNode *newDag )
//
//	Handle a dag node being instanced.  Go through the class and replace
//	any references to oldDag with newDag
//
{
printf("genericCmd::instanceDag()\n");

	if ( oldDag == NULL || newDag == NULL )
		return -1;
		
	if( AlAreEqual( domain, oldDag ) )
	{
		// Toss our old wrapper and replace it with a new one.
		delete domain;
		domain = newDag->copyWrapper()->asCurveNodePtr();
	}
	if( AlAreEqual( sf0, oldDag ) )
	{
		// Toss our old wrapper and replace it with a new one.
		delete sf0;
		sf0 = newDag->copyWrapper()->asSurfaceNodePtr();
	}
	if( AlAreEqual( sf1, oldDag ) )
	{
		// Toss our old wrapper and replace it with a new one.
		delete sf1;
		sf1 = newDag->copyWrapper()->asSurfaceNodePtr();
	}
/*	if( AlAreEqual( sf2, oldDag ) )
	{
		// Toss our old wrapper and replace it with a new one.
		delete sf2;
		sf2 = newDag->copyWrapper()->asSurfaceNodePtr();
	}*/

	return kOkay;
}

int genericCmd::declareReferences()
//
//	Declare any references to constructors and targets.
//	The constructors are the inputs to the command and
//	the targets are the outputs.  By setting this association,
//	Alias will know to call the methods implemented in
//	the plug-in for modifications to the constructor and
//	target dags.
//
{
printf("genericCmd::declareReferences()\n");

	if ( domain != NULL )
		addConstructorRef( domain );
	if ( sf0 != NULL )
		addConstructorRef( sf0 );
	if ( sf1 != NULL )
		addConstructorRef( sf1 );
//	if ( sf2 != NULL )
//		addTargetRef( sf2 );
	
	return kOkay;
}

int genericCmd::geometryModified( AlDagNode *dag )
//
//	The geometry for the constructor dags has been modified.
//
 {
	if ( dag == NULL )
		return -1;
		
//	if ( dag->name() != NULL )
//		printf("genericCmd::geometryModified( %s )\n",dag->name());
		
	// If the parameter dag is the same as one of our dags
	// then we don't have to do much.
	if( AlAreEqual( domain, dag) )
	{
		mmdf = 222;
		return kOkay;
	}
	if( AlAreEqual( sf0, dag) )
	{
		if((suv->uDegree() == ggu) && (suv->vDegree() == ggv))
			mmdf = 222;
		else
			mmdf = 333;
		return kOkay;
	}
	if( AlAreEqual( sf1, dag) )
	{
		if((pnbs->uDegree() == dru) && (pnbs->vDegree() == drv))
			mmdf = 555;
		else
			mmdf = 333;
		return kOkay;
	}
/*	if( AlAreEqual( sf2, dag) )
	{
		return kOkay;
	}*/


	// If we have gotten to this point, then one of the
	// dags our command knows about has changed.  Free
	// the dags up and let the command know it has been
	// modified.
	delete domain;	domain = NULL;
	delete sf0;	sf0 = NULL;
	delete sf1;	sf1 = NULL;
//	delete sf2;	sf2 = NULL;

	// Signal that the command has been modified by making the
	// appropriate call.
	AlCommand *cmd = command();
	cmd->modified();
	delete cmd;

	return kOkay;	
}

int genericCmd::dagModified( AlDagNode *dag )
//
//	The dag was modified.  This method will be called if the
//	dag is translated etc.
//
{
	if ( dag == NULL )
		return -1;
		
	if ( dag->name() != NULL )
		printf("genericCmd::dagModified( %s )\n",dag->name());
		
	// This method does not need to do much for this plug-in.
	if( ( AlIsValid( dag ) && AlIsValid( domain ) && AlAreEqual( dag, domain ) ) || 
		( AlIsValid( dag ) && AlIsValid( sf0 ) && AlAreEqual( dag, sf0 ) ) ||
		( AlIsValid( dag ) && AlIsValid( sf1 ) && AlAreEqual( dag, sf1 ) ))
	{

	} 
	return kOkay;
}

int genericCmd::debug( const char *prefix )
{
	if ( prefix != NULL )
		printf("genericCmd::debug( %s )\n",prefix);
		
	return kOkay;
}

int genericCmd::undo()
//
//	Undo everything the 'execute' did.  The cmdData class would need
//	to store the previous state of the world so we can undo one
//	step.
//
//	Note:  for this simple example undo does not need to be written.
//	If a user transforms the constructors curves and then undo's the
// 	transform from the Alias Edit menu, the ::execute() command will 
//	be called and the curves redrawn properly because of the dag
//	modification handler.
//
{
printf("genericCmd::undo() called\n");
	return kOkay;
}

int genericCmd::listModifiedDagNodes( const AlNotifyDagNode *dagMod, AlObject *obj )
//
//	This routine should call dagMod->notify on every dag node that might
//	be affected if obj is modified.
//	In our example, if one of the constructor is modified, we
//	call the notify() method on the targets of the command.
//
{
printf("genericCmd::listModifiedDagNodes() called\n");
	
	if ( dagMod == NULL || obj == NULL )
		return -1;
		
	if ( AlAreEqual( obj, domain->copyWrapper() ) || AlAreEqual( obj, sf0->copyWrapper() ) || AlAreEqual( obj, sf1->copyWrapper() ) )
	{
//		dagMod->notify( domain );
//		dagMod->notify( sf0 );
//		dagMod->notify( sf1 );
//		dagMod->notify( sf2 );
	}

	return -1;
}

statusCode genericCmd::retrieveWire( AlInput *input )
//
//	Handler called by the Alias retrieve code for the construction
//	history objects.
//

{
	AlData *dataod = new AlData;
	const int *odat;
	printf("genericCmd::retrieveWire()\n");

	// Replace the old pointers with the newly relocated ones.  The
	// order of resolving must be the same as the declare when
	// we store.
	
	AlObject *objDag  = input->resolveObject();
	AlObject *objDag2 = input->resolveObject();
	AlObject *objDag3 = input->resolveObject();
	AlObject *objDag4 = input->resolveObject();
	dataod = input->resolveData(0);
	
	if ( !objDag || ! objDag2 || !objDag3 || !objDag4 )
		return sFailure;
	
	// If a pointer was not resolved, then NULL is returned
	//
	// This can happen if the geometry has been deleted when
	// the plug-in is not loaded.
	// Alternatively, we could have just returned sSuccess.
	// When our command is executed, it will be invalid and so it will be
	// deleted.

	domain  = objDag->asCurveNodePtr(); 
	sf0 = objDag2->asSurfaceNodePtr();
	sf1    = objDag3->asSurfaceNodePtr();
	sf2   = objDag4->asSurfaceNodePtr();
	odat = dataod->asIntPtr();
	od = *odat;

	curve = domain->curve();
	suv = sf0->surface();
	pnbs = sf1->surface();
	nrbs = sf2->surface();
	zt = NULL;
	cmt_sz = NULL;
	cmt = NULL;

	if ( !domain || !sf0 || !sf1 || !sf2  )
		return sFailure;
	/*	
	// Test info;
	if ( charData )
		delete charData;
	int i;
	if ( ( charData = input->resolveData( AlData::kDataChar, kCharId )) == NULL )
		return sFailure;
	printf("char data <%s>\n",charData->asCharPtr());	// Null terminated string.
		
	if ( intData )
		delete intData;
	if ( ( intData = input->resolveData( AlData::kDataInt, kIntId ) ) == NULL )
		return sFailure;
	const int *idata = intData->asIntPtr();
	printf("int data <");
	for ( i = 0; i< intData->count(); i++ )
		printf("%d ",idata[i]);
	printf(">\n");
	
	if ( doubleData )
		delete doubleData;
	if ( ( doubleData = input->resolveData( AlData::kDataDouble, kDoubleId ) ) == NULL )
		return sFailure;
	printf("double data <");
	const double *ddata = doubleData->asDoublePtr();
	for ( i = 0; i< doubleData->count(); i++ )
		printf("%g ",ddata[i]);
	printf(">\n");
	*/
	return sSuccess;

}

statusCode genericCmd::storeWire( AlOutput *output )
//
//	This routine is the handler called by the Alias store code so
//	that it can get a pointer to the construction history plug-ins
//	data.
//

{
	int count;
	int odat ;
	AlData *data1 = new AlData;

	odat = od;

printf("genericCmd::storeWire()\n");

	if ( output == NULL )
		return sFailure;

	// Declare all of our references to data so that we can get them back
	// on retrieval.  We are telling Alias to keep track of these
	// pointers in the wire file.
	count = 1;
	data1->create(kIntId,&odat,count);
	output->declareObject( domain );
	output->declareObject( sf0 );
	output->declareObject( sf1 );
	output->declareObject( sf2 );
	output->declareData(data1);
/*
	// Test info
	AlData *data1 = new AlData;;
	char *cdata = "Save this data.";
	count = strlen( cdata ) + 1;
	if ( data1->create( kCharId, cdata, count ) != sSuccess )
		return sFailure;
	if ( output->declareData( data1 ) != sSuccess )
		return sFailure;

	AlData *data2 = new AlData;
	int idata[10] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
	count = 10;
	if ( data2->create( kIntId, idata, count ) != sSuccess )
		return sFailure;
	if ( output->declareData( data2 ) != sSuccess )
		return sFailure;

	AlData *data3 = new AlData;;
	double ddata[9] = { 10.1, 11.1, 12.1, 13.1, 14.2, 15.1, 16.1, 17.1, 18.1 };
	count = 9;
	if ( data3->create( kDoubleId, ddata, count ) != sSuccess )
		return sFailure;
	if ( output->declareData( data3 ) != sSuccess )
		return sFailure;
	*/
	return sSuccess;

}

statusCode genericCmd::set( AlDagNode *firstDag, AlDagNode *secondDag,AlDagNode *thirdDag, AlDagNode *fourthDag, int d)
//
//	Our helper function for setting the cmdData constructors.
//
{
	if ( firstDag == NULL || secondDag == NULL || thirdDag == NULL || fourthDag == NULL)
		return sFailure;
		
	domain = (AlCurveNode *) firstDag->copyWrapper();
	sf0 = (AlSurfaceNode *) secondDag->copyWrapper();
	sf1 = (AlSurfaceNode *) thirdDag->copyWrapper();
	sf2 = (AlSurfaceNode *) fourthDag->copyWrapper();
	od  = d;
		
	curve = domain->curve();
	suv = sf0->surface();
	pnbs = sf1->surface();
	nrbs = sf2->surface();

	requireStrongValidity = FALSE;
	
	return sSuccess;
}

//
//	End command definition
//

//
//	Begin command invocation (the UI chunk of the code) 
//

AlUserCommand *allocLinkObjectCmd()
//
//	Allocate & return a new command.  This function will be passed
//	to the AlCommand::add() routine
//
{
	return new genericCmd;
}

void do_add_cmd( AlDagNode *firstDag, AlDagNode *secondDag, AlDagNode *thirdDag, AlDagNode *fourthDag, int d)
{
	AlCommand::setDebug( FALSE );	// Static member function call
	AlCommand cmd;

	if( sSuccess == cmd.create( CMD_NAME ) )
	{
		genericCmd *g_cmd = (genericCmd *)cmd.userCommand()->asDerivedPtr();
			g_cmd->set( firstDag, secondDag, thirdDag, fourthDag, d);
			if( cmd.execute() == 0 )
				cmd.install();
			else
				cmd.deleteObject();
	}
}



static selctfc hFunc;

int selctfc::go_button_pressed( void )
{
	const char *pp;
	pp = hFunc.goButtonPressed();

	if (*pp == 'L')
	{
		hFunc.odu = 0;
		return 1;
	}
	if (*pp == 'R')
	{
		hFunc.odu = 1;
		return 1;
	}

	if(*pp == 'g')
	{
		if ((hFunc.pt1) != NULL && (hFunc.sf1) != NULL && (hFunc.sf2) != NULL && (hFunc.sf3) != NULL )
		{
			do_add_cmd( hFunc.pt1, hFunc.sf1, hFunc.sf2, hFunc.sf3, hFunc.odu);
			hFunc.clearGoButton(1);
			hFunc.finished();	
			hFunc.deleteObject();
		}
		return 1;
	}
	
		//hFunc.createGoButton( go_button_pressed, 1,"go","L","R" );
	return 0;
}

void selctfc::init_func( void ) {
	AlPickList::clearPickList();
	hFunc.pt1 = NULL;
	hFunc.sf1 = NULL;
	hFunc.sf2 = NULL;
	hFunc.sf3 = NULL;
	hFunc.odu = 0;
	hFunc.createGoButton( go_button_pressed, 1, "go","L","R" );
}

 void selctfc::down_func(int input, Screencoord x, Screencoord y )
{
	AlObject *firstPickedItem;
	firstPickedItem = NULL;
	AlPickList::pickFromScreen(x,y);
    if( AlPickList::firstPickItem() == sSuccess)
	{
	 AlObject *pickedItem = AlPickList::getObject();

	 if( pickedItem )
		firstPickedItem = pickedItem->copyWrapper();

		if(hFunc.pt1 == NULL)
		{
			if(firstPickedItem->asCurveNodePtr())
				hFunc.pt1 = firstPickedItem->asCurveNodePtr();
		}
		else 
			if(hFunc.sf1 == NULL)
			{
				if(firstPickedItem->asSurfaceNodePtr())
					hFunc.sf1 = firstPickedItem->asSurfaceNodePtr();
			}
			else 
				if(hFunc.sf2 == NULL)
				{
					if(firstPickedItem->asSurfaceNodePtr())
					hFunc.sf2 = firstPickedItem->asSurfaceNodePtr();
				}
			    else 
					if(hFunc.sf3 == NULL)
					{
						if(firstPickedItem->asSurfaceNodePtr())
						hFunc.sf3 = firstPickedItem->asSurfaceNodePtr();
					}
	}
	AlUniverse::redrawScreen(kRedrawActive);
}
void selctfc::move_func(int input, Screencoord x, Screencoord y )
{
}

void selctfc::up_func(int input, Screencoord x, Screencoord y )
{
}

void selctfc::cleanup_func() {
	hFunc.clearGoButton( TRUE );
}







static AlFunctionHandle h;


extern "C"
PLUGINAPI_DECL int plugin_init( const char *dirName )
{
	
	AlUniverse::initialize( kZUp );

	//
	// Create a new construction history command
	//

	if ( AlCommand::add(  allocLinkObjectCmd, CMD_NAME ) != sSuccess )
	{
		AlPrintf( kPrompt, "The linkObject plug-in failed to install.\n");
		return 1;
	}

	
	if ( hFunc.create( "Continuous",hFunc.init_func, hFunc.down_func, hFunc.move_func, 
								hFunc.up_func, hFunc.cleanup_func, TRUE ) != sSuccess )
		return 1;

	if ( h.create( "linkObject command", &hFunc ) != sSuccess )
		return 1;

	if ( h.setAttributeString( "linkObject plugin cmd" ) != sSuccess )
		return 1;
		
	if ( h.setIconPath( makeAltPath( dirName, NULL ) ) != sSuccess )
		return 1;
		
	if ( h.appendToMenu( "mp_objtools" ) != sSuccess )
		return 1;

	AlPrintf( kPrompt, "linkObject installed on Palette 'Object Edit'");
	return 0;
}

extern "C"
PLUGINAPI_DECL int plugin_exit( void )
{
	(void) AlCommand::remove(CMD_NAME );
	(void) h.deleteObject();
	(void) hFunc.deleteObject();
	
	// A redraw is required to ensure history
	// is no longer displayed for the plug-in's
	// entities.
	AlUniverse::redrawScreen( kRedrawAll );

	// do nothing
	return 0;
}


void zeros( double *ptr)
{
	int u,v,ui,vi;
	unsigned long sz,i,j,i1,j1;

	u = (int)ptr[0];
	v = (int)ptr[1];
	ui = (int)ptr[2];
	vi = (int)ptr[3];
	
	sz=(u*v*ui*vi+4);

	for(i=0;i<u;i++)
	{
		for(j=0;j<v;j++)
		{
			for(i1=0;i1<ui;i1++)
			{
				for(j1=0;j1<vi;j1++)
				{
					setmmxx(ptr,i,j,i1,j1,0.0);
				}
			}
		}
	}
}


void ones( double *ptr)
{
	int u,v,ui,vi;
	unsigned long sz,i,j,i1,j1;

	u = (int)ptr[0];
	v = (int)ptr[1];
	ui = (int)ptr[2];
	vi = (int)ptr[3];
	
	sz=(u*v*ui*vi+4);

	for(i=0;i<u;i++)
	{
		for(j=0;j<v;j++)
		{
			for(i1=0;i1<ui;i1++)
			{
				for(j1=0;j1<vi;j1++)
				{
					setmmxx(ptr,i,j,i1,j1,1.0);
				}
			}
		}
	}
}


void pascs( double *ptr)
{
	int a,b;
	unsigned long c;
	double pp = 1;
	unsigned long i,j,aa,bb;
	
	a = ptr[0] - 1;
	b = ptr[1] - 1;

	aa= ptr[0];bb= ptr[1];

	for(i=0;i<aa;i++)
	{
		for(j=0;j<bb;j++)
		{
			setmmxx(ptr,i,j,0,0,cnp(a,i)*cnp(b,j));
		}
	}
}

double cnp(double a,double b)
{
	return factr(a)/factr(b)/factr(a-b);
}

double factr(double a)
{
	double i,b=1;
	for(i=1;i<=a;i++)
	{
		b = b*i;
	}
	return b;
}


unsigned long precsnt(double *a,double *b)
{
	int u1,v1,u2,v2,u11,v11,u22,v22,u,v,ua,va;
	unsigned long  pm;

	u1=(int)a[0];
	v1=(int)a[1];
	u11=(int)a[2];
	v11=(int)a[3];
	u2=(int)b[0];
	v2=(int)b[1];
	u22=(int)b[2];
	v22=(int)b[3];
	u=u1+u2-1;
	v=v1+v2-1;
	ua=u11+u22-1;
	va=v11+v22-1;
	
	pm =u*v*ua*va+4;

	return pm;
}



void flwFM( double *a, double *b, double *c)
{
	int u1,v1,u2,v2,u11,v11,u22,v22,u,v,ua,va,i1,j1,pyp,elm;
	int uv1,uv2,i2,j2,ii1,jj1,ii2,jj2;
	int tmpa1,tmpa2,tmpa3,tmpb1,tmpb2,tmpb3,tmpab1,tmpab2,tmpab3;
	double tp=1,tmpa,tmpb,tmpc;
	double *pm,*ff,*ffa,*ffb;
	
	clearmm(c);

	u1=(int)a[0];
	v1=(int)a[1];
	u11=(int)a[2];
	v11=(int)a[3];
	u2=(int)b[0];
	v2=(int)b[1];
	u22=(int)b[2];
	v22=(int)b[3];
	u=u1+u2-1;
	v=v1+v2-1;
	ua=u11+u22-1;
	va=v11+v22-1;

	pm=c;

	ff = pm+4;
	ffa = a+4;
	ffb = b+4;
	uv1 = u11*v11;
	uv2 = u22*v22;
	elm = ua*va;

	for(i1=0;i1<u1;i1++)
	{
		for(j1=0;j1<v1;j1++)
		{
			for(i2=0;i2<u2;i2++)
			{
				for(j2=0;j2<v2;j2++)
				{
					for(ii1=0;ii1<u11;ii1++)
					{
						for(jj1=0;jj1<v11;jj1++)
						{
							tmpa=getmmxx(a,i1,j1,ii1,jj1);
							for(ii2=0;ii2<u22;ii2++)
							{
								#pragma omp parallel for
								for(jj2=0;jj2<v22;jj2++)
								{
									tmpb=getmmxx(b,i2,j2,ii2,jj2);
									tmpc=getmmxx(c,i1+i2,j1+j2,ii1+ii2,jj1+jj2);
									setmmxx(c,i1+i2,j1+j2,ii1+ii2,jj1+jj2,tmpc+tmpa*tmpb);
								}
							}
						}
					}
				}
			}
		}
	}






/*	for(i1=0;i1<u1;i1++)
	{
		tmpa1=i1*v1;
		for(j1=0;j1<v1;j1++)
		{
			tmpa2=(tmpa1+j1)*uv1;
			for(i2=0;i2<u2;i2++)
			{
				tmpb1=i2*v2;
				tmpab1=(i1+i2)*v;
				for(j2=0;j2<v2;j2++)
				{
					tmpb2=(tmpb1+j2)*uv2;
					tmpab2=(tmpab1+j1+j2)*elm;
					for(ii1=0;ii1<u11;ii1++)
					{
						tmpa3=ii1*v11+tmpa2;
						for(jj1=0;jj1<v11;jj1++)
						{
							tmpa = *(ffa+tmpa3+jj1);
							for(ii2=0;ii2<u22;ii2++)
							{
								tmpb3=ii2*v22+tmpb2;
								tmpab3=tmpab2+(ii1+ii2)*va+jj1;
								for(jj2=0;jj2<v22;jj2++)
								{
									tmpb = *(ffb+tmpb3+jj2);
									pyp=tmpab3+jj2;

									*(ff+pyp)=*(ff+pyp)+tmpa*tmpb;
								}
							}
						}
					}
				}
			}
		}
	}*/
}


void setmx(double *a,int u,int v,double nbr)
{
	int u1,v1;

	u1 = (int)a[0];
	v1 = (int)a[1];
	a[2+u*v1+v] = nbr;
}


double* cnstm(int u,int v,int ui,int vi)
{
	unsigned long sz,i;
	double p=1;

	sz=(u*v*ui*vi+4);

	double* ptr = new double [sz];

	ptr[0] = u;
	ptr[1] = v;
	ptr[2] = ui;
	ptr[3] = vi;

	for(i=4;i<sz;i++)
	{
		ptr[i] = 0xa3;
	}

	return ptr;
}

unsigned long cnst_size(int u,int v,int ui,int vi)
{
	unsigned long sz;
	sz=(u*v*ui*vi+4);
	return sz;
}

void cn_init( double *a,int u,int v,int u1,int v1)
{
	a[0] = u;
	a[1] = v;
	a[2] = u1;
	a[3] = v1;
	clearmm(a);
}

void clearmm(double *ptr)
{
	double p=1;
	int u,v,ui,vi;
	unsigned long sz,i,j,i1,j1;

	u = (int)ptr[0];
	v = (int)ptr[1];
	ui = (int)ptr[2];
	vi = (int)ptr[3];
	
	sz=(u*v*ui*vi+4);

	for(i=0;i<u;i++)
	{
		for(j=0;j<v;j++)
		{
			for(i1=0;i1<ui;i1++)
			{
				for(j1=0;j1<vi;j1++)
				{
					setmmxx(ptr,i,j,i1,j1,0.0);
				}
			}
		}
	}
}

void setmmxx(double *a,int u,int v,int u1,int v1,double nbr)
{
	int ua,va,ua2,va2,uva;

	ua = (int)a[0];
	va = (int)a[1];
	ua2 = (int)a[2];
	va2 = (int)a[3];

	uva = (u*va+v)*ua2*va2+u1*va2+v1;

	if((a+uva+4)<(zt0+ztsz0))
	{
		a[4+uva] = nbr;
	}
	else
	{
		_asm
		{
			int 3
		}
	}
}

double getmmxx(double *a,int u,int v,int u1,int v1)
{
	int ua,va,ua2,va2,uva;

	ua = (int)a[0];
	va = (int)a[1];
	ua2 = (int)a[2];
	va2 = (int)a[3];

	uva = (u*va+v)*ua2*va2+u1*va2+v1;
	if((a+uva+4)<(zt0+ztsz0))
	{
		return a[4+uva];
	}
	else
	{
		_asm
		{
			int 3
		}
		return 0;
	}
}

