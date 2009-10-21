#ifndef BLD_PACKET_H
#define BLD_PACKET_H
#include <stdio.h>
#include <stdint.h>

namespace EpicsBld
{   
    
class BldPacketHeader
{
public:
    /* 
     * Memory Mapped Data
     */
    uint32_t uSecs;
    uint32_t uNanoSecs;
    uint32_t uMBZ1;
    uint32_t uFiducialId;
    uint32_t uMBZ2;

    // Xtc Section 1
    uint32_t uDamage;
    uint32_t uLogicalId;
    uint32_t uPhysicalId;
    uint32_t uDataType;
    uint32_t uExtentSize;

    // Xtc Section 2
    uint32_t uDamage2;
    uint32_t uLogicalId2;
    uint32_t uPhysicalId2;
    uint32_t uDataType2;
    uint32_t uExtentSize2;
    
    /*
     * Enums imported from PDS repository
     */
     
    // Imported from PDS repository: pdsdata/xtc/BldInfo.hh : BldInfo::Type
    enum BldTypeId { EBeam, PhaseCavity, FEEGasDetEnergy, NumberOfBldTypeId }; 
    /*
       EBeam bld does not use this module
     */
    
    // Imported from PDS repository: pdsdata/xtc/TypeId.hh : TypId::Type
    enum XtcDataType {
      Any, 
      Id_Xtc,          // generic hierarchical container
      Id_Frame,        // raw image
      Id_AcqWaveform,
      Id_AcqConfig,
      Id_TwoDGaussian, // 2-D Gaussian + covariances
      Id_Opal1kConfig,
      Id_FrameFexConfig,
      Id_EvrConfig,
      Id_TM6740Config,
      Id_ControlConfig,
      Id_pnCCDframe,
      Id_pnCCDconfig,
      Id_Epics,        // Epics Data Type
      Id_FEEGasDetEnergy,
      Id_EBeam,
      Id_PhaseCavity,
      NumberOfXtcDataType
    };

    /*
     * static const tables
     */    
    static const int liBldPacketSizeByBldType[NumberOfBldTypeId];
    static const XtcDataType ltXtcDataTypeByBldType[NumberOfBldTypeId];

    /*
     * Public functions
     */    
    BldPacketHeader( unsigned int uMaxPacketSize, uint32_t uSecs1, uint32_t uNanoSecs1, uint32_t uFiducialId1, 
      uint32_t uDamage1, uint32_t uPhysicalId1, uint32_t uDataType1) :
      uSecs(setu32LE(uSecs1)), uNanoSecs(setu32LE(uNanoSecs1)), uMBZ1(0), 
      uFiducialId(setu32LE(uFiducialId1)), uMBZ2(0), uDamage(setu32LE(uDamage1)), 
      uLogicalId(setu32LE(uBldLogicalId)), uPhysicalId(setu32LE(uPhysicalId1)),
      uDataType(setu32LE(uDataType1))
    { 
        if ( uPhysicalId1 < 0 || uPhysicalId1 >= NumberOfBldTypeId )
        {
            printf( "BldPacketHeader::BldPacketHeader() Input argument uLogicalId value (%lu) is out of range\n",
                (unsigned long) uPhysicalId1 );
                
            uDamage = setu32LE(uDamgeTrue);
            uExtentSize = 0;
            return;
        }
       
        unsigned int uExtentSize1 = sizeof(BldPacketHeader) - 10 * sizeof(uint32_t) + liBldPacketSizeByBldType[uPhysicalId1];
            
        if ( uExtentSize1 > uMaxPacketSize )
        {
            printf( "BldPacketHeader::BldPacketHeader() Packet size (%u) is larger than given buffer size (%u) \n",
                (unsigned int) uExtentSize1, uMaxPacketSize );
                
            uDamage = setu32LE(uDamgeTrue);
            uExtentSize = 0;
            return;
        }
        
        uExtentSize = setu32LE(uExtentSize1);
        
        if ( (int) uDataType1 != ltXtcDataTypeByBldType[uPhysicalId1] )
        {
            printf( "BldPacketHeader::BldPacketHeader() Input argument uDataType value (%lu) is "
              "not compatible with physical id %lu. Expected uDataType value = %d.\n",
                (unsigned long) uDataType1, (unsigned long) uPhysicalId1, ltXtcDataTypeByBldType[uPhysicalId1] );
                
            uDamage = setu32LE(uDamgeTrue);
            uExtentSize = 0;
        }
        
        // Mirror Xtc Section 1 to Section 2
        uDamage2 = uDamage;
        uLogicalId2 = uLogicalId;
        uPhysicalId2 = uPhysicalId;
        uDataType2 = uDataType;
        uExtentSize2 = uExtentSize;        
    }        
    
    unsigned int getPacketSize() { return (unsigned int)  setu32LE(uExtentSize) + 10 * sizeof(uint32_t); }
    int setPvValue( int iPvIndex, void* pPvValue );
    
private:    
    static const uint32_t uBldLogicalId = 0x06000000; // from PDS Repository: pdsdata/xtc/Level.hh: Level::Reporter            
    static const uint32_t uDamgeTrue = 0x4000; // from Bld ICD
    
    typedef int (BldPacketHeader::*TSetPvFuncPointer)(int iPvIndex, void* pPvValue );
    static const TSetPvFuncPointer lfuncSetvFunctionTable[NumberOfBldTypeId];
    
    int setPvValuePulseEnergy( int iPvIndex, void* pPvValue );
    int setPvValuePhaseCavity( int iPvIndex, void* pPvValue );
    int setPvValueFEEGasDetEnergy( int iPvIndex, void* pPvValue );
    
// Check for little endian
#if defined(__rtems__)

#define BLD_BIG_ENDIAN

#endif

#ifdef BLD_BIG_ENDIAN
    inline static uint16_t setu16LE(uint16_t value) { return (value&0xFF)<<8 | (value&0xFF00)>>8; }
    inline static uint32_t setu32LE(uint32_t value) { return (value&0xFF)<<24 | (value&0xFF00)<<8 | (value&0xFF0000)>>8 | (value&0xFF000000)>>24; }
    inline static double setdoubleLE(double value) 
    { 
        union
        {
            struct { uint32_t uiVal1, uiVal2; };
            double fVal;
        } unTemp;
        
        unTemp.fVal = value;
        uint32_t uiValTmp = unTemp.uiVal1;

        unTemp.uiVal1 = setu32LE( unTemp.uiVal2 );
        unTemp.uiVal2 = setu32LE( uiValTmp );
        return unTemp.fVal;
    }
    
#else
    inline static uint16_t setu16LE(uint16_t value) { return value; }
    inline static uint32_t setu32LE(uint32_t value) { return value; }
    inline static double setdoubleLE(double value) { return value; }
#endif
    
};

}

#endif
