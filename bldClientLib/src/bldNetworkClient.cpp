#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/uio.h>
#include <net/if.h>
#include <string>
#include <sstream>

#include "bldNetworkClient.h"

/*
 * Global C function definitions
 */
extern "C" 
{   

/* 
 * The following functions provide C wrappers for accesing EpicsBld::BldNetworkClientInterface
 * and EpicsBld::BldNetworkClientFactory
 */

/**
 * Init function: Use EpicsBld::BldNetworkClientFactory to construct the BldNetworkClient
 * and save the pointer in (*ppVoidBldNetworkClient)
 */ 
int BldNetworkClientInitByInterfaceName(unsigned int uAddr, unsigned short uPort, 
    unsigned int uMaxDataSize, unsigned char ucTTL, const char* sInterfaceIp, 
    void** ppVoidBldNetworkClient)
{
    if ( ppVoidBldNetworkClient == NULL )
        return 1;
    
    EpicsBld::BldNetworkClientInterface* pBldNetworkClient = 
      EpicsBld::BldNetworkClientFactory::createBldNetworkClient(uAddr, uPort, 
      uMaxDataSize, ucTTL, sInterfaceIp);
        
    *ppVoidBldNetworkClient = reinterpret_cast<void*>(pBldNetworkClient);
    
    return 0;
}

int BldNetworkClientInitByInterfaceAddress(unsigned int uAddr, unsigned short uPort, 
    unsigned int uMaxDataSize, unsigned char ucTTL, unsigned int uInterfaceIp, 
    void** ppVoidBldNetworkClient)
{
    if ( ppVoidBldNetworkClient == NULL )
        return 1;
    
    EpicsBld::BldNetworkClientInterface* pBldNetworkClient = 
      EpicsBld::BldNetworkClientFactory::createBldNetworkClient(uAddr, uPort, 
      uMaxDataSize, ucTTL, uInterfaceIp);
        
    *ppVoidBldNetworkClient = reinterpret_cast<void*>(pBldNetworkClient);
    
    return 0;
}

/**
 * Release function: Call C++ delete operator to delete the BldNetworkClient
 */
int BldNetworkClientRelease(void* pVoidBldNetworkClient)
{
    if ( pVoidBldNetworkClient == NULL )
        return 1;
        
    EpicsBld::BldNetworkClientInterface* pBldNetworkClient = 
      reinterpret_cast<EpicsBld::BldNetworkClientInterface*>(pVoidBldNetworkClient);
    delete pBldNetworkClient;
        
    return 0;
}

/**
 * Call the Send function defined in EpicsBld::BldNetworkClientInterface 
 */
int BldNetworkClientSendRawData(void* pVoidBldNetworkClient, int iSizeData, char* pData)
{
    if ( pVoidBldNetworkClient == NULL || pData == NULL )
        return -1;

    EpicsBld::BldNetworkClientInterface* pBldNetworkClient = 
      reinterpret_cast<EpicsBld::BldNetworkClientInterface*>(pVoidBldNetworkClient);      

    return pBldNetworkClient->sendRawData(iSizeData, pData);
}



} // extern "C" 

using std::string;

/*
 * local class declarations
 */
namespace EpicsBld
{

/**
 * A Slim Bld Multicast Client class 
 *
 * Combination of BldNetworkClientBasic, Client, Port and Ins
 */
class BldNetworkClientSlim : public BldNetworkClientInterface
{
public:
    BldNetworkClientSlim(unsigned int uAddr, unsigned short uPort, unsigned int uMaxDataSize,
      unsigned char ucTTL = 32, const char* sInteraceIp = NULL);
    BldNetworkClientSlim(unsigned int uAddr, unsigned short uPort, unsigned int uMaxDataSize,
      unsigned char ucTTL = 32, unsigned int uInteraceIp = 0);
    virtual ~BldNetworkClientSlim();
    virtual int setPort(unsigned short uPort);
    virtual int setAddr(unsigned int uAddr);
    virtual int sendRawData(int iSizeData, const char* pData);
    
    // debug information control
    virtual void setDebugLevel(int iDebugLevel);
    virtual int getDebugLevel();
    
private:
    unsigned int _uAddr;
    unsigned short _uPort;
    int _iSocket;
    int _iDebugLevel;
    
    int _init( unsigned int uMaxDataSize, unsigned char ucTTL, 
      unsigned int uInterfaceIp);   
      
    static std::string addressToStr( unsigned int uAddr );      
};

} // namespace EpicsBld

/**
 * class member definitions
 */
