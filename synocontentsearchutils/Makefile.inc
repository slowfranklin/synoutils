include /env.mak

LIB_NAME=synocontentsearchutils
PROJ_NAME=$(LIB_NAME)

TOPSOURCE=/source
PROJ_DIR=$(TOPSOURCE)/$(PROJ_NAME)
CORE_INCS=$(shell pkg-config --cflags-only-I libsynocore)
SDK_INCS=$(shell pkg-config --cflags-only-I libsynosdk)
FILEOP_INCS=$(shell pkg-config --cflags-only-I libsynofileop)
JSONCPP_INCS=$(shell pkg-config --cflags jsoncpp)
SYNODAEMON_INCS=$(shell pkg-config --cflags libsynodaemon)

INCLUDE_DIR=$(PROJ_DIR)/include
INCS_OTHER=-I$(PROJ_DIR)/lib
INCS=$(CORE_INCS) $(SDK_INCS) $(FILEOP_INCS) $(JSONCPP_INCS) $(SYNODAEMON_INCS) -I$(INCLUDE_DIR) $(INCS_OTHER)
INCS_PKG_DIR=$(includedir)/$(LIB_NAME)

#CCFLAGS+=-Wall -Werror -D__STDC_FORMAT_MACROS -DSYNO_DEBUG
CCFLAGS+=-Wall -Werror -D__STDC_FORMAT_MACROS
PKG_CONFIG_DIR=$(DESTDIR)/$(libdir)/pkgconfig

DESCRIPTOR=$(PROJ_NAME).pc
LIB_DIR=$(PROJ_DIR)/lib
LDADD=-L$(LIB_DIR) \
	$(shell pkg-config --libs libsynosdk) \
	$(shell pkg-config --libs libsynocore) \
	$(shell pkg-config --libs libsynofileop) \
	$(shell pkg-config --libs jsoncpp) \
	$(shell pkg-config --libs libsynodaemon) \
	-lstdc++

LIB_NAMES=$(LIB_NAME)

#synocontentsearchutils.pc
INCS_PKG=
LDADD_PKG=

#so version
SHLIB_MAJOR=$(DSM_SHLIB_MAJOR)

CFLAGS+=$(CCFLAGS) $(INCS)
LDFLAGS+=$(LDADD)

ifneq ($(PLATFORM_FAMILY),$(filter $(PLATFORM_FAMILY),SYNOPLAT_F_ARMV5))
	CXXFLAGS += -std=c++11
else
	CXXFLAGS += -std=c++0x
endif

MAKE=make
INSTALL=/usr/bin/install
LINK=/bin/ln -fs
RMFILE=rm -f

SRCS:=$(shell echo *.c)
OBJS:=$(SRCS:%.c=%.o)

SUBDIR := $(patsubst %/Makefile,%,$(wildcard */Makefile))
.PHONY: $(SUBDIR)

.SUFFIXES: .o.c
.c.o:
	$(CC) $(CFLAGS) -fPIC -DPIC -c -o $@ $<
.cpp.o:
	$(CXX) $(CFLAGS) $(CXXFLAGS) -fPIC -DPIC -c -o $@ $<
