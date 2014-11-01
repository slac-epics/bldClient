#include "bldPacket.h"
#include <string.h>

/*
 * class member definitions
 */
namespace EpicsBld
{   
/**
 * class BldPacketHeader
 */

int								BldPacketHeader::liBldPacketSizeByBldType[	BldPacketHeader::NumberOfBldTypeId	];
BldPacketHeader::XtcDataType	BldPacketHeader::ltXtcDataTypeByBldType[	BldPacketHeader::NumberOfBldTypeId	];
TSetPvFuncPointer				BldPacketHeader::lfuncSetvFunctionTable[	BldPacketHeader::NumberOfBldTypeId	];
int BldPacketHeader::init_done = 0;

BldPacketHeader::BldPacketHeader(
    unsigned int    uMaxPacketSize,
    uint32_t        uSecs1,
    uint32_t        uNanoSecs1,
    uint32_t        uFiducialId1, 
    uint32_t        uDamage1,
    uint32_t        uPhysicalId1,
    uint32_t        uDataType1  )
    :   uSecs(          setu32LE(uSecs1)        ),
        uNanoSecs(      setu32LE(uNanoSecs1)    ),
        uMBZ1(          0                       ), 
        uFiducialId(    setu32LE(uFiducialId1)  ),
        uMBZ2(          0                       ),
        uDamage(        setu32LE(uDamage1)      ), 
        uLogicalId(     setu32LE(uBldLogicalId) ),
        uPhysicalId(    setu32LE(uPhysicalId1)  ),
        uDataType(      setu32LE(uDataType1)    )
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
 
    int xtcDataType = uDataType1 & 0xFFFF;
    if ( xtcDataType != ltXtcDataTypeByBldType[uPhysicalId1] )
    {
        printf( "BldPacketHeader::BldPacketHeader() Input argument uDataType value (%lu) is "
          "not compatible with physical id %lu. Expected uDataType value = %d.\n",
            (unsigned long) xtcDataType, (unsigned long) uPhysicalId1, ltXtcDataTypeByBldType[uPhysicalId1] );
            
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
    
int BldPacketHeader::setPvValue( int iPvIndex, void* pPvValue )
{
    TSetPvFuncPointer fn = this->lfuncSetvFunctionTable[ setu32LE(uPhysicalId) ];
    if (fn)
        return ( *fn ) ( iPvIndex, pPvValue, (void *)(this + 1));
    else
        return 0;
}

int setPvValuePulseEnergy( int iPvIndex, void* pPvValue, void *payload )
{
    return 0;
}

int setPvValuePhaseCavity( int iPvIndex, void* pPvValue, void *payload )
{
    double* pSrcPvValue = (double*) pPvValue;
    double* pDstPvValue = ((double*) payload) + iPvIndex;
    
    *pDstPvValue = BldPacketHeader::setdoubleLE(*pSrcPvValue);
    return 0;
}

int setPvValueFEEGasDetEnergy( int iPvIndex, void* pPvValue, void *payload )
{
    double* pSrcPvValue = (double*) pPvValue;
    double* pDstPvValue = ((double*) payload) + iPvIndex;
    
    *pDstPvValue = BldPacketHeader::setdoubleLE(*pSrcPvValue);
    return 0;
}

int setPvValueGMD( int iPvIndex, void* pPvValue, void *payload )
{
	char		*	pDstPvString = (char *) payload;
#if 0
	if ( iPvIndex == 0 )
	{
		const char	*	pSrcPvValue = (char *) pPvValue;
		strncpy( pDstPvString, pSrcPvValue, 32 );
	}
	else
#endif
	{
		double	*	pSrcPvValue = (double*) pPvValue;
//		double	*	pDstPvValue = ((double*) (pDstPvString+32)) + (iPvIndex-1);
		double	*	pDstPvValue = (double*) pDstPvString + iPvIndex;

		*pDstPvValue = BldPacketHeader::setdoubleLE(*pSrcPvValue);
	}
    return 0;
}

// GMD Packet: Version 0: 32 byte string, 15 doubles, OBSOLETE
// GMD Packet: Version 1: 6 doubles
//	From 
//	cPclass BldDataGMDV1
//	{
//		public:
//		enum	{	version	=	1	};
//	
//		double	milliJoulesPerPulse;	// Shot to shot pulse energy (mJ)
//		double	milliJoulesAverage;		// Average pulse energy from ION cup current (mJ)
//		double	correctedSumPerPulse;	// Bg corrected waveform integrated within limits in raw A/D counts
//		double	bgValuePerSample;		// Avg background value per sample in raw A/D counts
//		double	relativeEnergyPerPulse;	// Shot by shot pulse energy in arbitrary units
//		double	spare1;					// Spare value for use as needed
//	
//		int print() const;
//	};
//
//const size_t	sizeGMD = 32 + sizeof(double)*15;
const size_t	sizeGMD = sizeof(double)*6;
void BldPacketHeader::Initialize(void)
{
    if (!init_done) {
        init_done = 1; /* Do this first, so we don't loop! */
        Register(EBeam,           Any,                0,                &setPvValuePulseEnergy);
        Register(PhaseCavity,     Id_PhaseCavity,     sizeof(double)*4, &setPvValuePhaseCavity);
        Register(FEEGasDetEnergy, Id_FEEGasDetEnergy, sizeof(double)*6, &setPvValueFEEGasDetEnergy);
        Register(GMD,			  Id_GMD,			  sizeGMD,			&setPvValueGMD );
    }
}

} // namespace EpicsBld

extern "C"
{
    void BldRegister(unsigned int uPhysicalId, uint32_t uDataType, unsigned int pktsize,
                     int (*func)(int iPvIndex, void* pPvValue, void* payload))
    {
        EpicsBld::BldPacketHeader::Register(uPhysicalId, uDataType, pktsize, func);
    }
}
