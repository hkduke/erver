
BINARY = ./libddzai.a
SVROBJS =  Communication2.o ConfigIniFile.o Communication2_Thread.o SwitchMsgHead.o TcpConnection.o TcpSocket.o FMsgQueue.o DdzAiInterface.o stgy_split_card.o ddz_move.o

CC = cc
cpp = g++

# Debug
C_ARGS = -std=c++11 -O3 -g -I. -D_LDEBUG_ -D_LINUX -D_POSIX_MT_

all:$(BINARY)

$(BINARY) : $(SVROBJS)
	rm -f $@
	ar -ruv  ${BINARY} $?
	ranlib ${BINARY}

ConfigIniFile.o : ConfigIniFile.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ ConfigIniFile.cpp

Communication2.o : Communication2.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ Communication2.cpp

Communication2_Thread.o	: Communication2_Thread.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ Communication2_Thread.cpp	

SwitchMsgHead.o	: SwitchMsgHead.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ SwitchMsgHead.cpp	

TcpConnection.o	: TcpConnection.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ TcpConnection.cpp	

TcpSocket.o	: TcpSocket.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ TcpSocket.cpp	

FMsgQueue.o	: FMsgQueue.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ FMsgQueue.cpp	

DdzAiInterface.o : DdzAiInterface.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ DdzAiInterface.cpp	
		
stgy_split_card.o : stgy_split_card.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ stgy_split_card.cpp	

ddz_move.o : ddz_move.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ ddz_move.cpp	

clean:
	rm -f $(SVROBJS) $(BINARY)	



	



	
