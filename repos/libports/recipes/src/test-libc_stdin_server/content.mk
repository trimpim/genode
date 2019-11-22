SRC_DIR = src/test/libc_stdin_blocking_read/server

include $(GENODE_DIR)/repos/base/recipes/src/content.inc

GEMS_DIR             := $(GENODE_DIR)/repos/gems
MIRROR_FROM_GEMS_DIR := include/gems/magic_ring_buffer.h

content: $(MIRROR_FROM_GEMS_DIR)

$(MIRROR_FROM_GEMS_DIR):
	mkdir -p $(dir $@)
	cp $(GEMS_DIR)/$@ $@
