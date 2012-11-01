#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <dbDefs.h>
#include <registryFunction.h>
#include <subRecord.h>
#include <epicsExport.h>
#include <dbAddr.h>
#include <dbAccess.h>
#include <dbTest.h>

#include "bldPvClient.h"
#include "bldNetworkClient.h"
#include "bldClientSub.h"
#include "bldPacket.h"

/*
 * Global C function definitions
 */
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
int BldStart(int id)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(id).bldStart();
}

int BldStop(int id)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(id).bldStop();    
}

bool BldIsStarted(int id)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(id).IsStarted();
}

int BldSetControl(int id, int on)
{
    if (on) {
        if (!BldIsStarted(id))
            return BldStart(id);
    } else {
        if (BldIsStarted(id))
            return BldStop(id);
    }
    return 1;
}

int BldConfig(int id, const char* sAddr, unsigned short uPort, 
  unsigned int uMaxDataSize, const char* sInterfaceIp, unsigned int uSrcPhysicalId, 
  unsigned int uxtcDataType, const char* sBldPvPreTrigger, const char* sBldPvPostTrigger, 
  const char* sBldPvFiducial, const char* sBldPvList )
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(id).bldConfig(
            sAddr, uPort, uMaxDataSize, sInterfaceIp, uSrcPhysicalId, uxtcDataType, sBldPvPreTrigger,
            sBldPvPostTrigger, sBldPvFiducial, sBldPvList);
}

void BldShowConfig(int id)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(id).bldShowConfig();
}

int BldSetPreSub(int id, const char* sBldSubRec)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(id).bldSetPreSub(sBldSubRec);
}

int BldSetPostSub(int id, const char* sBldSubRec)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(id).bldSetPostSub(sBldSubRec);
}

int BldPrepareData(int id)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(id).bldPrepareData();
}

int BldSendData(int id)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(id).bldSendData();
}

void BldSetDebugLevel(int id, int iDebugLevel)
{
    EpicsBld::BldPvClientFactory::getSingletonBldPvClient(id).setDebugLevel(iDebugLevel);
}

int BldGetDebugLevel(int id)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(id).getDebugLevel();
}
 
} // extern "C"

using std::string;
using std::size_t;

/*
 * local class declarations
 */
namespace EpicsBld
{   
/**
 * Class for sending out Bld data
 *
 * design issue: Singleton class
 */
class BldPvClientBasic : public BldPvClientInterface
{
public: 
    virtual int bldStart();
    virtual int bldStop();
    virtual bool IsStarted() const;
    virtual int bldConfig( const char* sAddr, unsigned short uPort,
      unsigned int uMaxDataSize, const char* sInterfaceIp, 
      unsigned int uSrcPhysicalId, unsigned int uxtcDataType, 
      const char* sBldPvPreTrigger, const char* sBldPvPostTrigger,
      const char* sBldPvFiducial, const char* sBldPvList );
    virtual void bldShowConfig();

    // To be called by the init function of subroutine record
    virtual int bldSetPreSub( const char* sBldSubRec ); 
    virtual int bldSetPostSub( const char* sBldSubRec ); 

    // To be called by trigger variables (subroutine records)      
    virtual int bldPrepareData(); 
    virtual int bldSendData(); 

    // debug information control
    virtual void setDebugLevel(int iDebugLevel);
    virtual int getDebugLevel();    
    
    static BldPvClientBasic& getSingletonObject(int id); // singelton interface
    
private:
    bool _bBldStarted;
    std::auto_ptr<EpicsBld::BldNetworkClientInterface> _apBldNetworkClient;
    int _iDebugLevel;
    
    string          _sBldPvPreSubRec, _sBldPvPostSubRec,;
    unsigned int    _uBldServerAddr;
    unsigned short  _uBldServerPort;
    unsigned int    _uMaxDataSize;
    string          _sBldInterfaceIp;    
    unsigned int    _uSrcPhysicalId, _uxtcDataType;
    string          _sBldPvPreTrigger, _sBldPvPostTrigger, _sBldPvFiducial;
    string          _sBldPvList, _sBldPvPreTriggerPrevFLNK, _sBldPvPostTriggerPrevFLNK;
    unsigned int    _uFiducialIdCur;
    
     BldPvClientBasic(); /// Singleton. No explicit instantiation
     ~BldPvClientBasic();
          
     ///  Disable value semantics. No definitions (function bodies).
     BldPvClientBasic(const BldPvClientBasic&); 
     BldPvClientBasic& operator=(const BldPvClientBasic&);

    /*
     * Utility varaibles and functions
     */
#define iMTU 9000   // Ethernet packet MTU
    static const char sPvListSeparators[];

