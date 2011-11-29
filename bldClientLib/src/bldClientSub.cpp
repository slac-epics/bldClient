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

int bldPreSubDebug = 0;
int bldPostSubDebug = 0;

typedef long (*TFuncProcess)(subRecord *pSubrecord);

static long bldPreSubInit(subRecord *pSubrecord,TFuncProcess process)
{
    if (bldPreSubDebug)
        printf("bldPreSubInit() : Record %s called bldPreSubInit(%p, %p)\n",
          pSubrecord->name, (void*) pSubrecord, (void*) process);
          
    BldSetPreSub( (int) pSubrecord->a, pSubrecord->name );
          
    return(0);
}

static long bldPreSubProcess(subRecord *pSubrecord)
{
    if (bldPreSubDebug)
        printf("bldPreSubProcess() : Record %s called bldPreSubProcess(%p)\n",
          pSubrecord->name, (void*) pSubrecord); 
    
    BldPrepareData(pSubrecord->a);
        
    return(0);
}

static long bldPostSubInit(subRecord *pSubrecord,TFuncProcess process)
{
	if (bldPostSubDebug)
		printf("bldPostSubInit() : Record %s called bldPostSubInit(%p, %p)\n",
					 pSubrecord->name, (void*) pSubrecord, (void*) process);
          
	BldSetPostSub( (int) pSubrecord->a, pSubrecord->name );
          
	return(0);
}

static long bldPostSubProcess(subRecord *pSubrecord)
{
	if (bldPostSubDebug)
		printf("bldPostSubProcess() : Record %s called bldPostSubProcess(%p)\n",
					 pSubrecord->name, (void*) pSubrecord); 
    
	BldSendData((int) pSubrecord->a);
        
	return(0);
}

/* Register these symbols for use by IOC code: */

epicsExportAddress(int, bldPreSubDebug);
epicsRegisterFunction(bldPreSubInit);
epicsRegisterFunction(bldPreSubProcess);
epicsExportAddress(int, bldPostSubDebug);
epicsRegisterFunction(bldPostSubInit);
epicsRegisterFunction(bldPostSubProcess);

} // extern "C"
