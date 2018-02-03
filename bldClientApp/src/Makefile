TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# build an ioc application

PROD_IOC_Linux += BldTestApp

#DBD += BldTestApp.dbd

# BldTestApp.dbd will be made up from these files:
#BldTestApp_DBD += base.dbd
#include definitions for any other support applications needed

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
#BldTestApp_SRCS += BldTestApp_registerRecordDeviceDriver.cpp
BldTestApp_SRCS += BldTestAppMain.cpp
BldTestApp_SRCS += bldNetworkClientTest.cpp

#add a definition for each support application used by this application
#BldTestApp_LIBS_RTEMS += foo
BldTestApp_LIBS += bldClient
BldTestApp_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

