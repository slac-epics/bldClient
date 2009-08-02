TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# build an ioc application

PROD_IOC = bldClient

#DBD += bldClient.dbd

# bldClient.dbd will be made up from these files:
#bldClient_DBD += base.dbd
#bldClient_DBD += bldSub.dbd
#include definitions for any other support applications needed

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
#bldClient_SRCS += bldClient_registerRecordDeviceDriver.cpp
#bldClient_SRCS += bldClientSub.cpp
bldClient_SRCS += bldNetworkClientTest.cpp
bldClient_SRCS_Linux += bldClientMain.cpp
bldClient_SRCS_vxWorks += -nil-

#The following adds support from base/src/vxWorks
bldClient_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

#add a definition for each support application used by this application
#bldClient_LIBS_RTEMS += foo
bldClient_LIBS_Linux += $(EPICS_BASE_IOC_LIBS)

bldClient_LIBS += libBldClient

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

