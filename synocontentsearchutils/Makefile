include Makefile.inc

.PHONY: all clean remoteinstall $(SUBDIR) $(DESCRIPTOR)

all: $(SUBDIR)

tools: lib

daemon: lib

remoteinstall: $(SUBDIR)

$(SUBDIR):
	@echo "===>" $@
	$(MAKE) -C $@ $(MAKECMDGOALS)
	@echo "<===" $@

$(PROJ_NAME).pc: $(PROJ_NAME).pc.in Makefile.inc
	$(RMFILE) -f $@
	@cat $< | sed \
		-e "s%@CONFIG_NAME@%$(PROJ_NAME)%g" \
		-e "s%@PREFIX@%$(prefix)%g" \
		-e "s%@DEFINES@%$(DEFS)%g" \
		-e "s%@INCLUDES@%$(INCS_PKG)%g" \
		-e "s%@LIBS@%$(LDADD_PKG)%g" \
		> $@

install: $(SUBDIR) $(DESCRIPTOR)
	@[ -d $(PKG_CONFIG_DIR) ] || install -d $(PKG_CONFIG_DIR)
	@set -e ;\
	for i in $(DESCRIPTOR); do \
		install -c -m 644 $$i $(PKG_CONFIG_DIR); \
	done

clean: $(SUBDIR)
	$(RMFILE) $(DESCRIPTOR)
