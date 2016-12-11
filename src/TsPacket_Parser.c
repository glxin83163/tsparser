#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <pthread.h>
#include <ctype.h>
#include <semaphore.h>
#include <sys/mman.h>        /* mmap */
#include <sys/stat.h>        /* For mode constants */
#include <unistd.h>
#include <fcntl.h>           /* For O_* constants */
#include <time.h>

#include "TsPacket_ParserLog.h"
#include "TsPacket_Parser.h"
#include "TsPacket_SectionParser.h"


static unsigned int pat_packet_count = 0;
static unsigned int pmt_packet_count = 0;
static unsigned int cat_packet_count = 0;
static unsigned int nit_packet_count = 0;
static unsigned int eit_packet_count = 0;
static unsigned int tot_tdt_packet_count = 0;
static unsigned int sdt_bat_packet_count = 0;

static unsigned int gSection_count = 0;
static ts_ccinfo_t gTsPacketCCError[64];
static unsigned int curlistcount = 0;
static section_list_t *gSection_info = NULL;


static void Parser_TsPacketResetSectionCount(void)
{
	//Parser_TsPacketLog("%s %d TRACE!\n",__FUNCTION__,__LINE__);
	curlistcount = 0;
}

static int Parser_TsPacketRemoveSection(section_list_t *removelist)
{
	section_list_t *tmplist = NULL;
	section_list_t *curlist = NULL;
	
	if(NULL == gSection_info)
	{
		return -1;
	}
	
	if(gSection_info->pid == removelist->pid)
	{
		if(3 == gSection_info->flag)
		{
			tmplist = gSection_info->next;
			free(gSection_info);
			gSection_info = tmplist;
			gSection_count--;
			return 0;
		}
	}
	curlist = gSection_info->next;
	tmplist = gSection_info;
	while(curlist)
	{
		if(curlist->pid == removelist->pid)
		{
			if(3 == curlist->flag)
			{
				tmplist->next = curlist->next;
				free(curlist);
				gSection_count--;
				return 0;
			}
		}
		tmplist = curlist;
		curlist = curlist->next;
	}
	return -1;
}

static section_list_t *Parser_TsPacketFindSection(unsigned short pid)
{
	section_list_t *tmplist = NULL;
	static section_list_t *Curtmplist = NULL;
	
	if(0 == curlistcount)
	{
		tmplist = gSection_info;
	}
	else if(curlistcount < gSection_count )
	{
		tmplist = Curtmplist;
	}
	else
	{
		return NULL;
	}
	
	while(tmplist)
	{
		curlistcount++;
		if(pid == tmplist->pid)
		{
			break;
		}
		tmplist = tmplist->next;
	}
	if(NULL != tmplist)
	{
		Curtmplist = tmplist->next;
	}
	else
	{
		Curtmplist = NULL;
	}

	return tmplist;
}

static int Parser_TsPacketAddSection(section_list_t *seclist)
{
	section_list_t *tmplist = NULL;

	tmplist = gSection_info;
	
	while(tmplist)
	{
		if(NULL == tmplist->next)
		{
			break;
		}
		tmplist = tmplist->next;
	}
	if(NULL == tmplist)
	{
		gSection_info = seclist;
	}
	else
	{
		tmplist->next = seclist;
	}
	gSection_count++;
	return 0;
}

