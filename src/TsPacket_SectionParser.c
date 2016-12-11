#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "TsPacket_Parser_Common.h"
#include "TsPacket_ParserLog.h"
#include "TsPacket_SectionParser.h"
#include "TsPacket_DescriptorParser.h"




int Parser_SectionPAT(unsigned char *section,pat_info_t *info)
{
	unsigned int data_offset = 0;
	unsigned int i = 0;
	int ret = -1;

	if((NULL == info) || (NULL == section))
	{
		Parser_TsPacketLog(PAT_VALID,"PAT error: param is NULL !\n");
		return -1;
	}
		
	if( 0x00 != section[0])
	{
		Parser_TsPacketLog(PAT_VALID,"PAT table id is error : %x\n",section[0]);
		return -1;
	}

	memset((unsigned char*)(info->program_number),0,64);
	memset((unsigned char*)(info->pid_value),0xFF,64);
	
	info->table_id = section[0];
	info->section_syntax_indicator = (section[1] & 0x80) >> 7;
	info->section_length = ((section[1] & 0x0F) << 8) | (section[2] & 0xFF);
	info->transport_stream_id = ((section[3] & 0xFF) << 8) | (section[4] & 0xFF);
	info->version_number = (section[5] & 0x3E)>>1;
	info->current_next_indicator = section[5] & 0x01;
	info->section_number = section[6];
	info->last_section_number = section[7];
	data_offset = 8;
	
	for(i = 0;i<((info->section_length-5)/4)-1;i+=1)
	{
		info->program_number[i] = ((section[data_offset] & 0xFF) << 8) | (section[data_offset+1] & 0xFF);
		info->pid_value[i] = ((section[data_offset+2] & 0x1F) << 8) | (section[data_offset+3] & 0xFF);
		data_offset+=4;
	}
	
	info->crc = (section[info->section_length-1] <<24) | ((section[info->section_length] & 0xFF) << 16) 
		|((section[info->section_length+1] & 0xFF) << 8) | (section[info->section_length+2] & 0xFF);
	Parser_TsPacketLog(PAT_VALID,"****************PAT Info****************\n");
	Parser_TsPacketLog(PAT_VALID,"PAT table id = 0x%x\n",info->table_id);
	Parser_TsPacketLog(PAT_VALID,"PAT section_syntax_indicator = 0x%x\n",info->section_syntax_indicator);
	Parser_TsPacketLog(PAT_VALID,"PAT section_length = 0x%x\n",info->section_length);
	Parser_TsPacketLog(PAT_VALID,"PAT transport_stream_id = 0x%x\n",info->transport_stream_id);
	Parser_TsPacketLog(PAT_VALID,"PAT version_number = 0x%x\n",info->version_number);
	Parser_TsPacketLog(PAT_VALID,"PAT current_next_indicator = 0x%x\n",info->current_next_indicator);
	Parser_TsPacketLog(PAT_VALID,"PAT section_number = 0x%x\n",info->section_number);
	Parser_TsPacketLog(PAT_VALID,"PAT last_section_number = 0x%x\n",info->last_section_number);
	Parser_TsPacketLog(PAT_VALID,"PAT crc = 0x%08x\n",info->crc);
	i = 0;
	while((info->pid_value[i] != 0xFFFF) && (i < 32))
	{
		Parser_TsPacketLog(PAT_VALID,"PAT program_number = 0x%x\n",info->program_number[i]);
		Parser_TsPacketLog(PAT_VALID,"PAT pid_value = 0x%x\n",info->pid_value[i]);
		i++;
	}
	Parser_TsPacketLog(PAT_VALID,"****************************************\n");
	ret = 0;
	
	return ret;
}

