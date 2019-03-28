include ../Makefile.inc

.PHONY: all clean remoteinstall $(SUBDIR)

LIBRARIES=$(LIB_NAMES:%=lib%.a)
LIBRARIES+=$(LIB_NAMES:%=lib%.so.$(SHLIB_MAJOR))

SHARED_LIBS=$(LIB_NAMES:%=lib%.so)

LIB_DIRS=$(patsubst %/Makefile,%,$(wildcard */Makefile))
LDLIBS=$(LIBS)

all: $(LIBRARIES)

$(SUBDIR):
	@echo "===>" $@
	$(MAKE) -C $@ $(MAKECMDGOALS);
	@echo "<===" $@

lib$(LIB_NAME).a: $(LIB_DIRS)
	$(AR) $(ARFLAGS) $@ \
		$(foreach d, $^, $(shell echo $(d)/*.o))
	$(RANLIB) $@

lib$(LIB_NAME).so.$(SHLIB_MAJOR): lib$(LIB_NAME).a
	rm -f $@ lib$(LIB_NAME).so
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -shared -Wl,-soname,$@ -o $@ \
		$(foreach d, $(LIB_DIRS) , $(shell echo $(d)/*.o)) $(LDLIBS)
	ln -sf $@ lib$(LIB_NAME).so

install: $(SUBDIR)
	[ -d $(DESTDIR)/$(libdir) ] || install -d $(DESTDIR)/$(libdir)
	echo $(SHARED_LIBS)
	set -e ;\
	install -c -m 644 $(SHARED_LIBS).$(SHLIB_MAJOR) $(DESTDIR)/$(libdir)/$(SHARED_LIBS).$(SHLIB_MAJOR) ; \
	set -e ;\
	cd $(DESTDIR)/$(libdir)/;\
	rm -f $(SHARED_LIBS); \
	ln -s $(SHARED_LIBS).$(SHLIB_MAJOR) $(SHARED_LIBS);

remoteinstall: $(SUBDIR)
	[ -d $(DESTDIR)/$(libdir) ] || install -d $(DESTDIR)/$(libdir)
	echo $(SHARED_LIBS)
	set -e ;\
	install -c -m 644 $(SHARED_LIBS).$(SHLIB_MAJOR) $(DESTDIR)/$(libdir)/$(SHARED_LIBS).$(SHLIB_MAJOR) ; \
	set -e ;\
	cd $(DESTDIR)/$(libdir)/;\
	rm -f $(SHARED_LIBS); \
	ln -s $(SHARED_LIBS).$(SHLIB_MAJOR) $(SHARED_LIBS);
ifndef NOSTRIP
	$(STRIP) $(DESTDIR)/$(libdir)/$(SHARED_LIBS);
endif

clean: $(SUBDIR)
	$(RMFILE) -f $(LIBRARIES) $(SHARED_LIBS)