    long llBufPvVal[iMTU / sizeof(long)]; // Align with long int boundaries
    char lcMsgBuffer[iMTU];

    static int _splitPvList( const string& sBldPvList, std::vector<string>& vsBldPv );
    
    /* PV access and report */    
    static int readPv(const char *sVariableName, int iBufferSize, void* pBuffer, 
      short* piValueType, long* plNumElements );
    static int writePv(const char *sVariableName, const void* pBuffer, 
      short iValueType = DBR_STRING, long lNumElements = 1);
    static int printPv(const char *sVariableName, void* pBuffer, 
      short iValueType = DBR_STRING, long lNumElements = 1 );
    static int sprintPv(const char *sVariableName, void* pBuffer, 
      int iMsgBufferSize, char* lcMsgBuffer, int* piRealMsgSize, 
      short iValueType = DBR_STRING, long lNumElements = 1 );
};

}

/*
 * class member definitions
 */
namespace EpicsBld
{   
/**
 * class BldPvClientFactory
 */
BldPvClientInterface& BldPvClientFactory::getSingletonBldPvClient(int id)
{
    return BldPvClientBasic::getSingletonObject(id);
}

/*
 * class BldPvClientBasic
 */

/*
 * private static variables
 */

const char BldPvClientBasic::sPvListSeparators[] = " ,;\r\n";

/* static member functions */
 
BldPvClientBasic& BldPvClientBasic::getSingletonObject(int id)
{
    static BldPvClientBasic bldDataClient[10]; // Ick!
    return bldDataClient[id];
}

/* public member functions */

BldPvClientBasic::BldPvClientBasic() : _bBldStarted(false), _iDebugLevel(0),
  _uBldServerAddr(0), _uBldServerPort(0), _uMaxDataSize(0), _uSrcPhysicalId(0), _uxtcDataType(0)
{
}

BldPvClientBasic::~BldPvClientBasic()
{
    if ( _bBldStarted )
        bldStop();
}

void BldPvClientBasic::setDebugLevel(int iDebugLevel)
{
    _iDebugLevel = iDebugLevel;
    
    if ( _apBldNetworkClient.get() != NULL )
        _apBldNetworkClient->setDebugLevel( _iDebugLevel );
}

int BldPvClientBasic::getDebugLevel()
{
    return _iDebugLevel;
}

int BldPvClientBasic::bldStart()
{   
    if ( _bBldStarted )
        return 1; // return status, without error report

    printf( "Starting bld:\n" );
        
try
{
    const unsigned char ucTTL = 32; /// minimum: 1 + (# of routers in the middle)
            
    _apBldNetworkClient.reset(
      EpicsBld::BldNetworkClientFactory::createBldNetworkClient(_uBldServerAddr, _uBldServerPort, 
      _uMaxDataSize, ucTTL, _sBldInterfaceIp.c_str() )
      );
    if ( _apBldNetworkClient.get() == NULL )
        throw string("BldNetworkClient Init fail\n");

    _apBldNetworkClient->setDebugLevel( _iDebugLevel );
    
    /*
     * setup forward link:  _sBldPvPreTrigger -> _sBldPvPreSubRec -> _sBldPvPreTriggerPrevFLNK
     */
    if ( !_sBldPvPreTrigger.empty() )
    {
        short int iFieldType = 0;
        long lNumElements = 0;
        char* lcBufPvVal = (char*) llBufPvVal;
        llBufPvVal[0] = 0;

        // Read PV: (_sBldPvPreTrigger).FLNK
        // If it has ".XXX" at the end, no need to append ".FLNK"!
        string sBldPvPreTriggerFieldFLNK = 
            (_sBldPvPreTrigger.find('.') == string::npos) ? _sBldPvPreTrigger + ".FLNK"
                                                          : _sBldPvPreTrigger;
        if ( 
          readPv( sBldPvPreTriggerFieldFLNK.c_str(), sizeof(llBufPvVal), llBufPvVal, &iFieldType, &lNumElements )
          != 0 )
            throw string("readPv(") + sBldPvPreTriggerFieldFLNK + ") Failed\n";
        _sBldPvPreTriggerPrevFLNK.assign(lcBufPvVal);
        
        // Set PV: (_sBldPvPreSubRec).FLNK = (_sBldPvPreTrigger).FLNK
        if ( _sBldPvPreTriggerPrevFLNK == "0" ) // special case: "0" means NO FLNK
            _sBldPvPreTriggerPrevFLNK.clear();
        else if ( !_sBldPvPreTriggerPrevFLNK.empty() )
        {
            string sBldPvSubRecFieldFLNK = _sBldPvPreSubRec + ".FLNK";
            if ( _iDebugLevel >= 1 )
                printf( "Setting PV FLNK: %s = %s\n", sBldPvSubRecFieldFLNK.c_str(), 
                  _sBldPvPreTriggerPrevFLNK.c_str() );
            if ( 
              writePv( sBldPvSubRecFieldFLNK.c_str(), lcBufPvVal )
              != 0 )
                throw string("writePv(") + sBldPvSubRecFieldFLNK + ") Failed\n";    
        }

        // Set PV: (_sBldPvPreTrigger).FLNK = _sBldPvPreSubRec
        int iLenPvVal = _sBldPvPreSubRec.length();
        strncpy( lcBufPvVal, _sBldPvPreSubRec.c_str(), iLenPvVal );
        if ( _iDebugLevel >= 1 )
            printf( "Setting PV FLNK: %s = %s\n", sBldPvPreTriggerFieldFLNK.c_str(), 
              _sBldPvPreSubRec.c_str() );
        if ( 
          writePv( sBldPvPreTriggerFieldFLNK.c_str(), lcBufPvVal )
          != 0 )
            throw string("writePv(") + sBldPvPreTriggerFieldFLNK + ") Failed\n";            
    }
        
    
    /*
    * setup forward link:  _sBldPvPostTrigger -> _sBldPvPostSubRec -> _sBldPvPostTriggerPostvFLNK
    */
    if ( !_sBldPvPostTrigger.empty() )
    {
        short int iFieldType = 0;
        long lNumElements = 0;
        char* lcBufPvVal = (char*) llBufPvVal;
        llBufPvVal[0] = 0;

        // Read PV: (_sBldPvPostTrigger).FLNK
        string sBldPvPostTriggerFieldFLNK = _sBldPvPostTrigger + ".FLNK";
        if ( 
             readPv( sBldPvPostTriggerFieldFLNK.c_str(), sizeof(llBufPvVal), llBufPvVal, &iFieldType, &lNumElements )
             != 0 )
            throw string("readPv(") + sBldPvPostTriggerFieldFLNK + ") Failed\n";
        _sBldPvPostTriggerPrevFLNK.assign(lcBufPvVal);
        
        // Set PV: (_sBldPvPostSubRec).FLNK = (_sBldPvPostTrigger).FLNK
        if ( _sBldPvPostTriggerPrevFLNK == "0" ) // special case: "0" means NO FLNK
            _sBldPvPostTriggerPrevFLNK.clear();
        else if ( !_sBldPvPostTriggerPrevFLNK.empty() )
        {
            string sBldPvSubRecFieldFLNK = _sBldPvPostSubRec + ".FLNK";
            if ( _iDebugLevel >= 1 )
                printf( "Setting PV FLNK: %s = %s\n", sBldPvSubRecFieldFLNK.c_str(), 
                        _sBldPvPostTriggerPrevFLNK.c_str() );
            if ( 
                 writePv( sBldPvSubRecFieldFLNK.c_str(), lcBufPvVal )
                 != 0 )
                throw string("writePv(") + sBldPvSubRecFieldFLNK + ") Failed\n";    
        }

        // Set PV: (_sBldPvPostTrigger).FLNK = _sBldPvPostSubRec
        int iLenPvVal = _sBldPvPostSubRec.length();
        strncpy( lcBufPvVal, _sBldPvPostSubRec.c_str(), iLenPvVal );
        if ( _iDebugLevel >= 1 )
            printf( "Setting PV FLNK: %s = %s\n", sBldPvPostTriggerFieldFLNK.c_str(), 
                    _sBldPvPostSubRec.c_str() );
        if ( 
             writePv( sBldPvPostTriggerFieldFLNK.c_str(), lcBufPvVal )
             != 0 )
            throw string("writePv(") + sBldPvPostTriggerFieldFLNK + ") Failed\n";            
    }    
    
}   
catch (string& sError)
{
    printf( "[FAILED]\n" );    
    printf( "BldPvClientBasic::bldStart() : %s\n", sError.c_str() );     
    return 2;
}

    printf( "[OK]\n" );    
    _bBldStarted = true;
    return 0;
}


int BldPvClientBasic::bldStop()
{
    if ( !_bBldStarted )
        return 1; // return status, without error report
        
    printf( "Shutting down bld:\n" );
    
try
{        
    /*
     * reset forward link:  _sBldPvPreTrigger -> _sBldPvPreTriggerPrevFLNK , _sBldPvPreSubRec ->  ""
     */
    if ( !_sBldPvPreTrigger.empty() )
    {
        char* lcBufPvVal = (char*) llBufPvVal;
        llBufPvVal[0] = 0;
        
        // Set PV: (_sBldPvPreTrigger).FLNK = ""
        int iLenPvVal = _sBldPvPreTriggerPrevFLNK.length();
        strncpy( lcBufPvVal, _sBldPvPreTriggerPrevFLNK.c_str(), iLenPvVal );
        // If it has ".XXX" at the end, no need to append ".FLNK"!
        string sBldPvPreTriggerFieldFLNK = 
            (_sBldPvPreTrigger.find('.') == string::npos) ? _sBldPvPreTrigger + ".FLNK"
                                                          : _sBldPvPreTrigger;
        if ( _iDebugLevel >= 1 )
            printf( "Setting PV FLNK: %s = %s\n", sBldPvPreTriggerFieldFLNK.c_str(), 
              _sBldPvPreTriggerPrevFLNK.c_str() );
        if ( 
          writePv( sBldPvPreTriggerFieldFLNK.c_str(), lcBufPvVal )
          != 0 )
            throw string("writePv(") + sBldPvPreTriggerFieldFLNK + ") Failed\n";
            
        if ( !_sBldPvPreTriggerPrevFLNK.empty() )
        {
            lcBufPvVal[0] = 0; // set lcBufPvVal = ""
            string sBldPvSubRecFieldFLNK = _sBldPvPreSubRec + ".FLNK";
            if ( _iDebugLevel >= 1 )
                printf( "Setting PV FLNK: %s = 0\n", sBldPvSubRecFieldFLNK.c_str() );            
            if ( 
              writePv( sBldPvSubRecFieldFLNK.c_str(), lcBufPvVal )
              != 0 )
                throw string("writePv(") + sBldPvSubRecFieldFLNK + ") Failed\n";                        
        }
    }    

    _sBldPvPreTriggerPrevFLNK.clear();
    
    /*
    * reset forward link:  _sBldPvPostTrigger -> _sBldPvPostTriggerPrevFLNK , _sBldPvPostSubRec ->  ""
    */
    if ( !_sBldPvPostTrigger.empty() )
    {
        char* lcBufPvVal = (char*) llBufPvVal;
        llBufPvVal[0] = 0;
        
        // Set PV: (_sBldPvPostTrigger).FLNK = ""
        int iLenPvVal = _sBldPvPostTriggerPrevFLNK.length();
        strncpy( lcBufPvVal, _sBldPvPostTriggerPrevFLNK.c_str(), iLenPvVal );
        string sBldPvPostTriggerFieldFLNK = _sBldPvPostTrigger + ".FLNK";
        if ( _iDebugLevel >= 1 )
            printf( "Setting PV FLNK: %s = %s\n", sBldPvPostTriggerFieldFLNK.c_str(), 
                    _sBldPvPostTriggerPrevFLNK.c_str() );
        if ( 
             writePv( sBldPvPostTriggerFieldFLNK.c_str(), lcBufPvVal )
             != 0 )
            throw string("writePv(") + sBldPvPostTriggerFieldFLNK + ") Failed\n";
            
        if ( !_sBldPvPostTriggerPrevFLNK.empty() )
        {
            lcBufPvVal[0] = 0; // set lcBufPvVal = ""
            string sBldPvSubRecFieldFLNK = _sBldPvPostSubRec + ".FLNK";
            if ( _iDebugLevel >= 1 )
                printf( "Setting PV FLNK: %s = 0\n", sBldPvSubRecFieldFLNK.c_str() );            
            if ( 
                 writePv( sBldPvSubRecFieldFLNK.c_str(), lcBufPvVal )
                 != 0 )
                throw string("writePv(") + sBldPvSubRecFieldFLNK + ") Failed\n";                        
        }
    }    

    _sBldPvPostTriggerPrevFLNK.clear();
    
    _apBldNetworkClient.release();
}   
catch (string& sError)
{
    printf( "[FAILED]\n" );    
    printf( "BldPvClientBasic::bldStop() : %s\n", sError.c_str() );     
    return 2;
}

    printf( "[OK]\n" );    
    _bBldStarted = false;
    return 0;
}

int BldPvClientBasic::bldSetPreSub( const char* sBldSubRec )
{
    _sBldPvPreSubRec.assign(sBldSubRec);
        
    printf( "Bld PV Settings:\n  Subroutine Record <%s>\n",
      _sBldPvPreSubRec.c_str() );
    
    return 0;
}

int BldPvClientBasic::bldSetPostSub( const char* sBldSubRec )
{
    _sBldPvPostSubRec.assign(sBldSubRec);
        
    printf( "Bld PV Settings:\n  Subroutine Record <%s>\n",
            _sBldPvPostSubRec.c_str() );
    
    return 0;
}

int BldPvClientBasic::bldPrepareData()
{
    if ( !_bBldStarted )
        return 1; // return status, without error report

    int iRetErrorCode = 0;
    
try
{       
    short int iFieldType = 0;
    long lNumElements = 0;
    
    unsigned int uFiducialId = 0x1FFFF;
    if ( _sBldPvFiducial.length() > 0 )
    {
        if ( 
            readPv( _sBldPvFiducial.c_str(), sizeof(llBufPvVal), llBufPvVal, &iFieldType, &lNumElements )
            != 0 )
            throw string("readPv(") + _sBldPvFiducial + ") Failed to read Fiducial PV!\n";
                    
        uFiducialId  = *(unsigned long int*) llBufPvVal;
    }
 
    _uFiducialIdCur = uFiducialId;
    if ( _uFiducialIdCur >= FIDUCIAL_INVALID )
        throw string( "Invalid fiducial read from " + _sBldPvFiducial + "\n" );
}
catch (string& sError)
{
    printf( "BldPvClientBasic::bldPrepareData() : %s\n", sError.c_str() );
      
    iRetErrorCode = 2;
}
        
    if ( _iDebugLevel >= 3 )
        printf( "Preparing Data: Get Fiducial Id 0x%05X\n", _uFiducialIdCur ); 
        
    return iRetErrorCode;
    
}

int BldPvClientBasic::bldSendData()
{   
    if ( !_bBldStarted )
        return 1; // return status, without error report

    int iRetErrorCode = 0;
    
try
{
    if ( _apBldNetworkClient.get() == NULL )
        throw string( "BldNetworkClient is uninitialized\n" );

    short int iFieldType = 0;
    long lNumElements = 0;

    std::vector<string> vsBldPv;
    _splitPvList( _sBldPvList, vsBldPv );
    
    BldPacketHeader* pBldPacketHeader = (BldPacketHeader*) lcMsgBuffer;

    /* Set bld packet header */    
    struct timespec ts;
    int iStatus = clock_gettime (CLOCK_REALTIME, &ts);
    if (iStatus)
        throw string( "clock_gettime() Failed\n" );
        
    unsigned int uFiducialId = _uFiducialIdCur;
    _uFiducialIdCur = FIDUCIAL_NOT_SET;
    if ( _iDebugLevel >= 2 )
	{
		if ( uFiducialId >= FIDUCIAL_NOT_SET )
		{
			// throw string( "Fiducial not set.  Did your bldPreTrigger PV process?\n" );
			printf( "Fiducial not set.  Did your bldPreTrigger PV process?\n" );
		}
        printf( "Sending Bld to Addr %x Port %d Interface %s Fiducial 0x%05X\n",
          _uBldServerAddr, _uBldServerPort, _sBldInterfaceIp.c_str(), uFiducialId ); 
	}
    if ( _iDebugLevel >= 3 )
        printf( "PvList %s\n", _sBldPvList.c_str() );
 
    const unsigned int uDamage = 0;
    
    new ( pBldPacketHeader ) BldPacketHeader( sizeof(lcMsgBuffer), 
      ts.tv_sec, ts.tv_nsec, uFiducialId, uDamage, _uSrcPhysicalId, _uxtcDataType);
    //char* pcMsgBuffer = (char*) (pBldPacketHeader + 1);
    //unsigned int uDataSize = sizeof(BldPacketHeader);
    //const int iMaxMsgSize = sizeof(lcMsgBuffer);

    /* Set bld pv values */        
    int iPvIndex = 0;
    for ( std::vector<string>::const_iterator itsBldPv = vsBldPv.begin();
      itsBldPv != vsBldPv.end(); itsBldPv++, iPvIndex++ )
    {
        //// print out the separator "; " 
        //if ( itsBldPv != vsBldPv.begin() )
        //{
        //    *pcMsgBuffer++ = ';'; *pcMsgBuffer++ = ' ';            
        //    uDataSize += 2;
        //}
        
        const string& sBldPv = *itsBldPv;
        if ( _iDebugLevel >= 3 )
            printf( "Reading PV %s...\n", sBldPv.c_str());
        if ( 
          readPv( sBldPv.c_str(), sizeof(llBufPvVal), llBufPvVal, &iFieldType, &lNumElements )
          != 0 )
            throw string("readPv(") + sBldPv + ") Failed\n";
        
        //if ( _iDebugLevel >= 3)
        //    printf( "Msg Buffer before PV %s Available Size %d: %s\n", sBldPv.c_str(), iMaxMsgSize - uDataSize, lcMsgBuffer);
        
        int iFail = pBldPacketHeader->setPvValue( iPvIndex, llBufPvVal );
        if ( iFail != 0 )
            throw string("pBldPacketHeader->setPvValue() for PV ") + sBldPv + ") Failed\n";
        
        //int iNewMsgSize = 0;
        //if (
        //  sprintPv( sBldPv.c_str(), llBufPvVal, iMaxMsgSize - uDataSize, pcMsgBuffer, 
        //  &iNewMsgSize, iFieldType, lNumElements )
        //  != 0 )
        //    throw string("sprintPv(") + sBldPv + ") Failed\n";
        //        
        //pcMsgBuffer += iNewMsgSize;
        //uDataSize += iNewMsgSize;
        
        //if ( _iDebugLevel >= 3 )
        //    printf( "Msg Buffer after PV %s AvaSize %d: %s\n", sBldPv.c_str(), iMaxMsgSize - uDataSize, lcMsgBuffer);        
    }
    
    //if ( uDataSize > _uMaxDataSize )
    //    throw string("Data Size is larger than max value\n");
                
    /* Send out bld */    
    int iFailSend = _apBldNetworkClient->sendRawData( pBldPacketHeader->getPacketSize(), lcMsgBuffer);
    if ( iFailSend != 0 )
        throw string( "_apBldNetworkClient->sendRawData() Failed\n", _sBldPvList.c_str() );
}
catch (string& sError)
{
    printf( "BldPvClientBasic::bldSendData() : %s\n", sError.c_str() );
      
    iRetErrorCode = 2;
}
        
    return iRetErrorCode;
}

bool BldPvClientBasic::IsStarted() const
{
    return _bBldStarted;
}

int BldPvClientBasic::bldConfig( const char* sAddr, unsigned short uPort, 
  unsigned int uMaxDataSize, const char* sInterfaceIp, unsigned int uSrcPhysicalId, unsigned int uxtcDataType, 
  const char* sBldPvPreTrigger, const char* sBldPvPostTrigger, const char* sBldPvFiducial, const char* sBldPvList )
{   
    BldPacketHeader::Initialize();

    if ( _bBldStarted )
    {
        printf( "BldPvClientBasic::bldConfig() : Need to stop bld before config\n" );
        return 1;
    }
            
    // Check for valid parameters
    // Note: sInterfaceIp == NULL is okay, which means default NIC is used
    if ( sAddr == NULL || uMaxDataSize <= 0 || sBldPvPreTrigger == NULL || sBldPvPostTrigger == NULL || sBldPvList == NULL ) 
    {
        printf( "BldPvClientBasic::bldConfig() : Input parameter invalid\n" );
        return 2;
    }

    printf( "Configuring bld:\n" );
    
    _uBldServerAddr = ntohl( inet_addr( sAddr ) );
    _uBldServerPort = uPort;    
    _uMaxDataSize = uMaxDataSize;
    _sBldInterfaceIp.assign(sInterfaceIp == NULL? "" : sInterfaceIp);  
    _uSrcPhysicalId = uSrcPhysicalId;
    _uxtcDataType = uxtcDataType;
    _sBldPvPreTrigger.assign(sBldPvPreTrigger);
    _sBldPvPostTrigger.assign(sBldPvPostTrigger);
    _sBldPvFiducial.assign(sBldPvFiducial); 
    _sBldPvList.assign(sBldPvList);
                      
    bldShowConfig();
    
    return 0;
}

void BldPvClientBasic::bldShowConfig()
{
    unsigned int uServerNetworkAddr = htonl(_uBldServerAddr);
    unsigned char* pcAddr = (unsigned char*) &uServerNetworkAddr;
    printf( "  Configurable parameters:\n"
      "    Server Addr %u.%u.%u.%u  Port %d  MaxDataSize %u MCastIF %s\n"
      "    Source Id %d  Data Type %d\n"
      "    PvPreTrigger <%s>\n"
	  "    PvPostTrigger <%s>\n"
	  "    PvFiducial <%s>\n"
      "    PvList <%s>\n",
      pcAddr[0], pcAddr[1], pcAddr[2], pcAddr[3],
      _uBldServerPort, _uMaxDataSize, _sBldInterfaceIp.c_str(), 
      _uSrcPhysicalId, _uxtcDataType,
	  _sBldPvPreTrigger.c_str(), _sBldPvPostTrigger.c_str(),
	  _sBldPvFiducial.c_str(),   _sBldPvList.c_str() );

    printf( "  Internal Settings:\n"
      "    Pre  Subroutine Record <%s>  PvPreTrigger.FLNK <%s>\n"
      "    Post Subroutine Record <%s>  PvPostTrigger.FLNK <%s>\n"
      "    DebugLevel %d\n",
      _sBldPvPreSubRec.c_str(), _sBldPvPreTriggerPrevFLNK.c_str(),
      _sBldPvPostSubRec.c_str(), _sBldPvPostTriggerPrevFLNK.c_str(),
      _iDebugLevel );      
	if ( _sBldPvPreTrigger == _sBldPvPreSubRec )
	{
		printf( "WARNING: PvPreTrigger is same PV as %s!\n", _sBldPvPreSubRec.c_str() );
		printf( "Normally, PvPreTrigger should be the name of a local PV\n"
				"that processes when the fiducial is ready to fetch.\n" );
	}
	if ( _sBldPvPostTrigger == _sBldPvPostSubRec )
	{
		printf( "WARNING: PvPostTrigger is same PV as %s!\n", _sBldPvPostSubRec.c_str() );
		printf( "Normally, PvPostTrigger should be the name of a local PV\n"
				"that you want to process after the BLD data has been sent.\n" );
	}
}

/*
 * private static member functions
 */
int BldPvClientBasic::_splitPvList( const string& sBldPvList, std::vector<string>& vsBldPv )
{
    size_t	uOffsetStart = sBldPvList.find_first_not_of( sPvListSeparators, 0 );
    while ( uOffsetStart != string::npos )      
    {
        size_t uOffsetEnd = sBldPvList.find_first_of( sPvListSeparators, uOffsetStart+1 );

        if ( uOffsetEnd == string::npos )        
        {
            vsBldPv.push_back( sBldPvList.substr( uOffsetStart, string::npos ) );
            break;
        }
        
        vsBldPv.push_back( sBldPvList.substr( uOffsetStart, uOffsetEnd - uOffsetStart ) );
        uOffsetStart = sBldPvList.find_first_not_of( sPvListSeparators, uOffsetEnd+1 );        
    }
    return 0;
}

int BldPvClientBasic::readPv(
	const char	*	sVariableName,
	int				iBufferSize,
	void		*	pBuffer,
	short		*	piValueType,
	long		*	plNumElements	)
{
    if (	sVariableName == NULL || *sVariableName == 0
		||	pBuffer == NULL || piValueType == NULL || plNumElements == NULL )
    {
        printf( "readPv(): Invalid parameter\n" );
        return 1;
    }
    
    if ( (reinterpret_cast<unsigned long>(pBuffer) & 0x3) != 0  )
    {
        printf( "readPv(): Buffer should be aligned in 32 bits boundaries\n" );
        return 2;
    }
        
    DBADDR  dbaddrVariable;
    int iStatus = dbNameToAddr(sVariableName,&dbaddrVariable);  
    if ( iStatus != 0 )
    {
        printf("readPv(): dbNameToAddr(%s) failed. Status  = 0x%X\n", sVariableName, iStatus);
        return(iStatus);
    }

    long lNumElements = std::min( (int) dbaddrVariable.no_elements,
      (iBufferSize/dbaddrVariable.field_size) );
    long int lOptions=0;
    
    if(dbaddrVariable.dbr_field_type==DBR_ENUM) 
        iStatus = 
          dbGetField(&dbaddrVariable,DBR_STRING,pBuffer,&lOptions,
          &lNumElements,NULL);
    else
        iStatus = 
          dbGetField(&dbaddrVariable,dbaddrVariable.dbr_field_type,pBuffer,&lOptions,
          &lNumElements,NULL);          
          
    if ( iStatus != 0 )
    {
        printf("readPv(): dbGetField(%s) failed. Status  = 0x%X\n", sVariableName, iStatus);
        return(iStatus);
    }
        
    *piValueType = dbaddrVariable.dbr_field_type;
    *plNumElements = lNumElements;

    return(0);
}

int BldPvClientBasic::writePv(const char *sVariableName, const void* pBuffer, short iValueType, long lNumElements )
{
    if (sVariableName == NULL || *sVariableName == 0 || pBuffer == NULL )
    {
        printf( "writePv(): Invalid parameter\n" );
        return 1;
    }
    
    if ( (reinterpret_cast<unsigned long>(pBuffer) & 0x3) != 0  )
    {
        printf( "writePv(): Buffer should be aligned in 32 bits boundaries\n" );
        return 2;
    }

    DBADDR  dbaddrVariable;
    int iStatus = dbNameToAddr(sVariableName,&dbaddrVariable);  
    if ( iStatus != 0 )
    {
        printf("writePv(): dbNameToAddr(%s) failed. Status  = %d\n", sVariableName, iStatus);
        return(iStatus);
    }
    
    if(iValueType==DBR_ENUM) 
    {
        unsigned short suValue = (unsigned short) atoi((char*)pBuffer);

        iStatus = 
          dbPutField( &dbaddrVariable, DBR_ENUM, &suValue, lNumElements);
    }
    else
        iStatus = 
          dbPutField( &dbaddrVariable, iValueType, pBuffer, lNumElements);
          
    if ( iStatus != 0 )
    {
        printf("writePv(): dbPutField(%s) failed. Status  = %d\n", sVariableName, iStatus);
        return(iStatus);
    }
    
    return(0);
}

int BldPvClientBasic::printPv(const char *sVariableName, void* pBuffer, 
      short iValueType, long lNumElements )
{
    int iRealMsgSize = 0;
    char lcMsgBuffer2[iMTU] = {0};
    int iSprintFail = 
      sprintPv( sVariableName, pBuffer, sizeof(lcMsgBuffer2)-1, lcMsgBuffer2, &iRealMsgSize, 
      iValueType, lNumElements );
      
    if ( iSprintFail != 0 ) return iSprintFail;
     
    printf( "%s", lcMsgBuffer2 );
    return 0;
}

int BldPvClientBasic::sprintPv(const char *sVariableName, void* pBuffer, 
  int iMsgBufferSize, char* lcMsgBuffer2, int* piRealMsgSize, short iValueType, long lNumElements )
{
    if (sVariableName == NULL || *sVariableName == 0 || pBuffer == NULL || lcMsgBuffer2 == NULL )
    {
        printf( "sprintPv(): Invalid parameter\n" );
        return 1;
    }
    
    char*const pMsgBufferLimit = lcMsgBuffer2 + iMsgBufferSize;
    
    int iCharsPrinted = sprintf( lcMsgBuffer2, "%s ", sVariableName );
    char* pcMsgBufferFrom = lcMsgBuffer2 + iCharsPrinted;    
    long lElement = 0;
    
    for ( char *pcBufferFrom = (char*) pBuffer;
      lElement < lNumElements; lElement++ )
    {    
        if ( pcMsgBufferFrom >= pMsgBufferLimit )
        {
            printf( "sprintPv(): message buffer size (%d) is not large enough\n", iMsgBufferSize );
            return 2;
        }
        
        if (lNumElements > 1)
        {
            if ( lElement == 0 )
                sprintf( pcMsgBufferFrom, "Array[%ld] [0] ", lNumElements);
            else
                sprintf( pcMsgBufferFrom, ", [%ld] ", lElement );
        }
        pcMsgBufferFrom += strlen( pcMsgBufferFrom );
            
        switch (iValueType)
        {
        case DBR_STRING:
            sprintf( pcMsgBufferFrom, "(string) = %s", pcBufferFrom );
            pcBufferFrom += strlen( pcBufferFrom ) + 1;
            break;
        //case DBR_INT: // the same as DBR_SHORT, as defeind in <dbAccess.h>
        case DBR_SHORT:
            sprintf( pcMsgBufferFrom, "(short) = %d", *(short int*) pcBufferFrom );
            pcBufferFrom += sizeof(short);
            break;
        case DBR_FLOAT:
            sprintf( pcMsgBufferFrom, "(float) = %f", *(float*) pcBufferFrom );
            pcBufferFrom += sizeof(float);
            break;
        case DBR_ENUM:
            sprintf( pcMsgBufferFrom, "(enum) = %s", pcBufferFrom );
            pcBufferFrom += strlen( pcBufferFrom ) + 1;
            break;
        case DBR_CHAR:
            sprintf( pcMsgBufferFrom, "(char) = %c", *pcBufferFrom );
            pcBufferFrom += sizeof(char);
            break;
        case DBR_LONG:
            sprintf( pcMsgBufferFrom, "(long) = %ld", *(long int*) pcBufferFrom );
            pcBufferFrom += sizeof(long);
            break;
        default:
        case DBR_DOUBLE:
            sprintf( pcMsgBufferFrom, "(double) = %lf", *(double*) pcBufferFrom );
            pcBufferFrom += sizeof(double);
            break;      
        }
        
        pcMsgBufferFrom += strlen( pcMsgBufferFrom );        
    }
        
    *piRealMsgSize = pcMsgBufferFrom - lcMsgBuffer2;
    return 0;
}

} // namespace EpicsBld
