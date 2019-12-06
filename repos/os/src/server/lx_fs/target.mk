TARGET    = lx_fs
REQUIRES  = linux
SRC_CC    = main.cc
SRC_CC   += inotify.cc
LIBS      = lx_hybrid

INC_DIR += $(PRG_DIR) /usr/include
