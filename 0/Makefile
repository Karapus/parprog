CXX=mpic++
CXXFLAGS+=-Wall -Wextra -std=c++20

PROGSCC	::= $(shell ls *.cc)
PROGS	::= $(PROGS:.cc=)

list: ;echo $(PROGS)

$(PROGS): $@.cc

all: $(PROGS)
