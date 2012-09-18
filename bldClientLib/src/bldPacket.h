#ifndef BLD_PACKET_H
#define BLD_PACKET_H
#include <stdio.h>
#include <stdint.h>

namespace EpicsBld
{   

typedef int (*TSetPvFuncPointer)(int iPvIndex, void* pPvValue, void* payload);
    
class BldPacketHeader
{
public:
    /* 
     * Memory Mapped Data
     */
    uint32_t uNanoSecs;
    uint32_t uSecs;
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
    // Note that this is only the ones *we* handle!  Max is set large for the future!
    enum BldTypeId { EBeam, PhaseCavity, FEEGasDetEnergy, NumberOfBldTypeId=100 }; 
    /*
       EBeam bld does not use this module
     */
 
    // Imported from PDS repository: pdsdata/xtc/TypeId.hh : TypId::Type
    // Note that this is only the ones *we* handle!  Max is set large for the future!
    enum XtcDataType 
    {
        Any                 = 0, 
        Id_Xtc              = 1,          // generic hierarchical container
        Id_Frame            = 2,        // raw image
        Id_AcqWaveform      = 3,
        Id_AcqConfig        = 4,
        Id_TwoDGaussian     = 5, // 2-D Gaussian + covariances
        Id_Opal1kConfig     = 6,
        Id_FrameFexConfig   = 7,
        Id_EvrConfig        = 8,
        Id_TM6740Config     = 9,
        Id_ControlConfig    = 10,
        Id_pnCCDframe       = 11,
        Id_pnCCDconfig      = 12,
        Id_Epics            = 13,        // Epics Data Type
        Id_FEEGasDetEnergy  = 14,
        Id_EBeam            = 15,
        Id_PhaseCavity      = 16,
        NumberOfXtcDataType=200
    };

    /*
     * Public functions
     */    
    BldPacketHeader( unsigned int uMaxPacketSize, uint32_t uSecs1, uint32_t uNanoSecs1, uint32_t uFiducialId1, 
      uint32_t uDamage1, uint32_t uPhysicalId1, uint32_t uDataType1);


    static void Initialize(void);

    static void Register(unsigned int uPhysicalId, uint32_t uDataType, unsigned int pktsize,
                         TSetPvFuncPointer func) 
    {
        Initialize();
        if (uPhysicalId < NumberOfBldTypeId) {
            ltXtcDataTypeByBldType[uPhysicalId] = (XtcDataType) uDataType;
            liBldPacketSizeByBldType[uPhysicalId] = pktsize;
            lfuncSetvFunctionTable[uPhysicalId] = func;
        }
    }

    unsigned int getPacketSize()
    {
        return (unsigned int)  setu32LE(uExtentSize) + 10 * sizeof(uint32_t);
    }

    int setPvValue( int iPvIndex, void* pPvValue );
    
private:    
    static const uint32_t uBldLogicalId = 0x06000000; // from PDS Repository: pdsdata/xtc/Level.hh: Level::Reporter            
    static const uint32_t uDamgeTrue = 0x4000; // from Bld ICD

    static int init_done;
    
    /*
     * static tables
     */    
    static XtcDataType ltXtcDataTypeByBldType[NumberOfBldTypeId];
    static int liBldPacketSizeByBldType[NumberOfBldTypeId];
    static TSetPvFuncPointer lfuncSetvFunctionTable[NumberOfBldTypeId];

public:  
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
