#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <string.h>
#include <time.h>

#include <dbDefs.h>
#include <registryFunction.h>
#include <subRecord.h>
#include <epicsExport.h>
#include <epicsTime.h>
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
int BldStart(int bldClientId)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(bldClientId).bldStart();
}

int BldStop(int bldClientId)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(bldClientId).bldStop();    
}

bool BldIsStarted(int bldClientId)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(bldClientId).IsStarted();
}

int BldSetControl(int bldClientId, int on)
{
    if (on) {
        if (!BldIsStarted(bldClientId))
            return BldStart(bldClientId);
    } else {
        if (BldIsStarted(bldClientId))
            return BldStop(bldClientId);
    }
    return 1;
}

int BldConfigSend(
	int				bldClientId,
	const char	*	sAddr,
	unsigned short	uPort,
	unsigned int	uMaxDataSize,
	const char	*	sInterfaceIp	)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(bldClientId).bldConfigSend(	sAddr, uPort, uMaxDataSize, sInterfaceIp );
}

int BldConfig(int bldClientId, const char* sAddr, unsigned short uPort, 
  unsigned int uMaxDataSize,	const char* sInterfaceIp,
  unsigned int uSrcPhysicalId,	unsigned int uxtcDataType,
  const char* sBldPvPreTrigger,	const char* sBldPvPostTrigger, 
  const char* sBldPvFiducial,	const char* sBldPvList )
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(bldClientId).bldConfig(
            sAddr, uPort, uMaxDataSize, sInterfaceIp, uSrcPhysicalId, uxtcDataType, sBldPvPreTrigger,
            sBldPvPostTrigger, sBldPvFiducial, sBldPvList);
}

void BldShowConfig(int bldClientId)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(bldClientId).bldShowConfig();
}

int BldSetPreSub(int bldClientId, const char* sBldSubRec)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(bldClientId).bldSetPreSub(sBldSubRec);
}

int BldSetPostSub(int bldClientId, const char* sBldSubRec)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(bldClientId).bldSetPostSub(sBldSubRec);
}

int BldPrepareData(int bldClientId)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(bldClientId).bldPrepareData();
}

int BldSendData(int bldClientId)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(bldClientId).bldSendData();
}

int BldSendPacket(	int					bldClientId,
					unsigned int		srcId,
					unsigned int		xtcType,
					epicsTimeStamp	*	pts,
					void			*	pPkt,
					size_t				sPkt	)
{
//	printf( "BldSendPacket: client %d, srcId %u, xtc %u, pPkt 0x%p, sPkt %zd\n", bldClientId, srcId, xtcType, pPkt, sPkt );
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(bldClientId).bldSendPacket( srcId, xtcType, pts, pPkt, sPkt );
}

void BldSetDebugLevel(int bldClientId, int iDebugLevel)
{
    EpicsBld::BldPvClientFactory::getSingletonBldPvClient(bldClientId).setDebugLevel(iDebugLevel);
}

int BldGetDebugLevel(int bldClientId)
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient(bldClientId).getDebugLevel();
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
    virtual int bldConfigSend(	const char	*	sAddr,	unsigned short	uPort,
								unsigned int	uMaxDataSize, const char* sInterfaceIp );
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
    virtual int bldSendPacket(
			unsigned int		srcPhysicalId,
			unsigned int		xtcDataType,
			epicsTimeStamp	*	pTsFiducial,
			void			*	pPacket,
			size_t				sPacket	); 

    // debug information control
    virtual void setDebugLevel(int iDebugLevel);
    virtual int getDebugLevel();

	const char * GetInterfaceIp( ) const
	{
		if ( _sBldInterfaceIp.size() > 0 )
			return _sBldInterfaceIp.c_str();
		return "--default--";
	}

    static BldPvClientBasic& getSingletonObject(int bldClientId); // singelton interface
    
