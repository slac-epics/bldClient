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
int BldStart()
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient().bldStart();
}

int BldStop()
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient().bldStop();    
}

bool BldIsStarted()
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient().IsStarted();
}

int BldConfig( const char* sAddr, unsigned short uPort, unsigned int uMaxDataSize,  
      const char* sInterfaceIp, const char* sBldPvTrigger, const char* sBldPvList )
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient().bldConfig(
      sAddr, uPort, uMaxDataSize, sInterfaceIp, sBldPvTrigger, sBldPvList);
}

void BldShowConfig()
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient().bldShowConfig();
}

int BldSetSub( const char* sBldSubRec )
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient().bldSetSub(sBldSubRec);
}

int BldSendData()
{
    return EpicsBld::BldPvClientFactory::getSingletonBldPvClient().bldSendData();
}

void BldSetDebugLevel(int iDebugLevel)
{
    EpicsBld::BldPvClientFactory::getSingletonBldPvClient().setDebugLevel(iDebugLevel != 0);
}

int BldGetDebugLevel()
{
    return 
      (EpicsBld::BldPvClientFactory::getSingletonBldPvClient().getDebugLevel() ?
      1 : 0 );
}
 
} // extern "C"

using std::string;

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
    virtual int bldConfig( const char* sAddr, unsigned short uPort, unsigned int uMaxDataSize,  
      const char* sInterfaceIp, const char* sBldPvTrigger, const char* sBldPvList );
    virtual void bldShowConfig();
    
    // To be called by the init function of subroutine record
    virtual int bldSetSub( const char* sBldSubRec ); 

    // To be called by trigger variables (subroutine records)      
    virtual int bldSendData(); 

    // debug information control
    virtual void setDebugLevel(int iDebugLevel);
    virtual int getDebugLevel();    
    
    static BldPvClientBasic& getSingletonObject(); // singelton interface
    
private:
    bool _bBldStarted;
    std::auto_ptr<EpicsBld::BldNetworkClientInterface> _apBldNetworkClient;
    int _iDebugLevel;
    
    string          _sBldPvSubRec;
    unsigned int    _uBldServerAddr;
    unsigned short  _uBldServerPort;
    unsigned int    _uMaxDataSize;
    string          _sBldInterfaceIp;    
    string          _sBldPvTrigger, _sBldPvList, _sBldPvTriggerPrevFLNK;
    
     BldPvClientBasic(); /// Singelton. No explicit instantiation
     ~BldPvClientBasic();
          
     ///  Disable value semantics. No definitions (function bodies).
     BldPvClientBasic(const BldPvClientBasic&); 
     BldPvClientBasic& operator=(const BldPvClientBasic&);

    /*
     * Utility varaibles and functions
     */
    static const int iMTU; // Ethernet packet MTU
    static const char sPvListSeparators[];

    int _splitPvList( const string& sBldPvList, std::vector<string>& vsBldPv );
    
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
BldPvClientInterface& BldPvClientFactory::getSingletonBldPvClient()
{
    return BldPvClientBasic::getSingletonObject();
}

/*
 * class BldPvClientBasic
 */

/*
 * private static variables
 */

const int BldPvClientBasic::iMTU = 9000; // Ethernet packet MTU
const char BldPvClientBasic::sPvListSeparators[] = " ,;\r\n";

/* static member functions */
 
BldPvClientBasic& BldPvClientBasic::getSingletonObject()
{
    static BldPvClientBasic bldDataClient;
    return bldDataClient;
}

/* public member functions */