int Parser_SectionPMT(unsigned char *section,unsigned short pid,pmt_info_t *info)
{
	unsigned short stream_descriptor_offset = 0;
	unsigned short len = 0;
	unsigned int data_offset = 0;
	unsigned int i = 0,j = 0;
	int descriptor_offset = 0;
	int ret = -1;
	
	if((NULL == info) || (pid >= NULL_PID) || (NULL == section))
	{
		Parser_TsPacketLog(PMT_VALID,"PMT error: param is NULL !\n");
		return -1;
	}

	if( 0x02 != section[0])
	{
		Parser_TsPacketLog(PMT_VALID,"PMT table id is error : %x\n",section[0]);
		return -1;
	}

	info->table_id = section[0];
	info->section_syntax_indicator = (section[1] & 0x80) >> 7;
	info->section_length = ((section[1] & 0x0F) << 8) | (section[2] & 0xFF);
	info->program_number = ((section[3] & 0xFF) << 8) | (section[4] & 0xFF);
	info->version_number = (section[5] & 0x3E)>>1;
	info->current_next_indicator = section[5] & 0x01;
	info->section_number = section[6];
	info->last_section_number = section[7];
	info->pcr_pid = ((section[8] & 0x1F) << 8) | (section[9] & 0xFF);
	info->program_info_length = ((section[10] & 0x0F) << 8) | (section[11] & 0xFF);
					
	info->crc = (section[info->section_length-1] <<24) | ((section[info->section_length] & 0xFF) << 16) 
		|((section[info->section_length+1] & 0xFF) << 8) | (section[info->section_length+2] & 0xFF);
	data_offset = 12;
	Parser_TsPacketLog(PMT_VALID,"****************PMT Info****************\n");
	Parser_TsPacketLog(PMT_VALID,"PMT table id = 0x%x\n",info->table_id);
	Parser_TsPacketLog(PMT_VALID,"PMT section_syntax_indicator = 0x%x\n",info->section_syntax_indicator);
	Parser_TsPacketLog(PMT_VALID,"PMT section_length = 0x%x\n",info->section_length);
	Parser_TsPacketLog(PMT_VALID,"PMT program_number = 0x%x\n",info->program_number);
	Parser_TsPacketLog(PMT_VALID,"PMT version_number = 0x%x\n",info->version_number);
	Parser_TsPacketLog(PMT_VALID,"PMT current_next_indicator = 0x%x\n",info->current_next_indicator);
	Parser_TsPacketLog(PMT_VALID,"PMT section_number = 0x%x\n",info->section_number);
	Parser_TsPacketLog(PMT_VALID,"PMT last_section_number = 0x%x\n",info->last_section_number);
	Parser_TsPacketLog(PMT_VALID,"PMT pcr_pid = 0x%x\n",info->pcr_pid);
	Parser_TsPacketLog(PMT_VALID,"PMT program_info_length = 0x%x\n",info->program_info_length);
	Parser_TsPacketLog(PMT_VALID,"PMT crc = 0x%08x\n",info->crc);
	Parser_TsPacketLog(PMT_VALID,"****************************************\n");
	len = info->section_length -13;
	len = len - info->program_info_length;
	data_offset += info->program_info_length;
	Parser_TsPacketLog(PMT_VALID,"****************PMT components info****************\n");
	for(i=0;i<len;i+=stream_descriptor_offset)
	{
		Parser_TsPacketLog(PMT_VALID,"PMT stream type = 0x%x\n",section[data_offset]);
		Parser_TsPacketLog(PMT_VALID,"PMT elementary_pid = 0x%x\n",((section[data_offset+1] & 0x1F) << 8) | (section[data_offset+2] & 0xFF));
		stream_descriptor_offset = ((section[data_offset+3] & 0x0F) << 8) | (section[data_offset+4] & 0xFF);
		Parser_TsPacketLog(PMT_VALID,"PMT es_info_length = 0x%x\n",stream_descriptor_offset);
		data_offset += 5;
		for(j = 0;j<stream_descriptor_offset;j+=descriptor_offset)
		{
			switch(section[data_offset]){
				case 0x09:
					descriptor_offset = x09_Tag_DescriptorParser(PMT_VALID,section+data_offset);
					break;
				case 0x0a:
					descriptor_offset = x0A_Tag_DescriptorParser(PMT_VALID,section+data_offset);
					break;
				case 0x0e:
				default:
					descriptor_offset = x0E_Tag_DescriptorParser(PMT_VALID,section+data_offset);
					break;
			}
			descriptor_offset+=2;
			data_offset+=descriptor_offset;
		}
		stream_descriptor_offset+=5;
	}
	Parser_TsPacketLog(PMT_VALID,"****************************************\n");
	ret = 0;

	return ret;
}

