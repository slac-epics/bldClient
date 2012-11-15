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
	// /reg/g/pcds/dist/pds/sxr/princeton_tomy/pdsdata/xtc/BldInfo.hh
    // Note that this is only the ones *we* handle!  Max is set large for the future!
    enum BldTypeId	{	EBeam, PhaseCavity, FEEGasDetEnergy,
						Nh2Sb1Ipm01,  
						HxxUm6Imb01, HxxUm6Imb02,
						HfxDg2Imb01, HfxDg2Imb02,
						XcsDg3Imb03, XcsDg3Imb04,
						HfxDg3Imb01, HfxDg3Imb02,
						HxxDg1Cam,   HfxDg2Cam,
						HfxDg3Cam,   XcsDg3Cam,
						HfxMonCam,
						HfxMonImb01, HfxMonImb02,
						HfxMonImb03,
						MecLasEm01, MecTctrPip01,
						MecTcTrDio01,
						MecXt2Ipm02, MecXt2Ipm03, 
						MecHxmIpm01,
						GMD,
						NumberOfBldTypeId=100 }; 
    /*
       EBeam bld does not use this module
     */
 
    // Imported from PDS repository: pdsdata/xtc/TypeId.hh : TypId::Type
	// /reg/g/pcds/dist/pds/sxr/princeton_tomy/pdsdata/xtc/TypeId.hh
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
		Id_PrincetonFrame	= 17,
		Id_PrincetonConfig	= 18,
		Id_EvrData			= 19,
		Id_FrameFccdConfig	= 20,
		Id_FccdConfig		= 21,
		Id_IpimbData		= 22,
		Id_IpimbConfig		= 23,
		Id_EncoderData		= 24,
		Id_EncoderConfig	= 25,
		Id_EvrIOConfig		= 26,
		Id_PrincetonInfo	= 27,
		Id_CspadElement		= 28,
		Id_CspadConfig		= 29,
		Id_IpmFexConfig		= 30,  // LUSI Diagnostics
		Id_IpmFex			= 31,
		Id_DiodeFexConfig	= 32,
		Id_DiodeFex			= 33,
		Id_PimImageConfig	= 34,
		Id_SharedIpimb		= 35,
		Id_AcqTdcConfig		= 36,
		Id_AcqTdcData		= 37,
		Id_Index			= 38,
		Id_XampsConfig		= 39,
		Id_XampsElement		= 40,
		Id_Cspad2x2Element	= 41,
		Id_SharedPim		= 42,
		Id_Cspad2x2Config	= 43,
		Id_FexampConfig		= 44,
		Id_FexampElement	= 45,
		Id_Gsc16aiConfig	= 46,
		Id_Gsc16aiData		= 47,
		Id_PhasicsConfig	= 48,
		Id_TimepixConfig	= 49,
		Id_TimepixData		= 50,
		Id_CspadCompressedElement	= 51,
		Id_OceanOpticsConfig	= 52,
		Id_OceanOpticsData	= 53,
		Id_EpicsConfig		= 54,
		Id_FliConfig		= 55,
		Id_FliFrame			= 56,
		Id_QuartzConfig		= 57,
		Reserved1			= 58,	// previously Id_CompressedFrame        : no corresponding class
		Reserved2			= 59,	// previously Id_CompressedTimePixFrame : no corresponding class
		Id_AndorConfig		= 60,
		Id_AndorFrame		= 61,
		Id_UsdUsbData		= 62,
		Id_UsdUsbConfig		= 63,
		Id_GMD				= 64,
        NumberOfXtcDataType	= 200
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

extern "C" void BldRegister(	unsigned int	uPhysicalId,
								uint32_t		uDataType,
								unsigned int	pktsize,
                            	int (*func)(int iPvIndex, void* pPvValue, void* payload) );

#endif