BldPvClientBasic::BldPvClientBasic() : _bBldStarted(false), _iDebugLevel(0),
  _uBldServerAddr(0), _uBldServerPort(0), _uMaxDataSize(0)
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
     * setup forward link:  _sBldPvTrigger -> _sBldPvSubRec -> _sBldPvTriggerPrevFLNK
     */
    if ( !_sBldPvTrigger.empty() )
    {
        short int iFieldType = 0;
        long lNumElements = 0;
        long llBufPvVal[16] = {0}; // Align with long int boundaries
        char* lcBufPvVal = (char*) llBufPvVal;

        // Read PV: (_sBldPvTrigger).FLNK
        string sBldPvTriggerFieldFLNK = _sBldPvTrigger + ".FLNK";
        if ( 
          readPv( sBldPvTriggerFieldFLNK.c_str(), sizeof(llBufPvVal), llBufPvVal, &iFieldType, &lNumElements )
          != 0 )
            throw string("readPv(") + sBldPvTriggerFieldFLNK + ") Failed\n";
        _sBldPvTriggerPrevFLNK.assign(lcBufPvVal);
        
        // Set PV: (_sBldPvSubRec).FLNK = (_sBldPvTrigger).FLNK
        if ( _sBldPvTriggerPrevFLNK == "0" ) // special case: "0" means NO FLNK
            _sBldPvTriggerPrevFLNK.clear();
        else if ( !_sBldPvTriggerPrevFLNK.empty() )
        {
            string sBldPvSubRecFieldFLNK = _sBldPvSubRec + ".FLNK";
            if ( _iDebugLevel > 0 )
                printf( "Setting PV FLNK: %s = %s\n", sBldPvSubRecFieldFLNK.c_str(), 
                  _sBldPvTriggerPrevFLNK.c_str() );
            if ( 
              writePv( sBldPvSubRecFieldFLNK.c_str(), lcBufPvVal )
              != 0 )
                throw string("writePv(") + sBldPvSubRecFieldFLNK + ") Failed\n";    
        }

        // Set PV: (_sBldPvTrigger).FLNK = _sBldPvSubRec
        int iLenPvVal = _sBldPvSubRec.length();
        strncpy( lcBufPvVal, _sBldPvSubRec.c_str(), iLenPvVal );
        if ( _iDebugLevel > 0 )
            printf( "Setting PV FLNK: %s = %s\n", sBldPvTriggerFieldFLNK.c_str(), 
              _sBldPvSubRec.c_str() );
        if ( 
          writePv( sBldPvTriggerFieldFLNK.c_str(), lcBufPvVal )
          != 0 )
            throw string("writePv(") + sBldPvTriggerFieldFLNK + ") Failed\n";            
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
     * reset forward link:  _sBldPvTrigger -> _sBldPvTriggerPrevFLNK , _sBldPvSubRec ->  ""
     */
    if ( !_sBldPvTrigger.empty() )
    {
        long llBufPvVal[16] = {0}; // Align with long int boundaries
        char* lcBufPvVal = (char*) llBufPvVal;
        
        // Set PV: (_sBldPvTrigger).FLNK = ""
        int iLenPvVal = _sBldPvTriggerPrevFLNK.length();
        strncpy( lcBufPvVal, _sBldPvTriggerPrevFLNK.c_str(), iLenPvVal );
        string sBldPvTriggerFieldFLNK = _sBldPvTrigger + ".FLNK";
        if ( _iDebugLevel > 0 )
            printf( "Setting PV FLNK: %s = %s\n", sBldPvTriggerFieldFLNK.c_str(), 
              _sBldPvTriggerPrevFLNK.c_str() );
        if ( 
          writePv( sBldPvTriggerFieldFLNK.c_str(), lcBufPvVal )
          != 0 )
            throw string("writePv(") + sBldPvTriggerFieldFLNK + ") Failed\n";
            
        if ( !_sBldPvTriggerPrevFLNK.empty() )
        {
            lcBufPvVal[0] = 0; // set lcBufPvVal = ""
            string sBldPvSubRecFieldFLNK = _sBldPvSubRec + ".FLNK";
            if ( _iDebugLevel > 0 )
                printf( "Setting PV FLNK: %s = 0\n", sBldPvSubRecFieldFLNK.c_str() );            
            if ( 
              writePv( sBldPvSubRecFieldFLNK.c_str(), lcBufPvVal )
              != 0 )
                throw string("writePv(") + sBldPvSubRecFieldFLNK + ") Failed\n";                        
        }
    }    

    _sBldPvTriggerPrevFLNK.clear();
    
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

int BldPvClientBasic::bldSetSub( const char* sBldSubRec )
{
    _sBldPvSubRec.assign(sBldSubRec);
        
    printf( "Bld PV Settings:\n  Subroutine Record <%s>\n",
      _sBldPvSubRec.c_str() );
    
    return 0;
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

    if ( _iDebugLevel > 1 )
        printf( "Sending Bld to Addr %x Port %d Interface %s PvList %s\n",
          _uBldServerAddr, _uBldServerPort, _sBldInterfaceIp.c_str(), _sBldPvList.c_str() );
        
    short int iFieldType = 0;
    long lNumElements = 0;

    /* Read Bld PvList */
    
    std::vector<string> vsBldPv;
    _splitPvList( _sBldPvList, vsBldPv );

    static long llBufPvVal[iMTU / sizeof(long)] = {0}; // Align with long int boundaries
    static char lcMsgBuffer[iMTU] = {0};
    
    char* pcMsgBuffer = lcMsgBuffer;
    int iMaxMsgSize = sizeof(lcMsgBuffer)-1;
    unsigned int uDataSize = 0;
    
    for ( std::vector<string>::const_iterator itsBldPv = vsBldPv.begin();
      itsBldPv != vsBldPv.end(); itsBldPv++ )
    {
        const string& sBldPv = *itsBldPv;
        if ( _iDebugLevel > 2 )
            printf( "Reading PV %s...\n", sBldPv.c_str());
        if ( 
          readPv( sBldPv.c_str(), sizeof(llBufPvVal), llBufPvVal, &iFieldType, &lNumElements )
          != 0 )
            throw string("readPv(") + sBldPv + ") Failed\n";
        
        if ( _iDebugLevel > 2)
            printf( "Msg Buffer before PV %s AvaSize %d: %s\n", sBldPv.c_str(), iMaxMsgSize, lcMsgBuffer);
            
        int iNewMsgSize = 0;
        if (
          sprintPv( sBldPv.c_str(), llBufPvVal, iMaxMsgSize, pcMsgBuffer, 
          &iNewMsgSize, iFieldType, lNumElements )
          != 0 )
            throw string("sprintPv(") + sBldPv + ") Failed\n";
                
        pcMsgBuffer += iNewMsgSize;
        uDataSize += iNewMsgSize;
        iMaxMsgSize -= iNewMsgSize;
        
        if ( _iDebugLevel > 2 )
            printf( "Msg Buffer after PV %s AvaSize %d: %s\n", sBldPv.c_str(), iMaxMsgSize, lcMsgBuffer);        
    }
    
    if ( uDataSize > _uMaxDataSize )
        throw string("Data Size is larger than max value\n");
    
    int iFailSend = _apBldNetworkClient->sendRawData(strlen(lcMsgBuffer), lcMsgBuffer);
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

int BldPvClientBasic::bldConfig( const char* sAddr, unsigned short uPort, unsigned int uMaxDataSize,  
  const char* sInterfaceIp, const char* sBldPvTrigger, const char* sBldPvList )
{   
    if ( _bBldStarted )
    {
        printf( "BldPvClientBasic::bldConfig() : Need to stop bld before config\n" );
        return 1;
    }
            
    // Check for valid parameters
    // Note: sInterfaceIp == NULL is okay, which means default NIC is used
    if ( sAddr == NULL || uMaxDataSize <= 0 || sBldPvTrigger == NULL || sBldPvList == NULL ) 
    {
        printf( "BldPvClientBasic::bldConfig() : Input parameter invalid\n" );
        return 2;
    }

    printf( "Configuring bld:\n" );
    
    _uBldServerAddr = ntohl( inet_addr( sAddr ) );
    _uBldServerPort = uPort;    
    _uMaxDataSize = uMaxDataSize;
    _sBldInterfaceIp.assign(sInterfaceIp == NULL? "" : sInterfaceIp);  
    _sBldPvTrigger.assign(sBldPvTrigger);
    _sBldPvList.assign(sBldPvList);
                      
    bldShowConfig();
    
    return 0;
}

void BldPvClientBasic::bldShowConfig()
{
    printf( "  Configurable parameters:\n"
      "    Server Addr %x  Port %d  MaxDataSize %u\n"
      "    MCastIF %s  PvTrigger <%s>\n"
      "    PvList <%s>\n",
      _uBldServerAddr, _uBldServerPort, _uMaxDataSize, _sBldInterfaceIp.c_str(), 
      _sBldPvTrigger.c_str(), _sBldPvList.c_str() );
            
    printf( "  Internal Settings:\n"
      "    Subroutine Record <%s>  PvTrigger.FLNK <%s>\n"
      "    DebugLevel %d\n",
      _sBldPvSubRec.c_str(), _sBldPvTriggerPrevFLNK.c_str(),
      _iDebugLevel );      
}

/*
 * private static member functions
 */
int BldPvClientBasic::_splitPvList( const string& sBldPvList, std::vector<string>& vsBldPv )
{
    unsigned int uOffsetStart = sBldPvList.find_first_not_of( sPvListSeparators, 0 );
    while ( uOffsetStart != string::npos )      
    {        
        unsigned uOffsetEnd = sBldPvList.find_first_of( sPvListSeparators, uOffsetStart+1 );
        
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

int BldPvClientBasic::readPv(const char *sVariableName, int iBufferSize, void* pBuffer, short* piValueType, long* plNumElements )
{   
    if (sVariableName == NULL || *sVariableName == 0 || pBuffer == NULL || piValueType == NULL || plNumElements == NULL )
    {
        printf( "readPv(): Invalid parameter\n" );
        return 1;
    }
    
    if ( (unsigned int) pBuffer & 0x3 != 0  )
    {
        printf( "readPv(): Buffer should be aligned in 32 bits boundaries\n" );
        return 2;
    }
        
    DBADDR  dbaddrVariable;
    int iStatus = dbNameToAddr(sVariableName,&dbaddrVariable);  
    if ( iStatus != 0 )
    {
        printf("readPv(): dbNameToAddr(%s) failed. Status  = %d\n", sVariableName, iStatus);
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
        printf("readPv(): dbGetField(%s) failed. Status  = %d\n", sVariableName, iStatus);
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
    
    if ( (unsigned int) pBuffer & 0x3 != 0  )
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
    char lcMsgBuffer[iMTU] = {0};
    int iSprintFail = 
      sprintPv( sVariableName, pBuffer, sizeof(lcMsgBuffer)-1, lcMsgBuffer, &iRealMsgSize, 
      iValueType, lNumElements );
      
    if ( iSprintFail != 0 ) return iSprintFail;
     
    printf( "%s", lcMsgBuffer );
    return 0;
}

int BldPvClientBasic::sprintPv(const char *sVariableName, void* pBuffer, 
  int iMsgBufferSize, char* lcMsgBuffer, int* piRealMsgSize, short iValueType, long lNumElements )
{
    if (sVariableName == NULL || *sVariableName == 0 || pBuffer == NULL || lcMsgBuffer == NULL )
    {
        printf( "sprintPv(): Invalid parameter\n" );
        return 1;
    }
    
    char*const pMsgBufferLimit = lcMsgBuffer + iMsgBufferSize;
    
    int iCharsPrinted = sprintf( lcMsgBuffer, "%s = ", sVariableName );
    char* pcMsgBufferFrom = lcMsgBuffer + iCharsPrinted;    
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
                sprintf( pcMsgBufferFrom, "Array[%ld]: [0] ", lNumElements);
            else
                sprintf( pcMsgBufferFrom, ", [%ld] ", lElement );
        }
        pcMsgBufferFrom += strlen( pcMsgBufferFrom );
            
        switch (iValueType)
        {
        case DBR_STRING:
            sprintf( pcMsgBufferFrom, "(String) %s", pcBufferFrom );
            pcBufferFrom += strlen( pcBufferFrom ) + 1;
            break;
        //case DBR_INT: // the same as DBR_SHORT, as defeind in <dbAccess.h>
        case DBR_SHORT:
            sprintf( pcMsgBufferFrom, "(short) %d", *(short*) pcBufferFrom );
            pcBufferFrom += sizeof(short);
            break;
        case DBR_FLOAT:
            sprintf( pcMsgBufferFrom, "(float) %f", *(float*) pcBufferFrom );
            pcBufferFrom += sizeof(float);
            break;
        case DBR_ENUM:
            sprintf( pcMsgBufferFrom, "(enum) %s", pcBufferFrom );
            pcBufferFrom += strlen( pcBufferFrom ) + 1;
            break;
        case DBR_CHAR:
            sprintf( pcMsgBufferFrom, "(char) %c (%d)", *pcBufferFrom, *pcBufferFrom );
            pcBufferFrom += sizeof(char);
            break;
        case DBR_LONG:
            sprintf( pcMsgBufferFrom, "(long) %ld", *(long*) pcBufferFrom );
            pcBufferFrom += sizeof(long);
            break;
        default:
        case DBR_DOUBLE:
            sprintf( pcMsgBufferFrom, "(double) %lf", *(double*) pcBufferFrom );
            pcBufferFrom += sizeof(double);
            break;      
        }
        
        pcMsgBufferFrom += strlen( pcMsgBufferFrom );        
    }
        
    *piRealMsgSize = pcMsgBufferFrom - lcMsgBuffer;
    return 0;
}

} // namespace EpicsBld
