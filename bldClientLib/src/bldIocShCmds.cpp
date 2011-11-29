#include <epicsExport.h>
#include <iocsh.h>

#include "bldPvClient.h"

static int bldidx = 0;

/* Information needed by iocsh */

static const iocshArg     BldSetIDArgs[] =
{
    {"ID", iocshArgInt},
};

static const iocshArg*    BldSetIDArgPtrs[] = 
{ BldSetIDArgs };

static const iocshArg     BldConfigArgs[] = 
{
    {"sAddr", iocshArgString},
    {"uPort", iocshArgInt},
    {"uMaxDataSize", iocshArgInt},
    {"sInterfaceIp", iocshArgString},
    {"uSrcPyhsicalId", iocshArgInt},
    {"iDataType", iocshArgInt},
    {"sBldPvPreTrigger", iocshArgString},
    {"sBldPvPostTrigger", iocshArgString},
    {"sBldPvFiducial", iocshArgString},
    {"sBldPvList", iocshArgString},
};


static const iocshArg*    BldConfigArgPtrs[] = 
{ 
  BldConfigArgs, BldConfigArgs+1, BldConfigArgs+2,
  BldConfigArgs+3, BldConfigArgs+4, BldConfigArgs+5,
  BldConfigArgs+6, BldConfigArgs+7, BldConfigArgs+8,
  BldConfigArgs+9,
};

static const iocshArg     BldSetPreSubArgs[] = 
{
    {"sBldPreSubRec", iocshArgString},
};

static const iocshArg     BldSetPostSubArgs[] = 
{
    {"sBldPostSubRec", iocshArgString},
};

static const iocshArg*    BldSetPreSubArgPtrs[] = 
{ BldSetPreSubArgs };

static const iocshArg*    BldSetPostSubArgPtrs[] = 
{ BldSetPostSubArgs };

static const iocshArg     BldSetDebugLevelArgs[] = 
{
    {"iDebugLevel", iocshArgInt},
};

static const iocshArg*    BldSetDebugLevelArgPtrs[] = 
{ BldSetDebugLevelArgs };

static const iocshFuncDef iocShBldSetIDFuncDef = {"BldSetID", 1, BldSetIDArgPtrs};
static const iocshFuncDef iocShBldStartFuncDef = {"BldStart", 0, NULL};
static const iocshFuncDef iocShBldStopFuncDef = {"BldStop", 0, NULL};
static const iocshFuncDef iocShBldIsStartedFuncDef = {"BldIsStarted", 0, NULL};
static const iocshFuncDef iocShBldConfigFuncDef = {"BldConfig", 10, BldConfigArgPtrs};
static const iocshFuncDef iocShBldShowConfigFuncDef = {"BldShowConfig", 0, NULL};
static const iocshFuncDef iocShBldSetPreSubFuncDef = {"BldSetPreSub", 1, BldSetPreSubArgPtrs};
static const iocshFuncDef iocShBldSetPostSubFuncDef = {"BldSetPostSub", 1, BldSetPostSubArgPtrs};
static const iocshFuncDef iocShBldSendDataFuncDef = {"BldSendData", 0, NULL};
static const iocshFuncDef iocShBldPrepareDataFuncDef = {"BldPrepareData", 0, NULL};
static const iocshFuncDef iocShBldSetDebugLevelFuncDef = {"BldSetDebugLevel", 1, BldSetDebugLevelArgPtrs};
static const iocshFuncDef iocShBldGetDebugLevelFuncDef = {"BldGetDebugLevel", 0, NULL};

/* Wrapper called by iocsh, selects the argument types that iocShBldStart needs */
static void iocShBldSetIDCallFunc(const iocshArgBuf *args) 
{
    if (args[0].ival >= 0 && args[0].ival < 10) {
        bldidx = args[0].ival;
        printf("BLD ID set to %d.\n", bldidx);
    } else
        printf("%d is out of range, BLD ID remains %d.\n", args[0].ival, bldidx);
}

static void iocShBldStartCallFunc(const iocshArgBuf *args) 
{
    BldStart(bldidx);
}

static void iocShBldStopCallFunc(const iocshArgBuf *args) 
{
    BldStop(bldidx);
}

static void iocShBldIsStartedCallFunc(const iocshArgBuf *args) 
{
    printf( "bld is %sstarted.\n", (BldIsStarted(bldidx) == 0?"not ":"")  );
}

static void iocShBldConfigCallFunc(const iocshArgBuf *args) 
{
    BldConfig( bldidx, args[0].sval, args[1].ival, args[2].ival, args[3].sval, args[4].ival, args[5].ival,
               args[6].sval, args[7].sval, args[8].sval, args[9].sval  );
}

static void iocShBldShowConfigCallFunc(const iocshArgBuf *args) 
{
    BldShowConfig(bldidx);
}

static void iocShBldSetPreSubCallFunc(const iocshArgBuf *args) 
{
    BldSetPreSub( bldidx, args[0].sval );
}

static void iocShBldSetPostSubCallFunc(const iocshArgBuf *args) 
{
    BldSetPostSub( bldidx, args[0].sval );
}

static void iocShBldPrepareDataCallFunc(const iocshArgBuf *args) 
{
    BldPrepareData(bldidx);
}

static void iocShBldSendDataCallFunc(const iocshArgBuf *args) 
{
    BldSendData(bldidx);
}

static void iocShBldSetDebugLevelCallFunc(const iocshArgBuf *args) 
{
    BldSetDebugLevel( bldidx, args[0].ival );
}

static void iocShBldGetDebugLevelCallFunc(const iocshArgBuf *args) 
{
    printf( "bld debug level = %d\n", BldGetDebugLevel(bldidx) );
}

/* Registration routine, runs at startup */
void iocShBldSetIDRegister(void) 
  { iocshRegister(&iocShBldSetIDFuncDef, iocShBldSetIDCallFunc); }
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
static void iocShBldSetPreSubRegister(void) 
  { iocshRegister(&iocShBldSetPreSubFuncDef, iocShBldSetPreSubCallFunc); }
static void iocShBldSetPostSubRegister(void) 
{ iocshRegister(&iocShBldSetPostSubFuncDef, iocShBldSetPostSubCallFunc); }
static void iocShBldSendDataRegister(void) 
  { iocshRegister(&iocShBldSendDataFuncDef, iocShBldSendDataCallFunc); }
static void iocShBldPrepareDataRegister(void) 
{ iocshRegister(&iocShBldPrepareDataFuncDef, iocShBldPrepareDataCallFunc); }
static void iocShBldSetDebugLevelRegister(void) 
  { iocshRegister(&iocShBldSetDebugLevelFuncDef, iocShBldSetDebugLevelCallFunc); }
static void iocShBldGetDebugLevelRegister(void) 
  { iocshRegister(&iocShBldGetDebugLevelFuncDef, iocShBldGetDebugLevelCallFunc); }

epicsExportRegistrar(iocShBldSetIDRegister);
epicsExportRegistrar(iocShBldStartRegister);
epicsExportRegistrar(iocShBldStopRegister);
epicsExportRegistrar(iocShBldIsStartedRegister);
epicsExportRegistrar(iocShBldConfigRegister);
epicsExportRegistrar(iocShBldShowConfigRegister);
epicsExportRegistrar(iocShBldSetPreSubRegister);
epicsExportRegistrar(iocShBldSetPostSubRegister);
epicsExportRegistrar(iocShBldSendDataRegister);
epicsExportRegistrar(iocShBldPrepareDataRegister);
epicsExportRegistrar(iocShBldSetDebugLevelRegister);
epicsExportRegistrar(iocShBldGetDebugLevelRegister);

