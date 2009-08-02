#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <algorithm>
#include <memory>
#include <string>

#include <dbDefs.h>
#include <registryFunction.h>
#include <subRecord.h>
#include <epicsExport.h>
#include <dbAddr.h>
#include <dbAccess.h>
#include <dbTest.h>

#include "bldPvClient.h"

extern "C"
{

int bldSubDebug = 0;

typedef long (*TFuncProcess)(subRecord *pSubrecord);

static long bldSubInit(subRecord *pSubrecord,TFuncProcess process)
{
    if (bldSubDebug)
        printf("bldSubInit() : Record %s called bldSubInit(%p, %p)\n",
          pSubrecord->name, (void*) pSubrecord, (void*) process);
          
    BldSetSub( pSubrecord->name );
          
    return(0);
}

static long bldSubProcess(subRecord *pSubrecord)
{
    if (bldSubDebug)
        printf("bldSubProcess() : Record %s called bldSubProcess(%p)\n",
          pSubrecord->name, (void*) pSubrecord); 
    
    BldSendData();
        
    return(0);
}

/* Register these symbols for use by IOC code: */

epicsExportAddress(int, bldSubDebug);
epicsRegisterFunction(bldSubInit);
epicsRegisterFunction(bldSubProcess);

} // extern "C"
