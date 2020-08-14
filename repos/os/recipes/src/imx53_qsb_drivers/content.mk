include $(GENODE_DIR)/repos/base/recipes/src/content.inc

content: src/drivers include/gpio

include/gpio:
	mkdir -p include
	cp -r $(REP_DIR)/include/gpio $@

src/drivers:
	mkdir -p $@/framebuffer $@/gpio/spec
	cp -r $(REP_DIR)/src/drivers/gpio/spec/imx $@/gpio/spec/
	cp -r $(REP_DIR)/src/drivers/gpio/spec/imx53 $@/gpio/spec/
	cp -r $(REP_DIR)/src/drivers/framebuffer/imx53 $@/framebuffer/
	cp -r $(REP_DIR)/include/spec/imx53/imx_framebuffer_session $@/framebuffer/imx53/
