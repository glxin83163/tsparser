CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -I ../inc

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $<

OBJ = TsPacket_Parser.o TsPacket_ParserLog.o TsPacket_ParserCfg.o TsPacket_DescriptorParser.o TsPacket_SectionParser.o

all: $(OBJ)

clean:
	rm *.o || true