int Parser_SectionCAT(unsigned char *section,cat_info_t *info)
{
	unsigned int data_offset = 0;
	unsigned int i = 0;
	int descriptor_offset = 0;
	int ret = -1;
	
	if((NULL == info) || (NULL == section))
	{
		Parser_TsPacketLog(CAT_VALID,"CAT error: param is NULL !\n");
		return -1;
	}
				
	if( 0x01 != section[0])
	{
		Parser_TsPacketLog(CAT_VALID,"CAT table id is error : %x\n",section[0]);
		return -1;
	}
	
	for(i = 0;i<MAX_CAT_DESCRIPTOS_NUM;i++)
	{
		info->ca_descriptors[i].tag = 0;
		info->ca_descriptors[i].length = 0;
		info->ca_descriptors[i].ca_system_id = 0;
		info->ca_descriptors[i].ca_pid = 0;
		memset(info->ca_descriptors[i].data,0,256);
	}
	
	info->table_id = section[0];
	info->section_syntax_indicator = (section[1] & 0x80) >> 7;
	info->section_length = ((section[1] & 0x0F) << 8) | (section[2] & 0xFF);
	info->version_number = (section[5] & 0x3E)>>1;
	info->current_next_indicator = section[5] & 0x01;
	info->section_number = section[6];
	info->last_section_number = section[7];
	data_offset = 8;
	
	info->crc = (section[info->section_length-1] <<24) | ((section[info->section_length] & 0xFF) << 16) 
		|((section[info->section_length+1] & 0xFF) << 8) | (section[info->section_length+2] & 0xFF);
	
	Parser_TsPacketLog(CAT_VALID,"****************CAT Info****************\n");
	Parser_TsPacketLog(CAT_VALID,"CAT table id = 0x%x\n",info->table_id);
	Parser_TsPacketLog(CAT_VALID,"CAT section_syntax_indicator = 0x%x\n",info->section_syntax_indicator);
	Parser_TsPacketLog(CAT_VALID,"CAT section_length = 0x%x\n",info->section_length);
	Parser_TsPacketLog(CAT_VALID,"CAT version_number = 0x%x\n",info->version_number);
	Parser_TsPacketLog(CAT_VALID,"CAT current_next_indicator = 0x%x\n",info->current_next_indicator);
	Parser_TsPacketLog(CAT_VALID,"CAT section_number = 0x%x\n",info->section_number);
	Parser_TsPacketLog(CAT_VALID,"CAT last_section_number = 0x%x\n",info->last_section_number);
	Parser_TsPacketLog(CAT_VALID,"CAT crc = 0x%08x\n",info->crc);
	
	for(i = 0;i<info->section_length-9;i+=descriptor_offset)
	{
		switch(section[data_offset]){
			case 0x09:
				descriptor_offset = x09_Tag_DescriptorParser(CAT_VALID,section+data_offset);
				break;
			default:
				descriptor_offset = x00_Tag_DescriptorParser(CAT_VALID,section+data_offset);
				break;
		}
		descriptor_offset += 2;
		data_offset+=descriptor_offset;
	}	

	Parser_TsPacketLog(CAT_VALID,"****************************************\n");
	ret = 0;
			
	return ret;
}

int Parser_SectionNIT(unsigned char *section,nit_info_t *info)
{
	unsigned short transport_offset = 0;
	unsigned int i = 0,j = 0;
	unsigned int data_offset = 0;
	int descriptor_offset = 0;
	int ret = -1;
	
	if((NULL == info) || (NULL == section))
	{
		Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT error: param is NULL !\n");
		return -1;
	}

	if( (0x40 != section[0]) && (0x41 != section[0]))
	{
		Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT table id is error : %x\n",section[0]);
		return -1;
	}
	
	info->table_id = section[0];
	info->section_syntax_indicator = (section[1] & 0x80) >> 7;
	info->section_length = ((section[1] & 0x0F) << 8) | (section[2] & 0xFF);
	info->network_id = ((section[3] & 0xFF) << 8) | (section[4] & 0xFF);
	info->version_number = (section[5] & 0x3E)>>1;
	info->current_next_indicator = section[5] & 0x01;
	info->section_number = section[6];
	info->last_section_number = section[7];
	info->network_descriptors_length = ((section[8] & 0x0F) << 8) | (section[9] & 0xFF);
	info->transport_stream_loop_length = ((section[10+info->network_descriptors_length] & 0x0F) << 8) 
										| (section[11+info->network_descriptors_length] & 0xFF);
	
	info->crc = (section[info->section_length-1] <<24) | ((section[info->section_length] & 0xFF) << 16) 
		|((section[info->section_length+1] & 0xFF) << 8) | (section[info->section_length+2] & 0xFF);
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"****************NIT Info****************\n");
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT table id = 0x%x\n",info->table_id);
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT section_syntax_indicator = 0x%x\n",info->section_syntax_indicator);
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT section_length = 0x%x\n",info->section_length);
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT network_id = 0x%x\n",info->network_id);
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT version_number = 0x%x\n",info->version_number);
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT current_next_indicator = 0x%x\n",info->current_next_indicator);
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT section_number = 0x%x\n",info->section_number);
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT last_section_number = 0x%x\n",info->last_section_number);
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT network_descriptors_length = 0x%x\n",info->network_descriptors_length);
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT transport_stream_loop_length = 0x%x\n",info->transport_stream_loop_length);
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT crc = 0x%08x\n",info->crc);
	data_offset = 10;
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"****************NIT network_descriptors info****************\n");
	for(i = 0;i<info->network_descriptors_length;i+=descriptor_offset)
	{
		switch(section[data_offset]){
			case 0x40:
				descriptor_offset = x40_Tag_DescriptorParser(NIT_ACTUAL_VALID,section+data_offset);
				break;
			case 0x5F:
				descriptor_offset = x5F_Tag_DescriptorParser(NIT_ACTUAL_VALID,section+data_offset);
				break;
			default:
				descriptor_offset = x00_Tag_DescriptorParser(NIT_ACTUAL_VALID,section+data_offset);
				break;
		}
		descriptor_offset += 2;
		data_offset+=descriptor_offset;
	}
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"****************************************\n");
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"****************NIT transport_stream info****************\n");

	data_offset = 12+info->network_descriptors_length;
	for(i = 0;i<info->transport_stream_loop_length;i+=transport_offset)
	{
		Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT transport_stream_id = 0x%x\n",(((section[data_offset] & 0xFF) << 8) | (section[data_offset+1] & 0xFF)));
		Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT orignal_network_id = 0x%x\n",(((section[data_offset+2] & 0xFF) << 8) | (section[data_offset+3] & 0xFF)));
		transport_offset = ((section[data_offset+4] & 0x0F) << 8) | (section[data_offset+5] & 0xFF);
		Parser_TsPacketLog(NIT_ACTUAL_VALID,"NIT transport_length = 0x%x\n",transport_offset);
		descriptor_offset = 0;
		data_offset+=6;
		for(j = 0;j<transport_offset;j+=descriptor_offset)
		{
			switch(section[data_offset]){
			case 0x41:
				descriptor_offset = x41_Tag_DescriptorParser(NIT_ACTUAL_VALID,section+data_offset);
				break;
			case 0x44:
				descriptor_offset = x44_Tag_DescriptorParser(NIT_ACTUAL_VALID,section+data_offset);
				break;
			default:
				descriptor_offset = x00_Tag_DescriptorParser(NIT_ACTUAL_VALID,section+data_offset);
				break;
			}
			descriptor_offset += 2;
			data_offset+=descriptor_offset;
		}
		transport_offset += 6;
	}

	Parser_TsPacketLog(NIT_ACTUAL_VALID,"****************************************\n");
	Parser_TsPacketLog(NIT_ACTUAL_VALID,"****************************************\n");
	ret = 0;

	return ret;
}

