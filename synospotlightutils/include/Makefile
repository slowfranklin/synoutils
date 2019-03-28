include ../Makefile.inc


.PHONY: all clean remoteinstall

all:

install: $(HEADERDIR)
	@[ -d $(DESTDIR)/$(INCS_PKG_DIR) ] || ${INSTALL} -d $(DESTDIR)/$(INCS_PKG_DIR)
	${INSTALL} -D -m644 $(INCLUDE_DIR)/$(PROJ_NAME)/*.h $(DESTDIR)/$(INCS_PKG_DIR)
	[ -d $(DESTDIR)/$(includedir)/synodoc ] || $(INSTALL) -d $(DESTDIR)/$(includedir)/synodoc
	$(INSTALL) -D -m644 synodoc/* $(DESTDIR)/$(includedir)/synodoc

remoteinstall:

