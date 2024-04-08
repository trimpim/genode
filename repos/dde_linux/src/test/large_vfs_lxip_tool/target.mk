TARGET  = large_vfs_lxip_tool

LIBS    = base libssl libcrypto \
          stdcxx vfs \
          libmosquitto libmosquittopp jsonc

SRC_CC  = main.cc

CC_CXX_WARN_STRICT =