int Parser_SectionSDT(unsigned char *section)
{
	unsigned short descriptor_offset = 0;
	unsigned short service_offset = 0;
	unsigned short section_len = 0;
	unsigned short descriptor_loop_len = 0;
	unsigned short service_loop_len = 0;
	unsigned int i = 0,j = 0;
	unsigned int data_offset = 0;
	int ret = -1;

	if(NULL == section)
	{
		Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT error: section is NULL !\n");
		return -1;
	}
		
	if( (0x42 != section[0]) && (0x46 != section[0]))
	{
		Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT table id is error : %x\n",section[0]);
		return -1;
	}
	
	section_len = ((section[1] & 0x0F) << 8) | (section[2] & 0xFF);
	Parser_TsPacketLog(SDT_ACTUAL_VALID,"****************SDT Info****************\n");
	Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT table id = 0x%x\n",section[0]);
	Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT section_syntax_indicator = 0x%x\n",(section[1] & 0x80) >> 7);
	Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT section_length = 0x%x\n",section_len);
	Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT transport_stream_id = 0x%x\n",((section[3] & 0xFF) << 8) | (section[4] & 0xFF));
	Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT version_number = 0x%x\n",(section[5] & 0x3E)>>1);
	Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT current_next_indicator = 0x%x\n",section[5] & 0x01);
	Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT section_number = 0x%x\n",section[6]);
	Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT last_section_number = 0x%x\n",section[7]);
	Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT original_network_id = 0x%x\n",((section[8] & 0xFF) << 8) | (section[9] & 0xFF));
	Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT crc = 0x%08x\n",(section[section_len-1] <<24) | ((section[section_len] & 0xFF) << 16) 
					|((section[section_len+1] & 0xFF) << 8) | (section[section_len+2] & 0xFF));
	data_offset += 11;
	service_loop_len = section_len - 12;
	for(i= 0;i< service_loop_len;i+=service_offset)
	{
		Parser_TsPacketLog(SDT_ACTUAL_VALID,"****************SDT Service Info****************\n");
		descriptor_loop_len = ((section[data_offset+3] & 0x0F) << 8) | (section[data_offset+4] & 0xFF);
		Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT service_id = 0x%x\n",((section[data_offset] & 0xFF) << 8) | (section[data_offset+1] & 0xFF));
		Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT EIT_schedule_flag = 0x%x\n",(section[data_offset+2] & 0x02) >> 1);
		Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT EIT_present_following_flag = 0x%x\n",(section[data_offset+2] & 0x01));
		Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT running_status = 0x%x\n",(section[data_offset+3] & 0xE0) >> 5);//0:undefined 1:not running 2:starts in a few seconds 3:pausing 4:running 5-7:reserved for future use
		Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT free_CA_mode = 0x%x\n",(section[data_offset+3] & 0x10) >> 4);
		Parser_TsPacketLog(SDT_ACTUAL_VALID,"SDT descriptors_loop_length = 0x%x\n",descriptor_loop_len);
		data_offset += 5;
		for(j = 0;j<descriptor_loop_len;j+=descriptor_offset)
		{
			Parser_TsPacketLog(SDT_ACTUAL_VALID,"****************SDT Service Descriptor Info****************\n");
			switch(section[data_offset]){
				case 0x48:
					descriptor_offset = x48_Tag_DescriptorParser(SDT_ACTUAL_VALID,section+data_offset);
					break;
				case 0x5D:
					descriptor_offset = x5D_Tag_DescriptorParser(SDT_ACTUAL_VALID,section+data_offset);
					break;
				default:
					descriptor_offset = x00_Tag_DescriptorParser(SDT_ACTUAL_VALID,section+data_offset);
					break;
			}
			descriptor_offset += 2;
			data_offset += descriptor_offset;
			Parser_TsPacketLog(SDT_ACTUAL_VALID,"****************************************\n");
		}
		
		service_offset = descriptor_loop_len + 5;
		Parser_TsPacketLog(SDT_ACTUAL_VALID,"****************************************\n");
	}
	Parser_TsPacketLog(SDT_ACTUAL_VALID,"****************************************\n");
	ret = 0;

	return ret;
}

