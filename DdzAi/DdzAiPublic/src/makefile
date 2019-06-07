
BINARY = ./libmsg.a
SVROBJS = MsgDefine.o

CC = cc
cpp = g++

# Debug
C_ARGS = -O3 -g -I. -D_LDEBUG_ -D_LINUX -D_POSIX_MT_

all:$(BINARY)

$(BINARY) : $(SVROBJS)
	rm -f $@
	ar -ruv  ${BINARY} $?
	ranlib ${BINARY}

MsgDefine.o : MsgDefine.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ MsgDefine.cpp
		
clean:
	rm -f $(SVROBJS) $(BINARY)	