static int Parser_TsPacketCreateSection(char *buffer)
{
	unsigned char data_offset = 0;
	unsigned short pid = 0;
	int ret = -1;

	section_list_t *tmplist = NULL;
	
	if(NULL == buffer)
	{
		return -1;
	}
	pid = ((buffer[1] & 0x1F)<<8) | (buffer[2] & 0xFF);
	if(pid >= NULL_PID)
	{
		return -1;
	}
	if(0x40 == (0x40 & buffer[1]))
	{
		data_offset = 5 + buffer[4];
	}
	else
	{
		return -1;
	}
	
	tmplist = (section_list_t *)malloc(sizeof(section_list_t));
	if(NULL == tmplist)
	{
		return -1;
	}
	tmplist->pid = ((buffer[1] & 0x1F)<<8) | (buffer[2] & 0xFF);
	tmplist->table_id = buffer[data_offset];
	tmplist->cc = buffer[3] & 0x0F;
	memset(tmplist->secbuff,0x00,MAX_SECTION_LENGTH);
	tmplist->version_number = (buffer[data_offset+5] & 0x3E)>>1;
	tmplist->section_number = buffer[data_offset+6];
	tmplist->len = ((buffer[data_offset+1] & 0x0F) << 8) | (buffer[data_offset+2] & 0xFF);
	
	
	if(tmplist->len > (185-data_offset))
	{
		tmplist->flag = 0;
		tmplist->curlen = 188 - data_offset;
		tmplist->crc = 0;
		memcpy(tmplist->secbuff,buffer+data_offset,tmplist->curlen);
		ret = 0;
	}
	else
	{
		tmplist->flag = 1;
		tmplist->curlen = tmplist->len + 3;
		tmplist->crc = (buffer[data_offset+tmplist->len-1] <<24) | ((buffer[data_offset+tmplist->len] & 0xFF) << 16) 
					|((buffer[data_offset+tmplist->len+1] & 0xFF) << 8) | (buffer[data_offset+tmplist->len+2] & 0xFF);
		memcpy(tmplist->secbuff,buffer+data_offset,tmplist->curlen);
		ret = 1;
	}

	tmplist->next = NULL;
	Parser_TsPacketAddSection(tmplist);
	return ret;
}