private:
    bool _bBldStarted;
    std::auto_ptr<EpicsBld::BldNetworkClientInterface> _apBldNetworkClient;
    int _iDebugLevel;
    
    string          _sBldPvPreSubRec, _sBldPvPostSubRec;
    unsigned int    _uBldServerAddr;
    unsigned short  _uBldServerPort;
    unsigned int    _uMaxDataSize;
    string          _sBldInterfaceIp;    
    unsigned int    _uSrcPhysicalId, _uxtcDataType;
    string          _sBldPvPreTrigger, _sBldPvPostTrigger, _sBldPvFiducial;
    string          _sBldPvList, _sBldPvPreTriggerPrevFLNK, _sBldPvPostTriggerPrevFLNK;
    unsigned int    _uFiducialIdPrev;
    unsigned int    _uFiducialIdCur;
    epicsTimeStamp  _uFiducialTime;
    
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
      short* piValueType, long* plNumElements, epicsTimeStamp *ts );
    static int writePv(const char * sVariableName, const char * pBuffer ); 
    static int printPv(const char *sVariableName, void* pBuffer, 
      short iValueType = DBR_STRING, long lNumElements = 1 );
    static int sprintPv(const char *sVariableName, void* pBuffer, 
      int iMsgBufferSize, char* lcMsgBuffer2, int* piRealMsgSize, 
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
BldPvClientInterface& BldPvClientFactory::getSingletonBldPvClient(int bldClientId)
{
    return BldPvClientBasic::getSingletonObject(bldClientId);
}

/*
 * class BldPvClientBasic
 */

/*
 * private static variables
 */

const char BldPvClientBasic::sPvListSeparators[] = " ,;\r\n";

/* static member functions */
 
BldPvClientBasic& BldPvClientBasic::getSingletonObject(int bldClientId)
{
    static BldPvClientBasic bldDataClient[10]; // Ick!
	assert( bldClientId < 10 && "Make sure your first arg to Bld* shell commands is 0 or bldClientId" );
    return bldDataClient[bldClientId];
}

/* public member functions */

BldPvClientBasic::BldPvClientBasic() : _bBldStarted(false), _iDebugLevel(0),
  _uBldServerAddr(0), _uBldServerPort(0), _uMaxDataSize(0), _uSrcPhysicalId(0), _uxtcDataType(0),
  _uFiducialIdPrev(FIDUCIAL_NOT_SET), _uFiducialIdCur(FIDUCIAL_NOT_SET)

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

    printf( "Starting bld: MaxDataSize %u\n", _uMaxDataSize );

	try
	{
		const unsigned char ucTTL = 32; /// minimum: 1 + (# of routers in the middle)
				
		_apBldNetworkClient.reset(
		  EpicsBld::BldNetworkClientFactory::createBldNetworkClient( _uBldServerAddr, _uBldServerPort, 
		  _uMaxDataSize + sizeof(BldPacketHeader), ucTTL, _sBldInterfaceIp.c_str() ) );

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
			llBufPvVal[0] = 0;

			// Read PV: (_sBldPvPreTrigger).FLNK
			// If it has ".XXX" at the end, no need to append ".FLNK"!
			string tsBldPvPreTriggerFLNK = 
				(_sBldPvPreTrigger.find('.') == string::npos) ? _sBldPvPreTrigger + ".FLNK"
															  : _sBldPvPreTrigger;
			if ( readPv( tsBldPvPreTriggerFLNK.c_str(), sizeof(llBufPvVal), llBufPvVal,
						&iFieldType, &lNumElements, NULL ) != 0 )
				throw string("readPv(") + tsBldPvPreTriggerFLNK + ") Failed\n";
			char* lcBufPvVal = (char*) llBufPvVal;
			_sBldPvPreTriggerPrevFLNK.assign(lcBufPvVal);
			
			if ( !_sBldPvPreSubRec.empty() )
			{
				// Set PV: (_sBldPvPreSubRec).FLNK = (_sBldPvPreTrigger).FLNK
				if ( _sBldPvPreTriggerPrevFLNK == "0" ) // special case: "0" means NO FLNK
					_sBldPvPreTriggerPrevFLNK.clear();
				else if ( !_sBldPvPreTriggerPrevFLNK.empty() )
				{
					string tsBldPreSubFLNK = _sBldPvPreSubRec + ".FLNK";
					if ( _iDebugLevel >= 1 )
						printf( "Setting BldPreSub.FLNK:  %s = %s\n",
									tsBldPreSubFLNK.c_str(), 
									_sBldPvPreTriggerPrevFLNK.c_str() );
					if ( writePv(	tsBldPreSubFLNK.c_str(),
									_sBldPvPreTriggerPrevFLNK.c_str() ) != 0 )
						throw string("writePv(") + tsBldPreSubFLNK + ") Failed\n";    
				}

				// Set PV: (_sBldPvPreTrigger).FLNK = _sBldPvPreSubRec
				if ( _iDebugLevel >= 1 )
					printf(		"Setting PreTrigger.FLNK: %s = %s\n",
								 tsBldPvPreTriggerFLNK.c_str(), 
								_sBldPvPreSubRec.c_str() );
				if ( writePv(	tsBldPvPreTriggerFLNK.c_str(),
								_sBldPvPreSubRec.c_str() ) != 0 )
					throw string("writePv(") + tsBldPvPreTriggerFLNK + ") Failed\n";            
			}
		}
			

		/*
		* setup forward link:  _sBldPvPostTrigger -> _sBldPvPostSubRec -> _sBldPvPostTriggerPostvFLNK
		*/
		if ( !_sBldPvPostTrigger.empty() )
		{
			short int iFieldType = 0;
			long lNumElements = 0;
			llBufPvVal[0] = 0;

			// Read PV: (_sBldPvPostTrigger).FLNK
			string tsPvPostTriggerFLNK = _sBldPvPostTrigger + ".FLNK";
			if ( readPv( tsPvPostTriggerFLNK.c_str(), sizeof(llBufPvVal), llBufPvVal, &iFieldType, &lNumElements, NULL ) != 0 )
				throw string("readPv(") + tsPvPostTriggerFLNK + ") Failed\n";
			char* lcBufPvVal = (char*) llBufPvVal;
			_sBldPvPostTriggerPrevFLNK.assign(lcBufPvVal);

			if ( !_sBldPvPostSubRec.empty() )
			{
				// Set PV: (_sBldPvPostSubRec).FLNK = (_sBldPvPostTrigger).FLNK
				if ( _sBldPvPostTriggerPrevFLNK == "0" ) // special case: "0" means NO FLNK
					_sBldPvPostTriggerPrevFLNK.clear();
				else if ( !_sBldPvPostTriggerPrevFLNK.empty() )
				{
					string tsBldPostSubFLNK = _sBldPvPostSubRec + ".FLNK";
					if ( _iDebugLevel >= 1 )
						printf( "Setting PostTriggerFLNK: %s = %s\n",
								 tsBldPostSubFLNK.c_str(), 
								_sBldPvPostTriggerPrevFLNK.c_str() );
					if ( writePv(	tsBldPostSubFLNK.c_str(),
									_sBldPvPostTriggerPrevFLNK.c_str() ) != 0 )
						throw string("writePv(") + tsBldPostSubFLNK + ") Failed\n";    
				}

				// Set PV: (_sBldPvPostTrigger).FLNK = _sBldPvPostSubRec
				if ( _iDebugLevel >= 1 )
					printf(		"Setting PostTriggerFLNK: %s = %s\n",
								tsPvPostTriggerFLNK.c_str(), 
								_sBldPvPostSubRec.c_str() );
				if ( writePv(	 tsPvPostTriggerFLNK.c_str(),
								_sBldPvPostSubRec.c_str() ) != 0 )
					throw string("writePv(") + tsPvPostTriggerFLNK + ") Failed\n";            
			}
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
			llBufPvVal[0] = 0;
			
			// Set PV: (_sBldPvPreTrigger).FLNK = ""
			// If it has ".XXX" at the end, no need to append ".FLNK"!
			string tsBldPvPreTriggerFLNK = 
				(_sBldPvPreTrigger.find('.') == string::npos) ? _sBldPvPreTrigger + ".FLNK"
															  : _sBldPvPreTrigger;
			if ( _iDebugLevel >= 1 )
				printf( 	"Setting PreTrigger.FLNK: %s = %s\n",
							tsBldPvPreTriggerFLNK.c_str(), 
							_sBldPvPreTriggerPrevFLNK.c_str() );
			if ( writePv(	tsBldPvPreTriggerFLNK.c_str(),
							_sBldPvPreTriggerPrevFLNK.c_str() ) != 0 )
				throw string("writePv(") + tsBldPvPreTriggerFLNK + ") Failed\n";
				
			if ( !_sBldPvPreTriggerPrevFLNK.empty() )
			{
				string tsBldPreSubFLNK = _sBldPvPreSubRec + ".FLNK";
				if ( _iDebugLevel >= 1 )
					printf( "Setting BldPreSub.FLNK:  %s = 0\n", tsBldPreSubFLNK.c_str() );
				char* lcBufPvVal = (char*) llBufPvVal;
				lcBufPvVal[0] = 0; // set lcBufPvVal = ""
				if ( writePv( tsBldPreSubFLNK.c_str(), lcBufPvVal ) != 0 )
					throw string("writePv(") + tsBldPreSubFLNK + ") Failed\n";
			}
		}    

		_sBldPvPreTriggerPrevFLNK.clear();
		
		/*
		* reset forward link:  _sBldPvPostTrigger -> _sBldPvPostTriggerPrevFLNK , _sBldPvPostSubRec ->  ""
		*/
		if ( !_sBldPvPostTrigger.empty() )
		{
			llBufPvVal[0] = 0;
			
			// Set PV: (_sBldPvPostTrigger).FLNK = ""
			string tsPvPostTriggerFLNK = _sBldPvPostTrigger + ".FLNK";
			if ( _iDebugLevel >= 1 )
				printf(		"Setting PostTriggerFLNK: %s = %s\n",
							tsPvPostTriggerFLNK.c_str(), 
							_sBldPvPostTriggerPrevFLNK.c_str() );
			if ( writePv(	tsPvPostTriggerFLNK.c_str(),
							_sBldPvPostTriggerPrevFLNK.c_str() ) != 0 )
				throw string("writePv(") + tsPvPostTriggerFLNK + ") Failed\n";
				
			if ( !_sBldPvPostTriggerPrevFLNK.empty() )
			{
				string tsBldPostSubFLNK = _sBldPvPostSubRec + ".FLNK";
				if ( _iDebugLevel >= 1 )
					printf( "Setting BldPostSubFLNK:  %s = 0\n", tsBldPostSubFLNK.c_str() );            
				char* lcBufPvVal = (char*) llBufPvVal;
				lcBufPvVal[0] = 0; // set lcBufPvVal = ""
				if ( writePv( tsBldPostSubFLNK.c_str(), lcBufPvVal ) != 0 )
					throw string("writePv(") + tsBldPostSubFLNK + ") Failed\n";                        
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
				readPv( _sBldPvFiducial.c_str(), sizeof(llBufPvVal), llBufPvVal, &iFieldType, &lNumElements, &_uFiducialTime )
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
		if ( _iDebugLevel >= 2 )
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
#if 0
		int iStatus = clock_gettime (CLOCK_REALTIME, &ts);
		if (iStatus)
			throw string( "clock_gettime() Failed\n" );
#else
                /* New regime - Use the fiducial timestamp! */
                ts.tv_sec  = _uFiducialTime.secPastEpoch + POSIX_TIME_AT_EPICS_EPOCH;
                ts.tv_nsec = _uFiducialTime.nsec;
#endif
		// Get the current fiducial id and check to
		// see if it's been set by the bldPreTrigger.
		unsigned int uFiducialId = _uFiducialIdCur;
		_uFiducialIdCur = FIDUCIAL_NOT_SET;
		if ( uFiducialId >= FIDUCIAL_INVALID )
		{
			static unsigned long	msgCount	= 0;

			if ( !(msgCount++ % 10000) || ( _iDebugLevel >= 2 ) )
			{
				if ( uFiducialId == FIDUCIAL_NOT_SET )
					throw string( "Fiducial not set.  Did your bldPreTrigger PV process?\n" );
				else
					throw string( "Invalid Fiducial 0x1FFFF\n" );
			}
			return 2;
		}
        if ( _iDebugLevel < 0 )
			printf( "bldSendData: Cur Fiducial Id 0x%05X, Prev Fiducial Id 0x%05X\n", uFiducialId, _uFiducialIdPrev ); 

        if (_uFiducialIdPrev == uFiducialId) {
                        time_t t;
                        struct tm *tmp;
                        t = time(NULL);
                        tmp = localtime(&t);
                        printf("[%02d/%02d/%04d %02d:%02d:%02d] Fiducial: %d (0x%x)\n",
                               tmp->tm_mon, tmp->tm_mday, 1900 + tmp->tm_year,
                               tmp->tm_hour, tmp->tm_min, tmp->tm_sec,
                               uFiducialId, uFiducialId);
			throw string("Duplicate Fiducial in BLD!\n");
			return 2;
        }
        _uFiducialIdPrev = uFiducialId;
	 
		// Create a BldPacketHeader
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
			const string& sBldPv = *itsBldPv;
			if ( _iDebugLevel >= 3 )
				printf( "Reading PV %s...\n", sBldPv.c_str());
			if ( 
			  readPv( sBldPv.c_str(), sizeof(llBufPvVal), llBufPvVal, &iFieldType, &lNumElements, NULL )
			  != 0 )
				throw string("readPv(") + sBldPv + ") Failed\n";
			
			//if ( _iDebugLevel >= 3)
			//    printf( "Msg Buffer before PV %s Available Size %d: %s\n", sBldPv.c_str(), iMaxMsgSize - uDataSize, lcMsgBuffer);
			
			int iFail = pBldPacketHeader->setPvValue( iPvIndex, llBufPvVal );
			if ( iFail != 0 )
				throw string("pBldPacketHeader->setPvValue() for PV ") + sBldPv + ") Failed\n";
		}

		//if ( uDataSize > _uMaxDataSize )
		//    throw string("Data Size is larger than max value\n");
					
		/* Send out bld */    
		int iFailSend = _apBldNetworkClient->sendRawData( pBldPacketHeader->getPacketSize(), lcMsgBuffer);
		if ( iFailSend != 0 )
			throw string( "_apBldNetworkClient->sendRawData() Failed\n", _sBldPvList.c_str() );

		if ( _iDebugLevel >= 2 )
		{
			printf( "Sent Bld to Addr %x Port %d Interface %s Fiducial 0x%05X\n",
			  _uBldServerAddr, _uBldServerPort, _sBldInterfaceIp.c_str(), uFiducialId ); 
			if ( _iDebugLevel >= 3 )
				printf( "PvList %s\n", _sBldPvList.c_str() );
		}
	}
	catch (string& sError)
	{
		printf( "BldPvClientBasic::bldSendData() : %s\n", sError.c_str() );
		  
		iRetErrorCode = 2;
	}
        
    return iRetErrorCode;
}

