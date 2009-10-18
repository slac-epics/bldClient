#include "bldPacket.h"

/*
 * class member definitions
 */
namespace EpicsBld
{   
/**
 * class BldPacketHeader
 */

const int BldPacketHeader::liBldPacketSizeByBldType[] = 
{ 
    0, 
    0, 
    sizeof(double)*4 
};

const BldPacketHeader::XtcDataType BldPacketHeader::ltXtcDataTypeByBldType[] =
{
    Any,
    Id_FEEGasDetEnergy,
    Id_FEEGasDetEnergy
};

const BldPacketHeader::TSetPvFuncPointer BldPacketHeader::lfuncSetvFunctionTable[] =
{
	&BldPacketHeader::setPvValuePulseEnergy,
	&BldPacketHeader::setPvValuePhotonEnergy,
	&BldPacketHeader::setPvValueFEEGasDetEnergy,
};


BldPacketHeader::BldPacketHeader(
	unsigned int	uMaxPacketSize,
	uint32_t		uSecs1,
	uint32_t		uNanoSecs1,
	uint32_t		uFiducialId1, 
	uint32_t		uDamage1,
	uint32_t		uPhysicalId1,
	uint32_t		uDataType1	)
	:	uSecs(			setu32LE(uSecs1)		),
		uNanoSecs(		setu32LE(uNanoSecs1)	),
		uMBZ1(			0						), 
		uFiducialId(	setu32LE(uFiducialId1)	),
		uMBZ2(			0						),
		uDamage(		setu32LE(uDamage1)		), 
		uLogicalId(		setu32LE(uBldLogicalId)	),
		uPhysicalId(	setu32LE(uPhysicalId1)	),
		uDataType(		setu32LE(uDataType1)	)
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
    
int BldPacketHeader::setPvValue( int iPvIndex, void* pPvValue )
{
    return ( this->*lfuncSetvFunctionTable[ setu32LE(uPhysicalId) ] ) ( iPvIndex, pPvValue );
}

int BldPacketHeader::setPvValuePulseEnergy( int iPvIndex, void* pPvValue )
{
    return 0;
}

int BldPacketHeader::setPvValuePhotonEnergy( int iPvIndex, void* pPvValue )
{
    double* pSrcPvValue = (double*) pPvValue;
    double* pDstPvValue = ((double*) (this + 1)) + iPvIndex;
    
    *pDstPvValue = setdoubleLE(*pSrcPvValue);
    return 0;
}

int BldPacketHeader::setPvValueFEEGasDetEnergy( int iPvIndex, void* pPvValue )
{
    double* pSrcPvValue = (double*) pPvValue;
    double* pDstPvValue = ((double*) (this + 1)) + iPvIndex;
    
    *pDstPvValue = setdoubleLE(*pSrcPvValue);
    return 0;
}

} // namespace EpicsBld