static int Parser_TsPacketMakeSection(char *buffer)
{
	unsigned char data_offset = 0;
	unsigned char i = 0;
	unsigned char continuity_counter = 0;
	unsigned char payload_unit_start_indicator = 0;
	unsigned short pid = 0;
	int ret = -1;
	section_headinfo_t secheadinfo; 
	section_list_t *tmplist = NULL;
	
	if(NULL == buffer)
	{
		return -1;
	}
	pid = ((buffer[1] & 0x1F)<<8) | (buffer[2] & 0xFF);
	if(pid >= NULL_PID)
	{
		return -1;
	}
	memset((char*)&secheadinfo,0x00,sizeof(section_headinfo_t));
	continuity_counter = buffer[3] & 0x0F;
	if(0x40 == (0x40 & buffer[1]))
	{
		payload_unit_start_indicator = 1;
		data_offset = 5 + buffer[4];
	}
	else
	{
		data_offset = 4;
	}
	if(1 == payload_unit_start_indicator)
	{
		secheadinfo.table_id = buffer[data_offset];
		secheadinfo.len = ((buffer[data_offset+1] & 0x0F) << 8) | (buffer[data_offset+2] & 0xFF);
		secheadinfo.version_number = (buffer[data_offset+5] & 0x3E)>>1;
		secheadinfo.section_number = buffer[data_offset+6];
		if(secheadinfo.len <= (185-data_offset))
		{
			secheadinfo.crc = (buffer[data_offset+secheadinfo.len-1] <<24) | ((buffer[data_offset+secheadinfo.len] & 0xFF) << 16) 
					|((buffer[data_offset+secheadinfo.len+1] & 0xFF) << 8) | (buffer[data_offset+secheadinfo.len+2] & 0xFF);
		}
	}
	Parser_TsPacketResetSectionCount();
	do
	{
		tmplist = Parser_TsPacketFindSection(pid);
		if(NULL == tmplist)
		{
			ret = Parser_TsPacketCreateSection(buffer);
			return ret;
#if 0			
			if(1 != payload_unit_start_indicator)
			{
				return -1;
			}
			tmplist = (section_list_t *)malloc(sizeof(section_list_t));
			if(NULL == tmplist)
			{
				return -1;
			}
			tmplist->pid = pid;
			tmplist->table_id = secheadinfo.table_id;
			tmplist->cc = continuity_counter;
			memset(tmplist->secbuff,0x00,MAX_SECTION_LENGTH);
			tmplist->version_number = secheadinfo.version_number;
			tmplist->section_number = secheadinfo.section_number;
			tmplist->len = secheadinfo.len;
			if(tmplist->len > (185-data_offset))
			{
				tmplist->flag = 0;
				tmplist->curlen = 188 - data_offset;
				tmplist->crc = 0;
				memcpy(tmplist->secbuff,buffer+data_offset,tmplist->curlen);
				ret = 0;
			}
			else
			{
				tmplist->flag = 1;
				tmplist->curlen = tmplist->len + 3;
				tmplist->crc = (buffer[data_offset+tmplist->len-1] <<24) | ((buffer[data_offset+tmplist->len] & 0xFF) << 16) 
					|((buffer[data_offset+tmplist->len+1] & 0xFF) << 8) | (buffer[data_offset+tmplist->len+2] & 0xFF);
				memcpy(tmplist->secbuff,buffer+data_offset,tmplist->curlen);
				ret = 1;
			}

			tmplist->next = NULL;
			Parser_TsPacketAddSection(tmplist);
			return ret;
#endif			
		}
		else
		{
			if(payload_unit_start_indicator)
			{
				if((tmplist->section_number == secheadinfo.section_number)
					&& (tmplist->version_number == secheadinfo.version_number)
					&& (tmplist->table_id == secheadinfo.table_id)
					&& (tmplist->len == secheadinfo.len))
				{
					if(secheadinfo.len <= (185-data_offset))
					{
						if(tmplist->crc == secheadinfo.crc)
						{
							Parser_TsPacketResetSectionCount();
							return -1;
						}
					}
					else
					{
						for(i = 0;i<(185-data_offset);i++)
						{
							if(tmplist->secbuff[i] != buffer[i+data_offset])
							{
								break;
							}
						}
						if(i != tmplist->curlen)
						{
							Parser_TsPacketResetSectionCount();
							return -1;
						}
					}
				}
			}
			else
			{
				if(tmplist->flag == 0)
				{
					if((continuity_counter-1) == tmplist->cc)
					{
						tmplist->cc = continuity_counter;
						memcpy(tmplist->secbuff+tmplist->curlen,buffer+data_offset,188 - data_offset);
						tmplist->curlen += (188 - data_offset);
						if(tmplist->curlen >= tmplist->len)
						{
							tmplist->flag = 1;
							ret = 1;
						}
						else
						{
							ret = 0;
						}
						return ret;
					}
					else
					{
						for(i = 0;i<64;i++)
						{
							if(gTsPacketCCError[i].pid != 0xFFFF)
							{
								if(gTsPacketCCError[i].pid == pid)
								{
									gTsPacketCCError[i].cc_count++;
									break;
								}
								else
								{
									continue;
								}
							}
							else
							{
								gTsPacketCCError[i].pid = pid;
								gTsPacketCCError[i].cc_count++;
								break;
							}
						}
						tmplist->flag = 3;
						Parser_TsPacketRemoveSection(tmplist);
						Parser_TsPacketResetSectionCount();
						return -1;
					}
				}
			}
		}
	}
	while(tmplist);
		
	return 0;
}

static int Parser_TsPacketReleaseSection(unsigned short pid)
{
	section_list_t *tmplist = NULL;
	section_list_t *curtmplist = NULL;
	
	curtmplist = gSection_info;
	gSection_info = NULL;
	gSection_count = 0;
	while(curtmplist)
	{
		if(pid == curtmplist->pid)
		{
			tmplist = curtmplist->next;
			free(curtmplist);
			curtmplist = tmplist;
		}
		else
		{
			Parser_TsPacketAddSection(curtmplist);
			curtmplist = curtmplist->next;
		}
	}
	return 0;
}

