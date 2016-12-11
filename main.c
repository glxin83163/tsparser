#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#include "TsPacket_Parser.h"
#include "TsPacket_ParserCfg.h"

void usage(char* binary_name)
{
	printf("usage: %s [s] -i filename\n",binary_name);
	printf("switches:\n");
	printf("  -i filename - opens \"filename\"\n");
	printf("  -o outfile  - writes to \"outfile\"\n");
	printf("  -c cfgfile  - config file \n");
	printf("  -d display ts Parser info!\n");
	printf("  -p psi_type  - writes psipid info to \"outfile\"\n");
	printf("  psi_type : all-default(0)|pat(1)|pmt(2)|cat(4)|nit(8)\n");
	printf("             bat(32)|sdt(64)|eit(256)|tot(1024)|tdt(2048)\n");
	printf("\n");
}
int main(int argc, char* argv[]) 
{
	char* filename = NULL;
	char* outfile = NULL;
	char* cfgfile = NULL;
	
	int c = 0;
	int ret = 0;
	unsigned int startoffset = 0;
	unsigned int psi_type = 0;
	unsigned char packetlen = PACKET_188LEN;
	TsPacket_Operate_S info;

	info.operate_type = 0;
	while ((c = getopt(argc,argv,"o:i:dp:g:c:")) != -1){
		switch (c){
			case 'i':
				filename = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'c':
				cfgfile = optarg;
				break;	
			case 'd':
				psi_type |= DISPLAY_VALID;
				break;	
			case 'p':
				if(0 == atol(optarg))
				{
					psi_type |= DEFAULT_DISPLAYPSI;
				}
				else
				{
					psi_type |= atol(optarg);
				}
				
				break;
			//case 'r':
			//	info.operate_type |= DELET_VALID;
			//	break;
			default:
				printf("\n");
				usage(argv[0]);
				abort();
		}
	}
	if(filename == NULL)
	{
		usage(argv[0]);
		return 1;
	}
	memset((unsigned char*)(info.pid),0xFF,MAX_TS_PIDNUMBER*2);
	
	psi_type &= DEFAULT_DISPLAYPSI;
	info.operate_type |= psi_type;
	Parser_SetDisplayTagInfo(info.operate_type);
	if(NULL != cfgfile)
	{
		ret = Parser_TsPacketCfg(cfgfile,&info);
		if(0 != ret)
		{
			printf("Parser_TsPacketCfg error !\n");
		}
	}
	if(DISPLAY_VALID != (psi_type&DEFAULT_DISPLAYPSI))
	{
		info.operate_type &= ~DEFAULT_DISPLAYPSI;
		info.operate_type |= psi_type;
	}
	Parser_SetDisplayTagInfo(info.operate_type);

	printf("set in filename to: %s\n",filename);
	printf("set out outfile to: %s\n",outfile);
	if(strlen(filename) > MAXLEN_TS_FILENAME)
	{
		printf("set in filename too len > 256Bytes to: %s\n",filename);
		return 0;
	}
	memset(info.filename,0,MAXLEN_TS_FILENAME);
	memcpy(info.filename,filename,strlen(filename));
	ret = Parser_TsPacketLen(info.filename,&packetlen,&startoffset);
	if(ret<0)
	{
		printf("%s %d Parser_TsPacketLen error!\n",__FUNCTION__,__LINE__);
		return 0;
	}
	printf("%s %d tsfile packetlen = %d startoffset = %d \n",__FUNCTION__,__LINE__,packetlen,startoffset);
	if(0 != startoffset)
	{
		info.operate_type |= OFFSET_VALID;
	}
	info.operate_type |= psi_type;
	info.fstart_offset = startoffset;
	info.ts_packet_len = packetlen;
	ret = Parser_TsPacket(info,outfile);
	if(ret<0)
	{
		printf("%s %d Parser_TsPacket error!\n",__FUNCTION__,__LINE__);
		return 0;
	}
	return 0;
}