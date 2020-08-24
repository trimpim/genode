TARGET   = imx8q_i2c_drv

REQUIRES = arm_v8

SRC_CC  += driver.cc

INC_DIR += $(PRG_DIR)/src/drivers/i2c/spec/imx8q_evk

include $(REP_DIR)/src/drivers/i2c/target.inc