namespace EpicsBld
{
/**
 * class BldNetworkClientFactory
 */
BldNetworkClientInterface* BldNetworkClientFactory::createBldNetworkClient(unsigned int uAddr, 
  unsigned short uPort, unsigned int uMaxDataSize, unsigned char ucTTL, 
  const char* sInteraceIp)
{
    return new BldNetworkClientSlim(uAddr, uPort, uMaxDataSize, ucTTL, sInteraceIp );
}

BldNetworkClientInterface* BldNetworkClientFactory::createBldNetworkClient(unsigned int uAddr, 
  unsigned short uPort, unsigned int uMaxDataSize, unsigned char ucTTL, 
  unsigned int uInterfaceIp)
{
    return new BldNetworkClientSlim(uAddr, uPort, uMaxDataSize, ucTTL, uInterfaceIp );
}

/**
 * class BldNetworkClientSlim
 */
BldNetworkClientSlim::BldNetworkClientSlim(unsigned int uAddr, unsigned short uPort, 
  unsigned int uMaxDataSize, unsigned char ucTTL, const char* sInterfaceIp) : 
  _uAddr(uAddr), _uPort(uPort), _iSocket(-1), _iDebugLevel(0)
{
    unsigned int uInterfaceIp = ( 
      (sInterfaceIp == NULL || sInterfaceIp[0] == 0)?
      0 : ntohl(inet_addr(sInterfaceIp)) );
    
    _init(uMaxDataSize, ucTTL, uInterfaceIp);   
}

BldNetworkClientSlim::BldNetworkClientSlim(unsigned int uAddr, unsigned short uPort, 
  unsigned int uMaxDataSize, unsigned char ucTTL, unsigned int uInterfaceIp) : 
  _uAddr(uAddr), _uPort(uPort), _iSocket(-1), _iDebugLevel(0)
{   
    _init(uMaxDataSize, ucTTL, uInterfaceIp);
}

int BldNetworkClientSlim::_init( unsigned int uMaxDataSize, unsigned char ucTTL, 
  unsigned int uInterfaceIp)
{
    int iRetErrorCode = 0;
try
{
    /*
     * socket
     */
    _iSocket    = socket(AF_INET, SOCK_DGRAM, 0);   
    if ( _iSocket == -1 ) throw string("BldNetworkClientSlim::BldNetworkClientSlim() : socket() failed");

    /*
     * set sender buffer size
     */
    int iSendBufferSize = uMaxDataSize + sizeof(struct sockaddr_in);

#ifdef VXWORKS // Out for Tornado II 7/21/00 - RiC
    // The following was found exprimentally with ~claus/bbr/test/sizeTest
    // The rule may well change if the mBlk, clBlk or cluster parameters change
    // as defined in usrNetwork.c - RiC
    iSendBufferSize += (88 + 32 * ((parm - 1993) / 2048));
#endif

    if(
      setsockopt(_iSocket, SOL_SOCKET, SO_SNDBUF, (char*)&iSendBufferSize, sizeof(iSendBufferSize))
      == -1 )
        throw string("BldNetworkClientSlim::BldNetworkClientSlim() : setsockopt(...SO_SNDBUF) failed");
    
    /*
     * socket and bind
     */
        
    sockaddr_in sockaddrSrc;
    sockaddrSrc.sin_family      = AF_INET;
    sockaddrSrc.sin_addr.s_addr = INADDR_ANY;
    sockaddrSrc.sin_port        = 0;

    if ( 
      bind( _iSocket, (struct sockaddr*) &sockaddrSrc, sizeof(sockaddrSrc) ) 
      == -1 )
        throw string("BldNetworkClientSlim::BldNetworkClientSlim() : bind() failed");
        
    /*
     * getsockname
     */     
    sockaddr_in sockaddrName;
#ifdef __linux__
    unsigned int iLength = sizeof(sockaddrName);
#elif defined(__rtems__)
    socklen_t iLength = sizeof(sockaddrName);
#else
    int iLength = sizeof(sockaddrName);
#endif      

    if(getsockname(_iSocket, (sockaddr*)&sockaddrName, &iLength) == 0) 
    {
        unsigned int uSockAddr = ntohl(sockaddrName.sin_addr.s_addr);
        unsigned int uSockPort = (unsigned int )ntohs(sockaddrName.sin_port);
        
        if ( _iDebugLevel > 1 )
            printf( "Local addr: %s Port %u\n", addressToStr(uSockAddr).c_str(), uSockPort );
    }
    else
        throw string("BldServerSlim::BldServerSlim() : getsockname() failed");      
        
    /*
     * set multicast TTL and interface
     */     
    if ( 
      setsockopt( _iSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ucTTL, sizeof(ucTTL) ) 
      < 0 ) 
        throw string("BldNetworkClientSlim::BldNetworkClientSlim() : setsockopt(...IP_MULTICAST_TTL) failed");

    if (uInterfaceIp != 0) 
    {           
        in_addr address;
        address.s_addr = htonl(uInterfaceIp);       
        
        if ( 
          setsockopt( _iSocket, IPPROTO_IP, IP_MULTICAST_IF, (char*)&address, 
          sizeof(address) ) 
          < 0) 
            throw string("BldNetworkClientSlim::BldNetworkClientSlim() : setsockopt(...IP_MULTICAST_IF) failed");         
    }       

}
catch (string& sError)
{
    printf( "[Error] %s, errno = %d (%s)\n", sError.c_str(), errno,
      strerror(errno) );
      
    iRetErrorCode = 1;
}
    
    return iRetErrorCode;   
}

BldNetworkClientSlim::~BldNetworkClientSlim()
{
    if (_iSocket != 0)
        close(_iSocket);
}

// debug information control
void BldNetworkClientSlim::setDebugLevel(int iDebugLevel)
{
    _iDebugLevel = iDebugLevel;
}

int BldNetworkClientSlim::getDebugLevel()
{
    return _iDebugLevel;
}

int BldNetworkClientSlim::setPort(unsigned short uPort)
{
  _uPort = uPort;
  return  0;
}


int BldNetworkClientSlim::setAddr(unsigned int uAddr)
{
  _uAddr = uAddr;
  return  0;
}


int BldNetworkClientSlim::sendRawData(int iSizeData, const char* pData)
{
    int iRetErrorCode = 0;
    /*
     * sendmsg
     */     

    //// ! Debug only
    //printf("Bld send to %x port %d Data String: %s\n", _uAddr, _uPort, pData);
    
    sockaddr_in sockaddrDst;
    sockaddrDst.sin_family      = AF_INET;
    sockaddrDst.sin_addr.s_addr = htonl(_uAddr);
    sockaddrDst.sin_port        = htons(_uPort);    
    
    struct iovec iov[2];
    iov[0].iov_base = (caddr_t)(pData);
    iov[0].iov_len  = iSizeData;
    
    struct msghdr hdr;
    hdr.msg_iovlen      = 1;        
    hdr.msg_name        = (sockaddr*) &sockaddrDst;
    hdr.msg_namelen     = sizeof(sockaddrDst);
    hdr.msg_control     = (caddr_t)0;
    hdr.msg_controllen  = 0;
    hdr.msg_iov         = &iov[0];

    unsigned int uSendFlags = 0;
try
{
    if ( 
      sendmsg(_iSocket, &hdr, uSendFlags) 
      == -1 )
        throw string("BldNetworkClientSlim::sendRawData() : sendmsg failed");                  
}
catch (string& sError)
{
    printf( "[Error] %s, size = %u, errno = %d (%s)\n", sError.c_str(), iSizeData, errno,
      strerror(errno) );
    printf( "[Error] %s, hdr.msg_iovlen = %zu, hdr.msg_iov->iov_len = %zu \n", sError.c_str(), hdr.msg_iovlen, hdr.msg_iov->iov_len );
      
    iRetErrorCode = 1;
}

    return iRetErrorCode;   
}

/*
 * private static functions
 */
string BldNetworkClientSlim::addressToStr( unsigned int uAddr )
{
    unsigned int uNetworkAddr = htonl(uAddr);
    const unsigned char* pcAddr = (const unsigned char*) &uNetworkAddr;
    std::stringstream sstream;
    sstream << 
      (int) pcAddr[0] << "." <<
      (int) pcAddr[1] << "." <<
      (int) pcAddr[2] << "." <<
      (int) pcAddr[3];
      
     return sstream.str();
}

} // namespace EpicsBld

extern "C"
{


int BldNetworkClientSetPort(unsigned short uPort, void* pVoidBldNetworkClient)
{
    if ( pVoidBldNetworkClient == NULL )
        return -1;

    EpicsBld::BldNetworkClientSlim* pBldNetworkClient = 
      reinterpret_cast<EpicsBld::BldNetworkClientSlim*>(pVoidBldNetworkClient);      

    return pBldNetworkClient->setPort(uPort);  
}

int BldNetworkClientSetAddr(unsigned int uAddr, void* pVoidBldNetworkClient)
{
    if ( pVoidBldNetworkClient == NULL )
        return -1;

    EpicsBld::BldNetworkClientSlim* pBldNetworkClient = 
      reinterpret_cast<EpicsBld::BldNetworkClientSlim*>(pVoidBldNetworkClient);      

    return pBldNetworkClient->setAddr(uAddr);  
}
}