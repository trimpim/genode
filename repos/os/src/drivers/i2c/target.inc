LIBS    += base

INC_DIR += $(PRG_DIR)
INC_DIR += $(REP_DIR)/src/drivers/i2c

SRC_CC  += component.cc

vpath %.cc $(REP_DIR)/src/drivers/i2c
vpath %.cc $(PRG_DIR)