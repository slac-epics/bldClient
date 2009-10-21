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
    sizeof(double)*4, 
    sizeof(double)*4 
};

const BldPacketHeader::XtcDataType BldPacketHeader::ltXtcDataTypeByBldType[] =
{
    Any,
    Id_PhaseCavity,
    Id_FEEGasDetEnergy,    
};

const BldPacketHeader::TSetPvFuncPointer BldPacketHeader::lfuncSetvFunctionTable[] =
{
    &BldPacketHeader::setPvValuePulseEnergy,
    &BldPacketHeader::setPvValuePhaseCavity,
    &BldPacketHeader::setPvValueFEEGasDetEnergy,
};
    
int BldPacketHeader::setPvValue( int iPvIndex, void* pPvValue )
{
    return ( this->*lfuncSetvFunctionTable[ setu32LE(uPhysicalId) ] ) ( iPvIndex, pPvValue );
}

int BldPacketHeader::setPvValuePulseEnergy( int iPvIndex, void* pPvValue )
{
    return 0;
}

int BldPacketHeader::setPvValuePhaseCavity( int iPvIndex, void* pPvValue )
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