int Parser_SectionBAT(unsigned char *section)
{
	unsigned short section_len = 0;
	unsigned short descriptor_loop_len = 0;
	unsigned short transport_stream_loop_len = 0;
	unsigned short transport_stream_offset = 0;
	unsigned int i = 0,j = 0;
	unsigned int data_offset = 0;
	int descriptor_offset = 0;
	int ret = -1;

	if(NULL == section)
	{
		Parser_TsPacketLog(BAT_VALID,"BAT error: section is NULL !\n");
		return -1;
	}
	
	if(0x4A != section[0])
	{
		Parser_TsPacketLog(BAT_VALID,"BAT table id is error : %x\n",section[0]);
		return -1;
	}

	section_len = ((section[1] & 0x0F) << 8) | (section[2] & 0xFF);
	Parser_TsPacketLog(BAT_VALID,"****************BAT Info****************\n");
	Parser_TsPacketLog(BAT_VALID,"BAT table id = 0x%x\n",section[0]);
	Parser_TsPacketLog(BAT_VALID,"BAT section_syntax_indicator = 0x%x\n",(section[1] & 0x80) >> 7);
	Parser_TsPacketLog(BAT_VALID,"BAT section_length = 0x%x\n",section_len);
	Parser_TsPacketLog(BAT_VALID,"BAT bouquet_id = 0x%x\n",((section[3] & 0xFF) << 8) | (section[4] & 0xFF));
	Parser_TsPacketLog(BAT_VALID,"BAT version_number = 0x%x\n",(section[5] & 0x3E)>>1);
	Parser_TsPacketLog(BAT_VALID,"BAT current_next_indicator = 0x%x\n",section[5] & 0x01);
	Parser_TsPacketLog(BAT_VALID,"BAT section_number = 0x%x\n",section[6]);
	Parser_TsPacketLog(BAT_VALID,"BAT last_section_number = 0x%x\n",section[7]);
	Parser_TsPacketLog(BAT_VALID,"BAT bouquet_descriptors_length = 0x%x\n",((section[8] & 0x0F) << 8) | (section[9] & 0xFF));
	Parser_TsPacketLog(BAT_VALID,"BAT crc = 0x%08x\n",(section[section_len-1] <<24) | ((section[section_len] & 0xFF) << 16) 
					|((section[section_len+1] & 0xFF) << 8) | (section[section_len+2] & 0xFF));
	descriptor_loop_len = ((section[8] & 0x0F) << 8) | (section[9] & 0xFF);
	data_offset += 10;
	for(i = 0;i<descriptor_loop_len;i+=descriptor_offset)
	{
		switch(section[data_offset]){
				case 0x47:
					descriptor_offset = x47_Tag_DescriptorParser(BAT_VALID,section+data_offset);
					break;
				case 0x4A:
					descriptor_offset = x4A_Tag_DescriptorParser(BAT_VALID,section+data_offset);
					break;
				case 0x5F:
					descriptor_offset = x5F_Tag_DescriptorParser(BAT_VALID,section+data_offset);
					break;	
				default:
					descriptor_offset = x00_Tag_DescriptorParser(BAT_VALID,section+data_offset);
					break;
			}
		descriptor_offset += 2;
		data_offset += descriptor_offset;
	}
	transport_stream_loop_len = ((section[data_offset] & 0x0F) << 8) | (section[data_offset+1] & 0xFF);
	Parser_TsPacketLog(BAT_VALID,"BAT transport_stream_loop_length = 0x%x\n",transport_stream_loop_len);
	data_offset+=2;
	descriptor_offset = 0;
	for(i = 0;i<transport_stream_loop_len;i+=transport_stream_offset)
	{
		Parser_TsPacketLog(BAT_VALID,"BAT transport_stream_id = 0x%x\n",((section[data_offset] & 0xFF) << 8) | (section[data_offset+1] & 0xFF));
		Parser_TsPacketLog(BAT_VALID,"BAT original_network_id = 0x%x\n",((section[data_offset+2] & 0xFF) << 8) | (section[data_offset+3] & 0xFF));
		descriptor_loop_len = ((section[data_offset+4] & 0x0F) << 8) | (section[data_offset+5] & 0xFF);
		Parser_TsPacketLog(BAT_VALID,"BAT transport_descriptors_length = 0x%x\n",descriptor_loop_len);
		data_offset+=6;
		for(j = 0;j<descriptor_loop_len;j+=descriptor_offset)
		{
			switch(section[data_offset]){
				case 0x41:
					descriptor_offset = x41_Tag_DescriptorParser(BAT_VALID,section+data_offset);
					break;
				default:
					descriptor_offset = x00_Tag_DescriptorParser(BAT_VALID,section+data_offset);
					break;
			}
			descriptor_offset += 2;
			data_offset += descriptor_offset;
		}
		transport_stream_offset = descriptor_loop_len;
		transport_stream_offset+=6;
	}
	
	Parser_TsPacketLog(BAT_VALID,"****************************************\n");
	ret = 0;

	return ret;
}

