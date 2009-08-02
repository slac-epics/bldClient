#ifndef MULTICAST_BLD_LIB_H
#define MULTICAST_BLD_LIB_H

namespace EpicsBld
{   
/**
 * Abastract Interface of Bld Multicast Client 
 * 
 * The interface class for providing Bld Mutlicast Client functions.
 *
 * Design Issue:
 * 1. The value semantics are disabled. 
 */
class BldNetworkClientInterface
{
public:
    /**
     * Send raw data out to the Bld Server
     *
     * @param iSizeData  size of data (in bytes)
     * @param pData      pointer to the data buffer (char[] buffer)
     * @return  0 if successful,  otherwise the "errno" code (see <errno.h>)
     */
    virtual int sendRawData(int iSizeData, const char* pData) = 0;
    
    // debug information control
    virtual void setDebugLevel(int iDebugLevel) = 0;
    virtual int getDebugLevel() = 0;
    
    virtual ~BldNetworkClientInterface() {} /// polymorphism support
protected:  
    BldNetworkClientInterface() {} /// To be called from implementation classes
private:
    ///  Disable value semantics. No definitions (function bodies).
    BldNetworkClientInterface(const BldNetworkClientInterface&);
    BldNetworkClientInterface& operator=(const BldNetworkClientInterface&);
};

/**
 * Factory class of Bld Multicast Client 
 *
 * Design Issue:
 * 1. Object semantics are disabled. Only static utility functions are provided.
 * 2. Later if more factorie classes are needed, this class can have public 
 *    constructor, virtual destructor and virtual member functions.
 */
class BldNetworkClientFactory
{
public:
    /**
     * Create a Bld Client object
     *
     * @param uAddr         mutlicast address. Usually with 239.0.0.1 ~ 239.255.255.255
     * @param uPort         UDP port
     * @param uMaxDataSize  Maximum Bld data size. Better to be less than MTU.
     * @param ucTTL         TTL value in UDP packet. Ideal value is 1 + (# of middle routers)
     * @param sInteraceIp   Specify the NIC by IP address (in c string format)
     * @return              The created Bld Client object
     */
    static BldNetworkClientInterface* createBldNetworkClient(unsigned int uAddr, 
      unsigned short uPort, unsigned int uMaxDataSize, unsigned char ucTTL = 32, 
      const char* sInteraceIp = 0);
        
    /**
     * Create a Bld Client object
     *
     * Overloaded version
     * @param uInteraceIp   Specify the NIC by IP address (in unsigned int format)
     */
    static BldNetworkClientInterface* createBldNetworkClient(unsigned int uAddr, 
      unsigned short uPort, unsigned int uMaxDataSize, unsigned char ucTTL = 32, 
      unsigned int uInteraceIp = 0);
private:
    /// Disable object instantiation (No object semantics).
    BldNetworkClientFactory();
};

} // namespace EpicsBld

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
  void** ppVoidBldNetworkClient);
int BldNetworkClientInitByInterfaceAddress(unsigned int uAddr, unsigned short uPort, 
  unsigned int uMaxDataSize, unsigned char ucTTL, unsigned int uInterfaceIp, 
  void** ppVoidBldNetworkClient);  

/**
 * Release function: Call C++ delete operator to delete the BldNetworkClient
 */
int BldNetworkClientRelease(void* pVoidBldNetworkClient); 

/**
 * Call the Send function defined in EpicsBld::BldNetworkClientInterface 
 */
int BldNetworkClientSendRawData(void* pVoidBldNetworkClient, int iSizeData, char* pData);

} // extern "C"


#endif 
