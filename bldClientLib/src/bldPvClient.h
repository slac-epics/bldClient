#ifndef BLD_PV_CLIENT_H
#define BLD_PV_CLIENT_H

namespace EpicsBld
{   
/**
 * Abastract Interface of Bld PV Client 
 * 
 * The interface class for providing Bld PV Client functions.
 *
 * Design Issue:
 * 1. The value semantics are disabled. 
 */
class BldPvClientInterface
{
public:   
    virtual int bldStart() = 0;
    virtual int bldStop()  = 0;
    virtual bool IsStarted() const  = 0;
    virtual int bldConfig( const char* sAddr, unsigned short uPort, 
      unsigned int uMaxDataSize, const char* sInterfaceIp, 
      unsigned int uSrcPyhsicalId, unsigned int uDataType, 
      const char* sBldPvPreTrigger, const char* sBldPvPostTrigger,
      const char* sBldPvFiducial, const char* sBldPvList )  = 0;
    virtual void bldShowConfig()  = 0;

    // To be called by the init function of subroutine record
    virtual int bldSetPreSub( const char* sBldSubRec )  = 0; 
    virtual int bldSetPostSub( const char* sBldSubRec )  = 0; 
    
    // To be called by trigger variables (subroutine records)
    virtual int bldPrepareData() = 0;       
    virtual int bldSendData() = 0; 
    
    // debug information control
    virtual void setDebugLevel(int iDebugLevel) = 0;
    virtual int getDebugLevel() = 0;
    
    virtual ~BldPvClientInterface() {} /// polymorphism support
protected:  
    BldPvClientInterface() {} /// To be called from implementation classes
private:
    ///  Disable value semantics. No definitions (function bodies).
    BldPvClientInterface(const BldPvClientInterface&);
    BldPvClientInterface& operator=(const BldPvClientInterface&);
};

/**
 * Factory class of Bld PV Client 
 *
 * Design Issue:
 * 1. Object semantics are disabled. Only static utility functions are provided.
 * 2. Later if more factorie classes are needed, this class can have public 
 *    constructor, virtual destructor and virtual member functions.
 */
class BldPvClientFactory
{
public:
    /**
     * Get the singelton Bld PV Client object
     *
     * @return              The Bld PV Client singelton object
     *
     * OK, this name is now *completely* wrong, as it's not a singleton.
     */
    static BldPvClientInterface& getSingletonBldPvClient(int id);
        
private:
    /// Disable object instantiation (No object semantics).
    BldPvClientFactory();
};

}

extern "C"
{
/* 
 * The following functions provide C wrappers for accesing 
 * EpicsBld::BldPvClientInterface, combined with BldPvClientFactory functions
 *
 * Since BldPvClientInterface is designed for singleton objects, the functions 
 * uses the factory class BldPvClientFactory to get singleton object and operate
 * on the singelton objectdirectly. Therefore no object lifetime management 
 * function is provided.
 */
int BldStart(int id);
int BldStop(int id);
bool BldIsStarted(int id);
int BldSetControl(int id, int on);

int BldConfig( int id, const char* sAddr, unsigned short uPort, unsigned int uMaxDataSize, const char* sInterfaceIp, 
               unsigned int uSrcPhysicalId, unsigned int uXtcDataType, const char* sBldPvPreTrigger,
               const char* sBldPvPostTrigger, const char* sBldPvFiducial, const char* sBldPvList );
void BldShowConfig(int id);

int BldSetPreSub(int id, const char* sBldSubRec); 
int BldSetPostSub(int id, const char* sBldSubRec);
int BldPrepareData(int id); 
int BldSendData(int id); 

void BldSetDebugLevel(int id, int iDebugLevel); 
int BldGetDebugLevel(int id); 

#define	FIDUCIAL_NOT_SET	0x20000
#define FIDUCIAL_INVALID	0x1FFFF

}

#endif
