MIRROR_FROM_REP_DIR := src/test/openssl

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/openssl)

content: src/lib/openssl

src/lib/openssl:
	mkdir -p $@/crypto/evp
	cp -r $(REP_DIR)/src/lib/openssl/spec $@/
	cp -r $(PORT_DIR)/src/lib/openssl/apps $@/
	cp -r $(PORT_DIR)/src/lib/openssl/test $@/
	cp -r $(PORT_DIR)/src/lib/openssl/include $@/
	cp -r $(PORT_DIR)/src/lib/openssl/crypto/evp/*.h $@/crypto/evp/

content: LICENSE

LICENSE:
	cp $(PORT_DIR)/src/lib/openssl/LICENSE.txt $@
