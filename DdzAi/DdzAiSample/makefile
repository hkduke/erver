
# Created by Zhang Hui in 2009-3-19

DEBUG = -g
BINARY = ./DdzAiSample
SVROBJS = DdzAiSample.o

CC = cc
# 32bit
#cpp = g++
# 64 bit
cpp = g++ -lz

# debug
C_ARGS = -O3 -g -I. -I../include -D_LDEBUG_ -D_LINUX  -D_POSIX_MT_
# release
# C_ARGS = -O3 -I. -I../include -D_LINUX -D_POSIX_MT_

all:$(BINARY)

$(BINARY) : $(SVROBJS)
	rm -f $@
	$(cpp)	$(C_ARGS) -o $@ $(SVROBJS) -lpthread -lcrypt -L../DdzAiLib/ -lddzai -L../DdzAiPublic/src/ -lmsg
	
DdzAiSample.o : DdzAiSample.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ DdzAiSample.cpp
	
clean:
	rm -f $(SVROBJS) $(BINARY)

install:
	



	
