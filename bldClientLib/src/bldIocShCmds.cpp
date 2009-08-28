#include <epicsExport.h>
#include <iocsh.h>

#include "bldPvClient.h"

/* Information needed by iocsh */
static const iocshArg     BldConfigArgs[] = 
{
    {"sAddr", iocshArgString},
    {"uPort", iocshArgInt},
    {"uMaxDataSize", iocshArgInt},
    {"sInterfaceIp", iocshArgString},
    {"uSrcPyhsicalId", iocshArgInt},
    {"iDataType", iocshArgInt},
    {"sBldPvTrigger", iocshArgString},
    {"sBldPvFiducial", iocshArgString},
    {"sBldPvList", iocshArgString},
};


static const iocshArg*    BldConfigArgPtrs[] = 
{ 
  BldConfigArgs, BldConfigArgs+1, BldConfigArgs+2,
  BldConfigArgs+3, BldConfigArgs+4, BldConfigArgs+5,
  BldConfigArgs+6, BldConfigArgs+7, BldConfigArgs+8,
};

static const iocshArg     BldSetSubArgs[] = 
{
    {"sBldSubRec", iocshArgString},
};

static const iocshArg*    BldSetSubArgPtrs[] = 
{ BldSetSubArgs };

static const iocshArg     BldSetDebugLevelArgs[] = 
{
    {"iDebugLevel", iocshArgInt},
};

static const iocshArg*    BldSetDebugLevelArgPtrs[] = 
{ BldSetDebugLevelArgs };

static const iocshFuncDef iocShBldStartFuncDef = {"BldStart", 0, NULL};
static const iocshFuncDef iocShBldStopFuncDef = {"BldStop", 0, NULL};
static const iocshFuncDef iocShBldIsStartedFuncDef = {"BldIsStarted", 0, NULL};
static const iocshFuncDef iocShBldConfigFuncDef = {"BldConfig", 9, BldConfigArgPtrs};
static const iocshFuncDef iocShBldShowConfigFuncDef = {"BldShowConfig", 0, NULL};
static const iocshFuncDef iocShBldSetSubFuncDef = {"BldSetSub", 1, BldSetSubArgPtrs};
static const iocshFuncDef iocShBldSendDataFuncDef = {"BldSendData", 0, NULL};
static const iocshFuncDef iocShBldSetDebugLevelFuncDef = {"BldSetDebugLevel", 1, BldSetDebugLevelArgPtrs};
static const iocshFuncDef iocShBldGetDebugLevelFuncDef = {"BldGetDebugLevel", 0, NULL};

/* Wrapper called by iocsh, selects the argument types that iocShBldStart needs */
static void iocShBldStartCallFunc(const iocshArgBuf *args) 
{
    BldStart();
}

static void iocShBldStopCallFunc(const iocshArgBuf *args) 
{
    BldStop();
}

static void iocShBldIsStartedCallFunc(const iocshArgBuf *args) 
{
    printf( "bld is %sstarted.\n", (BldIsStarted() == 0?"not ":"")  );
}

static void iocShBldConfigCallFunc(const iocshArgBuf *args) 
{
    BldConfig( args[0].sval, args[1].ival, args[2].ival, args[3].sval, args[4].ival, args[5].ival,
      args[6].sval, args[7].sval, args[8].sval );
}

static void iocShBldShowConfigCallFunc(const iocshArgBuf *args) 
{
    BldShowConfig();
}

static void iocShBldSetSubCallFunc(const iocshArgBuf *args) 
{
    BldSetSub( args[0].sval );
}

static void iocShBldSendDataCallFunc(const iocshArgBuf *args) 
{
    BldSendData();
}

static void iocShBldSetDebugLevelCallFunc(const iocshArgBuf *args) 
{
    BldSetDebugLevel( args[0].ival );
}

static void iocShBldGetDebugLevelCallFunc(const iocshArgBuf *args) 
{
    printf( "bld debug level = %d\n", BldGetDebugLevel() );
}

/* Registration routine, runs at startup */
static void iocShBldStartRegister(void) 
  { iocshRegister(&iocShBldStartFuncDef, iocShBldStartCallFunc); }
static void iocShBldStopRegister(void) 
  { iocshRegister(&iocShBldStopFuncDef, iocShBldStopCallFunc); }
static void iocShBldIsStartedRegister(void) 
  { iocshRegister(&iocShBldIsStartedFuncDef, iocShBldIsStartedCallFunc); }
static void iocShBldConfigRegister(void) 
  { iocshRegister(&iocShBldConfigFuncDef, iocShBldConfigCallFunc); }
static void iocShBldShowConfigRegister(void) 
  { iocshRegister(&iocShBldShowConfigFuncDef, iocShBldShowConfigCallFunc); }
static void iocShBldSetSubRegister(void) 
  { iocshRegister(&iocShBldSetSubFuncDef, iocShBldSetSubCallFunc); }
static void iocShBldSendDataRegister(void) 
  { iocshRegister(&iocShBldSendDataFuncDef, iocShBldSendDataCallFunc); }
static void iocShBldSetDebugLevelRegister(void) 
  { iocshRegister(&iocShBldSetDebugLevelFuncDef, iocShBldSetDebugLevelCallFunc); }
static void iocShBldGetDebugLevelRegister(void) 
  { iocshRegister(&iocShBldGetDebugLevelFuncDef, iocShBldGetDebugLevelCallFunc); }

epicsExportRegistrar(iocShBldStartRegister);
epicsExportRegistrar(iocShBldStopRegister);
epicsExportRegistrar(iocShBldIsStartedRegister);
epicsExportRegistrar(iocShBldConfigRegister);
epicsExportRegistrar(iocShBldShowConfigRegister);
epicsExportRegistrar(iocShBldSetSubRegister);
epicsExportRegistrar(iocShBldSendDataRegister);
epicsExportRegistrar(iocShBldSetDebugLevelRegister);
epicsExportRegistrar(iocShBldGetDebugLevelRegister);