int Parser_SectionEIT(unsigned char *section)
{
	unsigned char month = 0;
	unsigned char day = 0;
	unsigned char week = 0;
	unsigned char tmpK = 0;
	unsigned short descriptor_offset = 0;
	unsigned short section_len = 0;
	unsigned short descriptor_loop_len = 0;
	unsigned short service_loop_len = 0;
	unsigned short mjd = 0;
	unsigned short year = 0;
	unsigned int i = 0,j = 0;
	unsigned int data_offset = 0;
	int ret = -1;

	if(NULL == section)
	{
		Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT error: section is NULL !\n");
		return -1;
	}
		
	if((0x4E > section[0]) || (0x6F < section[0]))
	{
		Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT table id is error : %x\n",section[0]);
		return -1;
	}

	section_len = ((section[1] & 0x0F) << 8) | (section[2] & 0xFF);
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"****************EIT Info****************\n");
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT table id = 0x%x\n",section[0]);
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT section_syntax_indicator = 0x%x\n",(section[1] & 0x80) >> 7);
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT section_length = 0x%x\n",section_len);
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT service_id = 0x%x\n",((section[3] & 0xFF) << 8) | (section[4] & 0xFF));
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT version_number = 0x%x\n",(section[5] & 0x3E)>>1);
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT current_next_indicator = 0x%x\n",section[5] & 0x01);
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT section_number = 0x%x\n",section[6]);
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT last_section_number = 0x%x\n",section[7]);
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT transport_stream_id = 0x%x\n",((section[8] & 0xFF) << 8) | (section[9] & 0xFF));
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT original_network_id = 0x%x\n",((section[10] & 0xFF) << 8) | (section[11] & 0xFF));
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT segment_last_section_number = 0x%x\n",(section[12] & 0xFF));
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT last_table_id = 0x%x\n",(section[13] & 0xFF));
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT crc = 0x%08x\n",(section[section_len-1] <<24) | ((section[section_len] & 0xFF) << 16) 
								|((section[section_len+1] & 0xFF) << 8) | (section[section_len+2] & 0xFF));
	data_offset += 14;
	for(i = 0;i<section_len - 15;i+=service_loop_len)
	{
		Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT event_id = 0x%x\n",((section[data_offset]&0xFF)<<8)| (section[data_offset+1]&0xFF));
		mjd = ((section[data_offset+2] & 0xFF) << 8) | (section[data_offset+3] & 0xFF);
		year = (mjd - 15078.2)/365.25;
		month = (mjd - 14956.1 - (int)(year * 365.25))/30.6001;
		day = mjd - 14956 - (int)(year * 365.25) - (int)(month * 30.6001);
		if((14 == month) || (15 == month))
		{
			tmpK = 1;
		}
		else
		{
			tmpK = 0;
		}
		year = year + tmpK;
		month = month -1 - tmpK *12;
		year += 1900;
		week = ((mjd+2)%7) + 1;
		Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT start_time : %d/%02d/%02d %d%d:%d%d:%d%d weekday: %d\n",year,month,day,
				(section[data_offset+4]& 0xF0)>>4,(section[data_offset+4]& 0x0F),(section[data_offset+5]& 0xF0)>>4,
				(section[data_offset+5]& 0x0F),(section[data_offset+6]& 0xF0)>>4,(section[data_offset+6]& 0x0F),week);
		Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT duration : %d%d:%d%d:%d%d\n",(section[data_offset+7]& 0xF0)>>4,
											section[data_offset+7]& 0x0F,(section[data_offset+8]& 0xF0)>>4,
											section[data_offset+8]& 0x0F,(section[data_offset+9]& 0xF0)>>4,
											section[data_offset+9]& 0x0F);
		Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT running_status = 0x%x\n",(section[data_offset+10]&0xE0)>>5);
		Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT free_CA_mode = 0x%x\n",(section[data_offset+10]&0x10)>>4);
		descriptor_loop_len = ((section[data_offset+10] & 0x0F) << 8) | (section[data_offset+11] & 0xFF);
		Parser_TsPacketLog(EIT_ACTUAL_VALID,"EIT descriptors_loop_length = 0x%x\n",descriptor_loop_len);
		data_offset+=12;
		for(j = 0;j<descriptor_loop_len;j+=descriptor_offset)
		{
			switch(section[data_offset]){
				case 0x4D:
					descriptor_offset = x4D_Tag_DescriptorParser(EIT_ACTUAL_VALID,section+data_offset);
					break;
				case 0x50:
					descriptor_offset = x50_Tag_DescriptorParser(EIT_ACTUAL_VALID,section+data_offset);
					break;
				case 0x54:
					descriptor_offset = x54_Tag_DescriptorParser(EIT_ACTUAL_VALID,section+data_offset);
					break;	
				default:
					descriptor_offset = x00_Tag_DescriptorParser(EIT_ACTUAL_VALID,section+data_offset);
					break;
			}
			descriptor_offset += 2;
			data_offset += descriptor_offset;
		}
		service_loop_len = descriptor_loop_len;
		service_loop_len+=12;
	}
	
	Parser_TsPacketLog(EIT_ACTUAL_VALID,"****************************************\n");
	ret = 0;

	return ret;
}

