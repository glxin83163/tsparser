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
#include "TsPacket_ParserCfg.h"
#define MAX_STRBUFF_LEN		(512)
#define TSPACKET_OPERATE	"TSPACKET_OPERATE"
#define TSPACKET_PID		"TSPACKET_PID"
#define LOGMODULE			"LOGMODULE"

static int Parser_TsPacketGet(char* filedata,char *label,TsPacket_Operate_S *info)
{
	char *ptr = NULL;
	char* line_end = NULL;
	char* equal = NULL;
	char strbuff[MAX_STRBUFF_LEN]={0};
	char strpid[8]={0};
	int str_len = 0;
	if((NULL == filedata) || (NULL == info) || (NULL == label))
	{
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d param is null! filedata = %d info = %d label = %d\n",__FUNCTION__,__LINE__,filedata,info,label);
		return -1;
	}

	ptr = strstr(filedata,label);
	if(NULL == ptr)
	{
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d no find %s in cfgfile\n",__FUNCTION__,__LINE__,label);
		return -1;
	}

	line_end = strstr(ptr, "\n");
	equal = strstr(ptr, "=");
	str_len = line_end - equal -1;
	if((str_len > MAX_STRBUFF_LEN -1) || (str_len < 0))
	{
		str_len = MAX_STRBUFF_LEN -1;
	}
	
	memcpy(strbuff, equal + 1, str_len);
	Parser_TsPacketLog(DISPLAY_VALID,"%s %d strbuff : %s in cfgfile\n",__FUNCTION__,__LINE__,strbuff);
	if(memcmp(label, TSPACKET_OPERATE, strlen(TSPACKET_OPERATE))==0)
	{
		int value = 0;
		value = atol(strbuff);
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d TSPACKET_OPERATE= %d in cfgfile\n",__FUNCTION__,__LINE__,value);
		if(value)
		{
			info->operate_type |= SAVE_VALID;
		}
		else
		{
			info->operate_type |= DELET_VALID;
		}
	}
	else if(memcmp(label, LOGMODULE, strlen(TSPACKET_OPERATE))==0)
	{
		int value = 0;
		value = atol(strbuff);
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d LOGMODULE= %d in cfgfile\n",__FUNCTION__,__LINE__,value);
		if(value)
		{
			info->operate_type |= value;
		}
		else
		{
			info->operate_type |= DEFAULT_DISPLAYPSI;
		}
	}
	else if(memcmp(label, TSPACKET_PID, strlen(TSPACKET_PID))==0)
	{
		int i = 0;
		strbuff[str_len-1] = ',';
		while(ptr = strstr(strbuff,","))
		{
			str_len = ptr - strbuff;
			if((str_len > 4) || (str_len < 0))
			{
				Parser_TsPacketLog(DISPLAY_VALID,"%s %d pid is error : %s in cfgfile str_len = %d\n",__FUNCTION__,__LINE__,strbuff,str_len);
				break;
			}
			memcpy(strpid,strbuff,str_len);
			memmove(strbuff,ptr+1,strlen(strbuff)-str_len+1);
			for(i = 0;i<MAX_TS_PIDNUMBER;i++)
			{
				if(info->pid[i] == 0xFFFF)
				{
					info->pid[i] = atol(strpid);
					Parser_TsPacketLog(DISPLAY_VALID,"%s %d pid is %d in cfgfile\n",__FUNCTION__,__LINE__,info->pid[i]);
					break;
				}
			}
		}
	}
	else
	{
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d no support %s in cfgfile\n",__FUNCTION__,__LINE__,label);
	}
	return 0;
}

int Parser_TsPacketCfg(char* fname,TsPacket_Operate_S *info)
{
	FILE *fp = NULL;
	unsigned int filelen = 0,rlen = 0;
	char *cfgdata = NULL;
	
	if((NULL == fname) || (NULL == info))
	{
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d param is null! fname = %d info = %d\n",__FUNCTION__,__LINE__,fname,info);
		return -1;
	}
	fp = fopen(fname,"rb+");
	if(NULL == fp)
	{
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d fopen %s error!\n",__FUNCTION__,__LINE__,fname);
		return -1;
	}
	fseek(fp,0,SEEK_END);
	filelen = ftell(fp);
	if(filelen > MAX_CFGFILE_LENGTH)
	{
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d cfgfilelen = %d is too big !\n",__FUNCTION__,__LINE__,filelen);
		return -1;
	}
	fseek(fp,0,SEEK_SET);
	cfgdata = (char*)malloc(filelen+1);
	if(NULL == cfgdata)
	{
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d filelen = %d malloc is null error !\n",__FUNCTION__,__LINE__,filelen);
		return -1;
	}
	memset(cfgdata,0,filelen+1);
	rlen = fread(cfgdata,1,filelen,fp);
	if(rlen != filelen)
	{
		Parser_TsPacketLog(DISPLAY_VALID,"%s %d error:filelen = %d rlen = %d !\n",__FUNCTION__,__LINE__,filelen,rlen);
		free(cfgdata);
		return -1;
	}
	Parser_TsPacketGet(cfgdata,TSPACKET_OPERATE,info);
	Parser_TsPacketGet(cfgdata,TSPACKET_PID,info);
	Parser_TsPacketGet(cfgdata,LOGMODULE,info);
	free(cfgdata);
	return 0;
}
