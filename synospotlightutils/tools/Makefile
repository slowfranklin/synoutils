include /env.mak
include ../Makefile.inc

SRCS=$(wildcard *.cpp)
TOOLS=$(SRCS:%.cpp=%)
VERSION_FILE=$(TOPSOURCE)/synoversion/VERSION
HAVE_VERSION=$(wildcard $(VERSION_FILE))
ifneq ($(strip $(HAVE_VERSION)),)
BUILD_NUM=$(shell grep -s "^buildnumber=" $(VERSION_FILE) | cut -d'"' -f 2)
else
BUILD_NUM=$(shell date "+%Y%m%d")
endif
DEFS+=-DBUILD_NUMBER=$(BUILD_NUM)
CPPFLAGS+=$(DEFS)
LDLIBS=$(LIBS_INNER) $(LIBS)

all: $(TOOLS)

$(TOOLS): $(SRCS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $@.cpp -o $@ $(LDLIBS)

install:

remoteinstall:

clean:
	rm -rf $(TOOLS) *.o
