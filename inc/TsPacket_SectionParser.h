#ifndef __TS_PACKET_PARSER_SECTION_H__
#define __TS_PACKET_PARSER_SECTION_H__
#include "TsPacket_Parser_Common.h"


#define MAX_INPAT_PMT_NUM		(32)
#define MAX_CAT_DESCRIPTOS_NUM	(32)
#define MAX_CAT_DESCRIPTOS_DATA	(256)

typedef struct
{
	unsigned char table_id;
	unsigned char section_syntax_indicator;
	unsigned short section_length;
	unsigned short transport_stream_id;
	unsigned char version_number;
	unsigned char current_next_indicator;
	unsigned char section_number;
	unsigned char last_section_number;
	unsigned short program_number[MAX_INPAT_PMT_NUM];
	unsigned short pid_value[MAX_INPAT_PMT_NUM];
	unsigned int crc;
}pat_info_t;

typedef struct
{
	unsigned char table_id;
	unsigned char section_syntax_indicator;
	unsigned short section_length;
	unsigned short program_number;
	unsigned char version_number;
	unsigned char current_next_indicator;
	unsigned char section_number;
	unsigned char last_section_number;
	unsigned short pcr_pid;
	unsigned short program_info_length;
	unsigned int crc;
}pmt_info_t;

typedef struct
{
	unsigned char tag;
	unsigned char length;
	unsigned short ca_system_id;
	unsigned short ca_pid;
	unsigned char data[MAX_CAT_DESCRIPTOS_DATA];
}cat_descriptor_t;

typedef struct
{
	unsigned char table_id;
	unsigned char section_syntax_indicator;
	unsigned short section_length;
	unsigned char version_number;
	unsigned char current_next_indicator;
	unsigned char section_number;
	unsigned char last_section_number;
	cat_descriptor_t ca_descriptors[MAX_CAT_DESCRIPTOS_NUM];
	unsigned int crc;
}cat_info_t;

typedef struct
{
	unsigned char table_id;
	unsigned char section_syntax_indicator;
	unsigned short section_length;
	unsigned short network_id;
	unsigned char version_number;
	unsigned char current_next_indicator;
	unsigned char section_number;
	unsigned char last_section_number;
	unsigned short network_descriptors_length;
	unsigned short transport_stream_loop_length;
	unsigned int crc;
}nit_info_t;

typedef struct
{
	unsigned char table_id;
	unsigned char section_syntax_indicator;
	unsigned short section_length;
	unsigned char utc[5];
	unsigned short descriptor_loop_length;
	unsigned int crc;
}tot_info_t;


int Parser_SectionPAT(unsigned char *section,pat_info_t *info);
int Parser_SectionPMT(unsigned char *section,unsigned short pid,pmt_info_t *info);
int Parser_SectionCAT(unsigned char *section,cat_info_t *info);
int Parser_SectionNIT(unsigned char *section,nit_info_t *info);
int Parser_SectionSDT(unsigned char *section);
int Parser_SectionBAT(unsigned char *section);
int Parser_SectionEIT(unsigned char *section);
int Parser_SectionTOT(unsigned char *section,tot_info_t *info);
int Parser_SectionTDT(unsigned char *section,tot_info_t *info);

#endif