int Parser_SectionTOT(unsigned char *section,tot_info_t *info)
{

	unsigned char month = 0;
	unsigned char day = 0;
	unsigned char week = 0;
	unsigned char tmpK = 0;
	unsigned short mjd = 0;
	unsigned short year = 0;
	unsigned int data_offset = 0;
	unsigned int i = 0;
	int descriptor_offset = 0;
	
	if((NULL == info) || (NULL == section))
	{
		return -1;
	}

	if(0x73 != section[0])
	{
		Parser_TsPacketLog(TOT_VALID,"TOT table id is error : %x\n",section[0]);
		return -1;
	}
	
	info->descriptor_loop_length = 0;
	info->table_id = section[0];
	info->section_syntax_indicator = (section[1] & 0x80) >> 7;
	info->section_length = ((section[1] & 0x0F) << 8) | (section[2] & 0xFF);
	memcpy(info->utc,section+3,5);
	mjd = ((section[3] & 0xFF) << 8) | (section[4] & 0xFF);
	year = (mjd - 15078.2)/365.25;
	month = (mjd - 14956.1 - (int)(year * 365.25))/30.6001;
	day = mjd - 14956 - (int)(year * 365.25) - (int)(month * 30.6001);
	if((14 == month) || (15 == month))
	{
		tmpK = 1;
	}
	else
	{
		tmpK = 0;
	}
	year = year + tmpK;
	month = month -1 - tmpK *12;
	year += 1900;
	week = ((mjd+2)%7) + 1;
	
	info->crc = (section[info->section_length-1] <<24) | ((section[info->section_length] & 0xFF) << 16) 
		|((section[info->section_length+1] & 0xFF) << 8) | (section[info->section_length+2] & 0xFF);
	Parser_TsPacketLog(TOT_VALID,"****************TOT Info****************\n");
	Parser_TsPacketLog(TOT_VALID,"TOT table id = 0x%x\n",info->table_id);
	Parser_TsPacketLog(TOT_VALID,"TOT section_syntax_indicator = 0x%x\n",info->section_syntax_indicator);
	Parser_TsPacketLog(TOT_VALID,"TOT section_length = 0x%x\n",info->section_length);
	Parser_TsPacketLog(TOT_VALID,"TOT utc = 0x%02x%02x%02x%02x%02x\n",info->utc[0],info->utc[1],info->utc[2],info->utc[3],info->utc[4]);
	Parser_TsPacketLog(TOT_VALID,"TOT time : %d/%02d/%02d %d%d:%d%d:%d%d weekday: %d\n",year,month,day,
				(info->utc[2]& 0xF0)>>4,(info->utc[2]& 0x0F),(info->utc[3]& 0xF0)>>4,(info->utc[3]& 0x0F),
				(info->utc[4]& 0xF0)>>4,(info->utc[4]& 0x0F),week);
	Parser_TsPacketLog(TOT_VALID,"TOT crc = 0x%x\n",info->crc);
	
	if(0x73 == info->table_id)
	{
		info->descriptor_loop_length = ((section[8] & 0x0F) << 8) | (section[9] & 0xFF);
		Parser_TsPacketLog(TOT_VALID,"TOT descriptor_loop_length = 0x%x\n",info->descriptor_loop_length);
		data_offset = 10;
		for(i = 0;i<info->descriptor_loop_length;i+=descriptor_offset)
		{
			switch(section[data_offset]){
				case 0x58:
					descriptor_offset = x58_Tag_DescriptorParser(TOT_VALID,section+data_offset);
					break;
				default:
					descriptor_offset = x00_Tag_DescriptorParser(TOT_VALID,section+data_offset);
					break;
			}
			descriptor_offset+=2;
			data_offset+=descriptor_offset;
		}
	}
	Parser_TsPacketLog(TOT_VALID,"****************************************\n");
	return 0;
}