static int Parser_TsPacketReleaseAllSection(void)
{
	section_list_t *tmplist = NULL;
	
	gSection_count = 0;
	tmplist = gSection_info;
	while(tmplist)
	{
		gSection_info = tmplist->next;
		free(tmplist);
		tmplist = gSection_info;
	}
	
	return 0;
}


static int TSParser_SectionPAT(pat_info_t *info)
{
	int ret = -1;
	section_list_t *tmplist = NULL;
	
	if(NULL == info)
	{
		return -1;
	}
	Parser_TsPacketResetSectionCount();

	do
	{
		tmplist = Parser_TsPacketFindSection(PAT_PID);
		if(tmplist)
		{
			if(1 == tmplist->flag)
			{
				//tmplist->flag = 2;
				ret = Parser_SectionPAT((unsigned char *)&(tmplist->secbuff[0]),info);
			}
		}
	}
	while(tmplist);
	return ret;
}

int TSParser_SectionPMT(unsigned short pid,pmt_info_t *info)
{
	int ret = -1;
	
	section_list_t *tmplist = NULL;
	
	if((NULL == info) || (pid >= NULL_PID))
	{
		return -1;
	}
	Parser_TsPacketResetSectionCount();

	do
	{
		tmplist = Parser_TsPacketFindSection(pid);
		if(tmplist)
		{
			if(1 == tmplist->flag)
			{
				ret = Parser_SectionPMT((unsigned char *)&(tmplist->secbuff[0]),pid,info);
			}
		}
	}
	while(tmplist);
	return ret;
}

int TSParser_SectionCAT(cat_info_t *info)
{
	int ret = -1;
	section_list_t *tmplist = NULL;	
	
	if(NULL == info)
	{
		return -1;
	}
	Parser_TsPacketResetSectionCount();
	do
	{
		tmplist = Parser_TsPacketFindSection(CAT_PID);
		if(tmplist)
		{
			if(1 == tmplist->flag)
			{
				//tmplist->flag = 2;
				ret = Parser_SectionCAT((unsigned char *)&(tmplist->secbuff[0]),info);
			}
		}
	}
	while(tmplist);
	return ret;
}

int TSParser_SectionNIT(nit_info_t *info)
{
	int ret = -1;
	section_list_t *tmplist = NULL;
	
	if(NULL == info)
	{
		return -1;
	}
	Parser_TsPacketResetSectionCount();
	do
	{
		tmplist = Parser_TsPacketFindSection(NIT_PID);
		if(tmplist)
		{
			if(1 == tmplist->flag)
			{
				ret = Parser_SectionNIT((unsigned char *)&(tmplist->secbuff[0]),info);
			}
		}
	}
	while(tmplist);
	return ret;
}

int TSParser_SectionSDT(void)
{
	int ret = -1;
	section_list_t *tmplist = NULL;
	
	Parser_TsPacketResetSectionCount();
	do
	{
		tmplist = Parser_TsPacketFindSection(SDT_BAT_PID);
		if(tmplist)
		{
			if( (0x42 != tmplist->secbuff[0]) && (0x46 != tmplist->secbuff[0]))
			{
				continue;
			}
			ret = Parser_SectionSDT((unsigned char *)&(tmplist->secbuff[0]));
		}
	}
	while(tmplist);
	return ret;
}

int TSParser_SectionBAT(void)
{
	int ret = -1;
	section_list_t *tmplist = NULL;
	
	Parser_TsPacketResetSectionCount();
	do
	{
		tmplist = Parser_TsPacketFindSection(SDT_BAT_PID);
		if(tmplist)
		{
			if(0x4A != tmplist->secbuff[0])
			{
				continue;
			}
			ret = Parser_SectionBAT((unsigned char *)&(tmplist->secbuff[0]));
		}
	}
	while(tmplist);
	return ret;
}

