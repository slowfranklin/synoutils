include ../../Makefile.inc

SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:%.cpp=%.o)
CCFLAGS+=-fPIC -DPIC

all: $(OBJS)

install:
	@[ -d $(DESTDIR)/$(INCS_PKG_DIR) ] || $(INSTALL) -d $(DESTDIR)/$(INCS_PKG_DIR)

remoteinstall:

clean:
	$(RMFILE) $(OBJS)
