HOST_INC_DIR += $(dir $(call select_from_repositories,src/lib/syscall/linux_syscalls.h))
IMPORT_DIR   := $(dir $(call select_from_repositories,lib/import/import-syscall-linux.mk))

ifeq ($(filter-out $(SPECS),x86),)
	include $(IMPORT_DIR)/spec/x86/import-syscall-linux.mk
endif

ifeq ($(filter-out $(SPECS),arm_v7a),)
	include $(IMPORT_DIR)/spec/arm_v7a/import-syscall-linux.mk
endif