int TSParser_SectionEIT(void)
{
	int ret = -1;
	section_list_t *tmplist = NULL;
	
	Parser_TsPacketResetSectionCount();
	do
	{
		tmplist = Parser_TsPacketFindSection(EIT_PID);
		if(tmplist)
		{
			ret = Parser_SectionEIT((unsigned char *)&(tmplist->secbuff[0]));
		}
	}
	while(tmplist);
	return ret;
}

int TSParser_SectionTOT(tot_info_t *info)
{
	int ret = -1;
	section_list_t *tmplist = NULL;
	
	if(NULL == info)
	{
		return -1;
	}
	Parser_TsPacketResetSectionCount();

	do
	{
		tmplist = Parser_TsPacketFindSection(TDT_TOT_PID);
		if(tmplist)
		{
			if(1 == tmplist->flag)
			{
				if(0x73 == tmplist->secbuff[0])
				{
					ret = Parser_SectionTOT((unsigned char *)&(tmplist->secbuff[0]),info);
				}
				else if(0x70 == tmplist->secbuff[0])
				{
					ret = Parser_SectionTDT((unsigned char *)&(tmplist->secbuff[0]),info);
				}
			}
		}
	}
	while(tmplist);
	return ret;
}

static int Parser_IsPMTPid(unsigned short pid,pat_info_t info)
{
	int i = 0;
	for(i = 0;i<MAX_INPAT_PMT_NUM;i++)
	{
		if(pid == info.pid_value[i])
		{
			return 1;
		}
		if(info.pid_value[i] == 0xFFFF)
		{
			break;
		}
	}
	return -1;
}

