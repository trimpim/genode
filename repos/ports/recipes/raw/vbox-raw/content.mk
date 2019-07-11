content: init.config machine.vbox machine.vmdk

init.config:
	cp $(REP_DIR)/recipes/raw/vbox-raw/$@ $@

machine.vbox:
	cp $(REP_DIR)/recipes/raw/vbox-raw/$@ $@

machine.vmdk:
	cp $(REP_DIR)/recipes/raw/vbox-raw/$@ $@
