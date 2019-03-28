include /env.mak

LIB_NAME=synospotlightutils
PROJ_NAME=$(LIB_NAME)

TOPSOURCE=/source
PROJ_DIR=$(TOPSOURCE)/$(PROJ_NAME)

REQUIRES=libxml-2.0 libsynocore
REQUIRES_PRIVATE=
LIBS_PRIVATE=

#DEFS=-D__STDC_FORMAT_MACROS -DSYNO_DEBUG
DEFS=-D__STDC_FORMAT_MACROS
INCS=$(shell pkg-config --cflags $(REQUIRES) $(REQUIRES_PRIVATE))
LIBS=$(shell pkg-config --libs $(REQUIRES) $(REQUIRES_PRIVATE)) $(LIBS_PRIVATE)
INCS_INNER=-I$(PROJ_DIR)/include -I$(PROJ_DIR)/include_int -I$(PROJ_DIR)/lib
LIBS_INNER=-L$(PROJ_DIR)/lib -lsynospotlightutils

CCFLAGS+=-Wall -Werror

DESCRIPTOR=$(PROJ_NAME).pc
LIB_DIR=$(PROJ_DIR)/lib

LIB_NAMES=$(LIB_NAME)

#so version
SHLIB_MAJOR=$(DSM_SHLIB_MAJOR)

CPPFLAGS+=$(DEFS) $(INCS_INNER) $(INCS)
CFLAGS+=$(CCFLAGS)
CXXFLAGS+=$(CCFLAGS)

#install
PKG_CONFIG_DIR=$(DESTDIR)/$(libdir)/pkgconfig
INCLUDE_DIR=$(PROJ_DIR)/include
INCS_PKG_DIR=$(includedir)/$(LIB_NAME)

INSTALL=/usr/bin/install
LINK=/bin/ln -fs
RMFILE=rm -f

SRCS:=$(shell echo *.c)
OBJS:=$(SRCS:%.c=%.o)

SUBDIR := $(patsubst %/Makefile,%,$(wildcard */Makefile))
.PHONY: $(SUBDIR)

.SUFFIXES: .o.c