// Use this form when caller has already packed the data into
// a buffer and has a timestamp w/ a valid fiducial
int BldPvClientBasic::bldSendPacket(
	unsigned int		srcPhysicalId,
	unsigned int		xtcDataType,
	epicsTimeStamp	*	pTsFiducial,
	void			*	pPacket,
	size_t				sPacket	)
{
    if ( !_bBldStarted )
        return 1; // return status, without error report

    int iRetErrorCode = 0;

	try
	{
		if ( _apBldNetworkClient.get() == NULL )
			throw string( "BldNetworkClient is uninitialized\n" );

		/* Set bld packet header */
		struct timespec		ts;
		unsigned int		uFiducialId;
		/* New regime - Use the fiducial timestamp! */
		ts.tv_sec	= pTsFiducial->secPastEpoch + POSIX_TIME_AT_EPICS_EPOCH;
		ts.tv_nsec	= pTsFiducial->nsec;
		uFiducialId	= pTsFiducial->nsec & FIDUCIAL_MASK;
		if ( uFiducialId >= FIDUCIAL_INVALID )
			throw string( "Invalid Fiducial 0x1FFFF\n" );
		if (_uFiducialIdPrev == uFiducialId)
			throw string("Duplicate Fiducial in bldPacket!\n");
        _uFiducialIdPrev = uFiducialId;

		if ( sPacket > _uMaxDataSize )
		    throw string("Packet Size is larger than max value\n");

		// Create a BldPacketHeader in our network msg buffer
		BldPacketHeader		*	pBldPacketHeader = (BldPacketHeader*) lcMsgBuffer;
		new ( pBldPacketHeader ) BldPacketHeader( );
		pBldPacketHeader->Setup(	sPacket, ts.tv_sec, ts.tv_nsec,
									uFiducialId, srcPhysicalId, xtcDataType	);

		// Get ptr and size for data buffer
		// The lcMsgBuffer starts w/ a BldPacketHeader object,
		// which is then followed by the data buffer which must
		// fit into the one jumbo MTU sized buffer.
		void		*	pHeaderData	= (void *)(pBldPacketHeader + 1);
		size_t			sMsgBufferLeft	= sizeof(lcMsgBuffer) - sizeof(BldPacketHeader);
		if ( sPacket > sMsgBufferLeft )
		    throw string( "bldSendPacket: Error, Packet too large for buffer!\n");

		// Load the packet into the header
		memcpy( pHeaderData, pPacket, sPacket );
		assert( ((char *)pHeaderData - lcMsgBuffer) == sizeof(BldPacketHeader) );

		/* Send out bld */
		int iFailSend = _apBldNetworkClient->sendRawData( sizeof(BldPacketHeader) + sPacket, lcMsgBuffer);
		if ( iFailSend != 0 )
			throw string( "bldSendPacket: _apBldNetworkClient->sendRawData() Failed\n" );

		if ( _iDebugLevel >= 2 )
		{
			printf( "Sent Bld to Addr %x Port %d Interface %s sPkt %zu Fiducial 0x%05X\n",
			  _uBldServerAddr, _uBldServerPort, GetInterfaceIp(), sPacket, uFiducialId );
		}
	}
	catch (string& sError)
	{
		printf( "BldPvClientBasic::bldSendPacket() : %s\n", sError.c_str() );

		iRetErrorCode = 2;
	}

    return iRetErrorCode;
}

