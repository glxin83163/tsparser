#ifndef __TS_PACKET_PARSER_LOG_H__
#define __TS_PACKET_PARSER_LOG_H__

void Parser_SetDisplayTagInfo(unsigned int type);
int Parser_TsPacketLog(unsigned int type,const char  *format, ...);


#endif