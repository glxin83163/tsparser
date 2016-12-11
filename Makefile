CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -I ./inc

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $<

DIRS = src
OBJ = main.o
SRCOBJ = $(DIRS)/TsPacket_Parser.o $(DIRS)/TsPacket_ParserLog.o $(DIRS)/TsPacket_ParserCfg.o $(DIRS)/TsPacket_SectionParser.o $(DIRS)/TsPacket_DescriptorParser.o
LINKOBJ =  $(SRCOBJ) $(OBJ)

all: $(OBJ) compile
	$(CC) $(CFLAGS) -o tsparser $(LINKOBJ) $(LDFLAGS)
	
compile:
	for i in $(DIRS); do make -C $$i; done

clean: clean-all
	rm *.o tsparser || true
	
clean-all:
	for i in $(DIRS); do make -C $$i clean; done
