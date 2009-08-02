#Makefile at top of application tree
TOP = .
include $(TOP)/configure/CONFIG
DIRS += configure
DIRS += bldClientLib
DIRS += bldClientApp
include $(TOP)/configure/RULES_TOP