int Parser_TsPacketLen(char* fname,unsigned char *tslen,unsigned int *start_offset)
{
	FILE *fp = NULL;
	int c = 0;
	unsigned int offset = 0;
	char tsdata[PACKET_204LEN] ={0};
	char data[4096] ={0};
	
	if((NULL == fname) || (NULL == tslen) || (NULL == start_offset))
	{
		printf("%s %d param is null!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	fp = fopen(fname,"rb+");
	if(NULL == fp)
	{
		printf("%s %d fopen error!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	
	while(1)
	{
		c=fread(tsdata,1,1,fp);
		if(tsdata[0]!= TS_HEAD_TAG)
		{
			offset++;
			continue;
		}
		memset(data,0,PACKET_188LEN);
		c=fread(data,PACKET_188LEN,1,fp);
		if(c!=1)
		{
			fclose(fp);
			printf("%s %d file read end!\n",__FUNCTION__,__LINE__);
			return -1;
		}
		if(tsdata[0] == data[PACKET_188LEN-1])
		{
			memset(data,0,PACKET_188LEN);
			c=fread(data,PACKET_188LEN,1,fp);
			if(c == 1)
			{
				if(tsdata[0] != data[PACKET_188LEN-1])
				{
					fseek(fp,offset+1,SEEK_SET);
					offset++;
					continue;
				}
			}
			printf("%s %d tsfile packet tag = %x\n",__FUNCTION__,__LINE__,tsdata[0]);
			printf("%s %d tsfile offset = %d\n",__FUNCTION__,__LINE__,offset);
			*tslen = PACKET_188LEN;
			*start_offset = offset;
			break;
		}
		
		printf("%s %d tsfile offset = %d\n",__FUNCTION__,__LINE__,offset);
		fseek(fp,offset+1,SEEK_SET);
		memset(data,0,PACKET_204LEN);
		c=fread(data,PACKET_204LEN,1,fp);
		if(c!=1)
		{
			fclose(fp);
			printf("%s %d file read end!\n",__FUNCTION__,__LINE__);
			return -1;
		}
		if(tsdata[0] == data[PACKET_204LEN-1])
		{
			memset(data,0,PACKET_204LEN);
			c=fread(data,PACKET_204LEN,1,fp);
			if(c == 1)
			{
				if(tsdata[0] != data[PACKET_204LEN-1])
				{
					fseek(fp,offset+1,SEEK_SET);
					offset++;
					continue;
				}
			}
			printf("%s %d tsfile packet tag = %x\n",__FUNCTION__,__LINE__,tsdata[0]);
			printf("%s %d tsfile offset = %d\n",__FUNCTION__,__LINE__,offset);
			*tslen = PACKET_204LEN;
			*start_offset = offset;
			break;
		}
		fseek(fp,offset+1,SEEK_SET);
		offset++;
		if(offset>1024*1024*10)
		{
			fclose(fp);
			printf("%s %d tsfile offset = %d\n",__FUNCTION__,__LINE__,offset);
			return -1;
		}
		
	}
	fclose(fp);
	return 0;
}

int Parser_TsPacket(TsPacket_Operate_S info,char *outfile)
{
	FILE *fp = NULL;
	FILE *of = NULL;
	unsigned short packet_pid = NULL_PID;
	int c = 0;
	int ret = 0;
	int i = 0;
	char data[4096] ={0}; 
	tot_info_t totinfo;
	nit_info_t nitinfo;
	cat_info_t catinfo;
	pat_info_t patinfo;
	pmt_info_t pmtinfo;
	
	memset((unsigned char*)(patinfo.pid_value),0xFF,64);
	for(i = 0;i<64;i++)
	{
		gTsPacketCCError[i].pid = 0xFFFF;
	}
	if((PACKET_204LEN != info.ts_packet_len) && (PACKET_188LEN != info.ts_packet_len))
	{
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d ts_packet_len is error : %d!\n",__FUNCTION__,__LINE__,info.ts_packet_len);
		return -1;
	}
	
	fp = fopen(info.filename,"rb+");
	if(NULL == fp)
	{
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d fopen %s error!\n",__FUNCTION__,__LINE__,info.filename);
		return -1;
	}
	if((NULL != outfile) && (strlen(outfile) < MAXLEN_TS_FILENAME))
	{
		memcpy(data,outfile,(strlen(outfile)));
	}
	else
	{
		memcpy(data,"gl_default_output.ts",(strlen("gl_default_output.ts")));
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d fopen %s info!\n",__FUNCTION__,__LINE__,data);
	}
	of = fopen(data,"wb+");
	if(NULL == of)
	{
		fclose(fp);
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d fopen %s error!\n",__FUNCTION__,__LINE__,data);
		return -1;
	}
	memset(data,0,4096);
	if(OFFSET_VALID == (info.operate_type & OFFSET_VALID))
	{
		fseek(fp,info.fstart_offset,SEEK_SET);
	}

	while(1)
	{
		c=fread(data,info.ts_packet_len,1,fp);
		if(c!=1)
		{
			Parser_TsPacketLog(DISPLAY_VALID,"%s %d file read end!\n",__FUNCTION__,__LINE__);
			break;
		}
		if(TS_HEAD_TAG != data[0])
		{
			Parser_TsPacketLog(DISPLAY_VALID,"%s %d tsfile format error %x!\n",__FUNCTION__,__LINE__,data[0]);
			break;
		}
		
		packet_pid = ((data[1] & 0x1F)<<8) | (data[2] & 0xFF);		
		ret = 0;
		if((PAT_PID == packet_pid) || (NIT_PID == packet_pid) || (CAT_PID == packet_pid)
					|| (SDT_BAT_PID == packet_pid) || (EIT_PID == packet_pid) || (TDT_TOT_PID == packet_pid))
		{
			ret = Parser_TsPacketMakeSection(data);			
		}
		else if(1 == Parser_IsPMTPid(packet_pid,patinfo))
		{
			ret = Parser_TsPacketMakeSection(data);	
			if(1 == ret)
			{
				pmt_packet_count++;
			}
		}
		
		if(SAVE_VALID == (info.operate_type & SAVE_VALID))
		{
			for(i = 0;i<MAX_TS_PIDNUMBER;i++)
			{
				if(info.pid[i] == 0xFFFF)
				{
					break;
				}
				if(info.pid[i] == packet_pid)
				{
					fwrite(data,1,info.ts_packet_len,of);
				}
			}
		}
		else if(DELET_VALID == (info.operate_type & DELET_VALID))
		{
			char deleteflag = 0;
			for(i = 0;i<MAX_TS_PIDNUMBER;i++)
			{
				if(info.pid[i] == 0xFFFF)
				{
					break;
				}
				if(info.pid[i] == packet_pid)
				{
					deleteflag = 1;
					break;
				}
			}
			if(0 == deleteflag)
			{
				fwrite(data,1,info.ts_packet_len,of);
			}
		}
		
		switch(packet_pid){
			case PAT_PID:
				if(1 == ret)
				{
					TSParser_SectionPAT(&patinfo);
					pat_packet_count++;
				}
				break;
			case CAT_PID:
				if(1 == ret)
				{
					cat_packet_count++;
				}
				break;
			case NIT_PID:
				if(1 == ret)
				{
					nit_packet_count++;
				}
				break;
			case SDT_BAT_PID:
				if(1 == ret)
				{
					sdt_bat_packet_count++;
				}
				break;
			case EIT_PID:
				if(1 == ret)
				{
					eit_packet_count++;
				}
				break;
			case TDT_TOT_PID:
				if(1 == ret)
				{
					tot_tdt_packet_count++;
				}
				break;
			default:
				break;
		}
	}
	TSParser_SectionPAT(&patinfo);
	TSParser_SectionCAT(&catinfo);
	TSParser_SectionNIT(&nitinfo);
	TSParser_SectionTOT(&totinfo);
	TSParser_SectionSDT();
	TSParser_SectionBAT();
	TSParser_SectionEIT();
	for(i = 0;i<MAX_INPAT_PMT_NUM;i++)
	{
		if(0xFFFF != patinfo.pid_value[i])
		{
			TSParser_SectionPMT(patinfo.pid_value[i],&pmtinfo);
		}
		else
		{
			break;
		}
	}
	
	Parser_TsPacketLog(DISPLAY_VALID,"List SectionCount = 0x%x\n",gSection_count);
	Parser_TsPacketLog(DISPLAY_VALID,"PAT PacketCount = 0x%x\n",pat_packet_count);
	Parser_TsPacketLog(DISPLAY_VALID,"PMT PacketCount = 0x%x\n",pmt_packet_count);
	Parser_TsPacketLog(DISPLAY_VALID,"CAT PacketCount = 0x%x\n",cat_packet_count);
	Parser_TsPacketLog(DISPLAY_VALID,"NIT PacketCount = 0x%x\n",nit_packet_count);
	Parser_TsPacketLog(DISPLAY_VALID,"EIT PacketCount = 0x%x\n",eit_packet_count);
	Parser_TsPacketLog(DISPLAY_VALID,"SDT and BAT PacketCount = 0x%x\n",sdt_bat_packet_count);
	Parser_TsPacketLog(DISPLAY_VALID,"TOT and TDT PacketCount = 0x%x\n",tot_tdt_packet_count);
	for(i = 0;i<64;i++)
	{
		if(gTsPacketCCError[i].pid != 0xFFFF)
		{
			Parser_TsPacketLog(DISPLAY_VALID,"Continuity Counter error pid = 0x%x\n",gTsPacketCCError[i].pid);
			Parser_TsPacketLog(DISPLAY_VALID,"Continuity Counter error counter = 0x%x\n",gTsPacketCCError[i].cc_count);
		}
		else
		{
			break;
		}
	}
	Parser_TsPacketReleaseAllSection();
	fflush(of);
	fclose(fp);
	fclose(of);
	return 0;
}