bool BldPvClientBasic::IsStarted() const
{
    return _bBldStarted;
}

int BldPvClientBasic::bldConfigSend(
	const char		*	sAddr,
	unsigned short		uPort, 
	unsigned int		uMaxDataSize,
	const char		*	sInterfaceIp	)
{
    BldPacketHeader::Initialize();

    if ( _bBldStarted )
    {
        printf( "BldPvClientBasic::bldConfigSend() : Need to stop bld before config\n" );
        return 1;
    }
            
    // Check for valid parameters
    // Note: sInterfaceIp == NULL is okay, which means default NIC is used
    if ( sAddr == NULL || uMaxDataSize <= 0 ) 
    {
        printf( "BldPvClientBasic::bldConfigSend() : Input parameter invalid\n" );
        return 2;
    }

    printf( "Configuring bld:\n" );
    
    _uBldServerAddr = ntohl( inet_addr( sAddr ) );
    _uBldServerPort = uPort;
    _uMaxDataSize = uMaxDataSize;
    _sBldInterfaceIp.assign(sInterfaceIp == NULL? "" : sInterfaceIp);  

    bldShowConfig();
 
    return 0;
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
    printf(	"  Configurable parameters:\n"
			"    Server Addr %u.%u.%u.%u  Port %d  MaxDataSize %u\n"
			"    MulticastIF %s\n",
			pcAddr[0], pcAddr[1], pcAddr[2], pcAddr[3],
			_uBldServerPort, _uMaxDataSize, GetInterfaceIp()	);
    if ( _uSrcPhysicalId || _uxtcDataType )
		printf(	"    Source Id %d Data Version %d Data Type %d (0x%X)\n",
				_uSrcPhysicalId, (_uxtcDataType>>16), (_uxtcDataType&0xFFFF), _uxtcDataType );

	if ( _sBldPvPreTrigger.size() == 0 && _sBldPvList.size() == 0 )
	{
		// Simple BldConfigSend
		printf( "BLD Configured w/o PV dependencies.\n"
				"Use BldSendPacket() from driver code to send packets\n" );
	}
	else
	{
		// Full BldConfig w/ PreTrigger, PostTrigger, and PV list
		printf(
		  "    PvPreTrigger <%s>\n"
		  "    PvPostTrigger <%s>\n"
		  "    PvFiducial <%s>\n"
		  "    PvList <%s>\n",
		  _sBldPvPreTrigger.c_str(), _sBldPvPostTrigger.c_str(),
		  _sBldPvFiducial.c_str(),   _sBldPvList.c_str() );

		printf( "  Internal Settings:\n"
		  "    Pre  Subroutine Record <%s>  PvPreTrigger.FLNK <%s>\n"
		  "    Post Subroutine Record <%s>  PvPostTrigger.FLNK <%s>\n",
		  _sBldPvPreSubRec.c_str(), _sBldPvPreTriggerPrevFLNK.c_str(),
		  _sBldPvPostSubRec.c_str(), _sBldPvPostTriggerPrevFLNK.c_str() );      
		if ( _sBldPvPreTrigger.size() != 0 && _sBldPvPreTrigger == _sBldPvPreSubRec )
		{
			printf( "WARNING: PvPreTrigger is same PV as %s!\n", _sBldPvPreSubRec.c_str() );
			printf( "Normally, PvPreTrigger should be the name of a local PV\n"
					"that processes when the fiducial is ready to fetch.\n" );
		}
		if ( _sBldPvPostTrigger.size() != 0 && _sBldPvPostTrigger == _sBldPvPostSubRec )
		{
			printf( "WARNING: PvPostTrigger is same PV as %s!\n", _sBldPvPostSubRec.c_str() );
			printf( "Normally, PvPostTrigger should be the name of a local PV\n"
					"that you want to process after the BLD data has been sent.\n" );
		}
	}
	printf( "    DebugLevel %d\n", _iDebugLevel );      
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
	int			iBufferSize,
	void		*	pBuffer,
	short		*	piValueType,
	long		*	plNumElements,
        epicsTimeStamp  *       ts	)
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
 
	// TODO: No need to use dbNameToAddr, as that restricts us to local PV's only
	// Update to support ca based PV's via dbGetLink and dbGetTimeStamp
	// NOTE: If we do support CA, we need to be careful about code that uses
	// readPv() and writePv() to manipulate FLNK or for BLD timing.
    DBADDR  dbaddrVariable;
    int iStatus = dbNameToAddr(sVariableName,&dbaddrVariable);  
    if ( iStatus != 0 )
    {
        printf("readPv(): dbNameToAddr(%s) failed. Status  = 0x%X\n", sVariableName, iStatus);
        return(iStatus);
    }

    if (ts)
        *ts = dbaddrVariable.precord->time;

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

int BldPvClientBasic::writePv(const char *sVariableName, const char * pBuffer )
{
    if (sVariableName == NULL || *sVariableName == 0 || pBuffer == NULL )
    {
        printf( "writePv(): Invalid parameter\n" );
        return 1;
    }

	int iStatus = dbpf( sVariableName, pBuffer );
          
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