int Parser_SectionTDT(unsigned char *section,tot_info_t *info)
{
	unsigned char month = 0;
	unsigned char day = 0;
	unsigned char week = 0;
	unsigned char tmpK = 0;
	unsigned short mjd = 0;
	unsigned short year = 0;
	
	if((NULL == info) || (NULL == section))
	{
		Parser_TsPacketLog(TDT_VALID,"TDT error: param is NULL !\n");
		return -1;
	}

	if(0x70 != section[0])
	{
		Parser_TsPacketLog(TDT_VALID,"TDT table id is error : %x\n",section[0]);
		return -1;
	}
	
	info->descriptor_loop_length = 0;
	info->table_id = section[0];
	info->section_syntax_indicator = (section[1] & 0x80) >> 7;
	info->section_length = ((section[1] & 0x0F) << 8) | (section[2] & 0xFF);
	memcpy(info->utc,section+3,5);
	mjd = ((section[3] & 0xFF) << 8) | (section[4] & 0xFF);
	year = (mjd - 15078.2)/365.25;
	month = (mjd - 14956.1 - (int)(year * 365.25))/30.6001;
	day = mjd - 14956 - (int)(year * 365.25) - (int)(month * 30.6001);
	if((14 == month) || (15 == month))
	{
		tmpK = 1;
	}
	else
	{
		tmpK = 0;
	}
	year = year + tmpK;
	month = month -1 - tmpK *12;
	year += 1900;
	week = ((mjd+2)%7) + 1;
	
	info->crc = (section[info->section_length-1] <<24) | ((section[info->section_length] & 0xFF) << 16) 
		|((section[info->section_length+1] & 0xFF) << 8) | (section[info->section_length+2] & 0xFF);
	Parser_TsPacketLog(TDT_VALID,"****************TDT Info****************\n");
	Parser_TsPacketLog(TDT_VALID,"TDT table id = 0x%x\n",info->table_id);
	Parser_TsPacketLog(TDT_VALID,"TDT section_syntax_indicator = 0x%x\n",info->section_syntax_indicator);
	Parser_TsPacketLog(TDT_VALID,"TDT section_length = 0x%x\n",info->section_length);
	Parser_TsPacketLog(TDT_VALID,"TDT utc = 0x%02x%02x%02x%02x%02x\n",info->utc[0],info->utc[1],info->utc[2],info->utc[3],info->utc[4]);
	Parser_TsPacketLog(TDT_VALID,"TDT time : %d/%02d/%02d %d%d:%d%d:%d%d weekday: %d\n",year,month,day,
				(info->utc[2]& 0xF0)>>4,(info->utc[2]& 0x0F),(info->utc[3]& 0xF0)>>4,(info->utc[3]& 0x0F),
				(info->utc[4]& 0xF0)>>4,(info->utc[4]& 0x0F),week);
	Parser_TsPacketLog(TDT_VALID,"TDT crc = 0x%x\n",info->crc);
	
	Parser_TsPacketLog(TDT_VALID,"****************************************\n");
	return 0;
}

