#ifndef __TS_PACKET_PARSER_COMMON_H__
#define __TS_PACKET_PARSER_COMMON_H__

#define PACKET_188LEN	(188)
#define PACKET_204LEN	(204)
#define NULL_PID		(0x1FFF)
#define TS_HEAD_TAG		(0x47)

#define OFFSET_VALID			(0x80000000)
#define DISPLAY_VALID			(0x40000000)
#define GET_VALID				(0x20000000)
#define DELET_VALID				(0x10000000)
#define SAVE_VALID				(0x08000000)
#define PAT_VALID				(0x00000001)
#define PMT_VALID				(0x00000002)
#define CAT_VALID				(0x00000004)
#define NIT_ACTUAL_VALID		(0x00000008)
#define NIT_OTHER_VALID			(0x00000010)
#define BAT_VALID				(0x00000020)
#define SDT_ACTUAL_VALID		(0x00000040)
#define SDT_OTHER_VALID			(0x00000080)
#define EIT_ACTUAL_VALID		(0x00000100)
#define EIT_ACTUAL_SCH_VALID	(0x00000200)
#define TOT_VALID				(0x00000400)
#define TDT_VALID				(0x00000800)


#define PAT_PID					(0x00)
#define CAT_PID					(0x01)
#define NIT_PID					(0x10)
#define SDT_BAT_PID				(0x11)
#define EIT_PID					(0x12)
#define TDT_TOT_PID				(0x14)

#define MAXLEN_TS_FILENAME		(256)
#define MAX_TS_PIDNUMBER		(256)
#define MAXBUFFER_LOG			(256)
#define DEFAULT_DISPLAYPSI		(0x4000056F)
#define MAX_SECTION_LENGTH		(4096)
#define MAX_CFGFILE_LENGTH		(0x100000)





#endif
