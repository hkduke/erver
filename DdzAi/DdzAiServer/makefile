
# Created by Zhang Hui in 2009-3-19

DEBUG = -g
BINARY = ./DdzAiServer
SVROBJS = Communication2.o Thread.o ConfigIniFile.o FMsgQueue.o DdzAiServer.o DdzAiMain.o AiThread.o  Communication2_Thread.o SwitchMsgHead.o TcpConnection.o TcpSocket.o ddz_move.o ddz_split_node.o ddz_state.o mcts_node.o random_engine.o stgy_initiative_play_card.o stgy_passive_play_card.o stgy_split_card.o tinymt32.o

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
	$(cpp)	$(C_ARGS) -o $@ $(SVROBJS) -lpthread -lcrypt -L../DdzAiPublic/src/ -lmsg
	
DdzAiServer.o : DdzAiServer.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ DdzAiServer.cpp
	
DdzAiMain.o : DdzAiMain.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ DdzAiMain.cpp

ConfigIniFile.o : ConfigIniFile.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ ConfigIniFile.cpp

Thread.o : Thread.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ Thread.cpp

FMsgQueue.o : FMsgQueue.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ FMsgQueue.cpp

AiThread.o : AiThread.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ AiThread.cpp

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

ddz_move.o	: ./ai/ddz_move.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ ./ai/ddz_move.cpp	

ddz_split_node.o : ./ai/ddz_split_node.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ ./ai/ddz_split_node.cpp	

ddz_state.o : ./ai/ddz_state.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ ./ai/ddz_state.cpp	

mcts_node.o : ./ai/mcts_node.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ ./ai/mcts_node.cpp	

random_engine.o : ./ai/random_engine.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ ./ai/random_engine.cpp	

stgy_initiative_play_card.o : ./ai/stgy_initiative_play_card.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ ./ai/stgy_initiative_play_card.cpp	

stgy_passive_play_card.o : ./ai/stgy_passive_play_card.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ ./ai/stgy_passive_play_card.cpp	

stgy_split_card.o : ./ai/stgy_split_card.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ ./ai/stgy_split_card.cpp	

tinymt32.o : ./ai/tinymt32.cpp
	rm -f $@
	$(cpp) $(C_ARGS) -c -o $@ ./ai/tinymt32.cpp	

clean:
	rm -f $(SVROBJS) $(BINARY)

install:
	



	
