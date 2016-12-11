#ifndef __TS_PACKET_PARSER_H__
#define __TS_PACKET_PARSER_H__

#include "TsPacket_Parser_Common.h"

typedef struct {
    char filename[MAXLEN_TS_FILENAME];             
    unsigned char ts_packet_len;
	unsigned short pid[MAX_TS_PIDNUMBER];
    unsigned int fstart_offset;
    unsigned int operate_type;
    unsigned int reserved;               /*reserved 0*/
    unsigned int unused;               /*reserved 0*/
}TsPacket_Operate_S;

typedef struct
{
	unsigned char section_number;
	unsigned char version_number;
	unsigned char table_id;
	unsigned short len;
	unsigned int crc;
}section_headinfo_t;

typedef struct section_list
{
	unsigned char cc;
	unsigned char flag;
	unsigned char section_number;
	unsigned char version_number;
	unsigned char table_id;
	unsigned short pid;
	unsigned short len;
	unsigned short curlen;
	unsigned int crc;
	unsigned char secbuff[MAX_SECTION_LENGTH];
	struct section_list *next;
}section_list_t;


typedef struct
{
	unsigned short pid;
	unsigned int cc_count;
}ts_ccinfo_t;


int Parser_TsPacketLen(char* fname,unsigned char *tslen,unsigned int *start_offset);
int Parser_TsPacket(TsPacket_Operate_S info,char *outfile);

#endif