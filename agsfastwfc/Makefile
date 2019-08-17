UNAME := $(shell uname)

INCDIR = ../../ags/Engine ../../ags/Common ./fastwfc/include
LIBDIR = ./fastwfc/lib
CC = gcc
CXX = g++
CFLAGS = -fPIC -fvisibility=hidden -O2 -g -Wall -std=c++17
LIBS = -lm -lstdc++ 

ifeq ($(UNAME), Darwin)
TARGET = libagsfastwfc.dylib
CFLAGS += -DMAC_VERSION
else
TARGET = libagsfastwfc.so
CFLAGS += -DLINUX_VERSION
endif

CXXFLAGS = $(CFLAGS)

include Makefile-objs


CFLAGS   := $(addprefix -I,$(INCDIR)) $(CFLAGS)
CXXFLAGS := $(CFLAGS) $(CXXFLAGS)
ASFLAGS  := $(CFLAGS) $(ASFLAGS)


.PHONY: clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CC) -shared -dynamiclib -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)

%.o: %.c
	@echo $@
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	@echo $@
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	@rm -f $(TARGET) $(OBJS)
