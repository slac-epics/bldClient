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
    enum BldTypeId { PulseEnergy, PhotonEnergy, FEEGasDetEnergy, NumberOfBldTypeId };
 
    // Imported from PDS repository: pdsdata/xtc/TypeId.hh : TypId::Type
    enum XtcDataType 
    {   Any					= 0, 
        Id_Xtc				= 1,          // generic hierarchical container
        Id_Frame			= 2,        // raw image
        Id_AcqWaveform		= 3,
        Id_AcqConfig		= 4,
        Id_TwoDGaussian		= 5, // 2-D Gaussian + covariances
        Id_Opal1kConfig		= 6,
        Id_FrameFexConfig	= 7,
        Id_EvrConfig		= 8,
        Id_TM6740Config		= 9,
        Id_ControlConfig	= 10,
        Id_pnCCDframe		= 11,
        Id_pnCCDconfig		= 12,
        Id_Epics			= 13,        // Epics Data Type
        Id_FEEGasDetEnergy	= 14,
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
      uint32_t uDamage1, uint32_t uPhysicalId1, uint32_t uDataType1);

    unsigned int getPacketSize()
	{
		return (unsigned int)  setu32LE(uExtentSize) + 10 * sizeof(uint32_t);
	}
    int setPvValue( int iPvIndex, void* pPvValue );
    
private:    
    static const uint32_t uBldLogicalId = 0x06000000; // from PDS Repository: pdsdata/xtc/Level.hh: Level::Reporter            
    static const uint32_t uDamgeTrue = 0x4000; // from Bld ICD
    
    typedef int (BldPacketHeader::*TSetPvFuncPointer)(int iPvIndex, void* pPvValue );
    static const TSetPvFuncPointer lfuncSetvFunctionTable[NumberOfBldTypeId];
    
    int setPvValuePulseEnergy( int iPvIndex, void* pPvValue );
    int setPvValuePhotonEnergy( int iPvIndex, void* pPvValue );
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
