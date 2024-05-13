SHARED_LIB := yes

SRC_CC     += i2c_vfs.cc
SRC_CC     += i2c_file_system.cc

INC_DIR    += $(REP_DIR)/src/lib/vfs/i2c
INC_DIR    += $(REP_DIR)/src/include

vpath % $(REP_DIR)/src/lib/vfs/i2c
