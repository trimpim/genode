SRC_DIR = src/drivers/rtc/am1805
include $(GENODE_DIR)/repos/base/recipes/src/content.inc

CONTENT = src/drivers/rtc/README src/drivers/rtc/rtc.h src/drivers/rtc/main.cc \
          src/drivers/rtc/target.inc

content: $(CONTENT)

$(CONTENT):
	$(mirror_from_rep_dir)
