TOP=../..

include $(TOP)/configure/CONFIG

#CROSS_COMPILER_TARGET_ARCHS=RTEMS-beatnik
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# Build an IOC support library

# Comment this line to build a host-ioc; you'd need to exclude
# several facilities (evr, micro, ...) for this to work.
#BUILDFOR=_RTEMS

LIBRARY_IOC	+= bldClient
INC			+= bldNetworkClient.h bldPvClient.h


#DBD        = foo.dbd
DBD			+= bldClient.dbd

#bldClient_DBD		+= base.dbd
bldClient_DBD		+= bldSub.dbd
bldClient_DBD		+= bldIocShCmds.dbd

bldClient_SRCS      += bldNetworkClient.cpp 
bldClient_SRCS      += bldPvClient.cpp
bldClient_SRCS      += bldClientSub.cpp
bldClient_SRCS      += bldIocShCmds.cpp
bldClient_SRCS      += bldPacket.cpp
bldClient_SRCS	    += bldClient_registerRecordDeviceDriver.cpp

#multicastBLDApp_LIBS += $(EPICS_BASE_IOC_LIBS)

#USR_CFLAGS  = -I$(TOP)/include -I$(RTEMS_BASE)/

#DB              += multicastBLDApp.db

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

