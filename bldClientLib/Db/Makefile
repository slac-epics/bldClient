TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------
# Create and install (or just install)
# databases, templates, substitutions like this
DB += bldSettings.db
DB += bldFanout.db

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_TEMPLATE = <templatename>

#
# Create and/or install archive files
ARCHIVE += bldSettings.tpl-arch

# Create and/or install autosave files
REQ += bldSettings.tpl-req

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

