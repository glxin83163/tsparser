#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "TsPacket_ParserLog.h"
#include "TsPacket_DescriptorParser.h"

/*Possible locations of descriptors
Descriptor 												Tag value 	NIT BAT SDT EIT TOT PMT SIT(note 1)
network_name_descriptor 								0x40 		* 	- 	- 	- 	- 	- 	-
service_list_descriptor 								0x41 		* 	* 	- 	- 	- 	- 	-
stuffing_descriptor 									0x42 		* 	* 	* 	*	- 	- 	*
satellite_delivery_system_descriptor 					0x43 		* 	- 	- 	- 	- 	- 	-
cable_delivery_system_descriptor 						0x44 		* 	- 	- 	- 	- 	- 	-
VBI_data_descriptor 									0x45 		- 	- 	- 	- 	- 	* 	-
VBI_teletext_descriptor 								0x46 		- 	- 	- 	- 	- 	* 	-
bouquet_name_descriptor 								0x47 		- 	* 	- 	- 	- 	- 	*
service_descriptor 										0x48 		- 	- 	* 	- 	- 	- 	*
country_availability_descriptor 						0x49 		- 	* 	* 	- 	- 	- 	*
linkage_descriptor 										0x4A 		* 	* 	* 	* 	- 	- 	*
NVOD_reference_descriptor 								0x4B 		- 	- 	* 	- 	- 	- 	*
time_shifted_service_descriptor 						0x4C 		- 	- 	* 	- 	- 	- 	*
short_event_descriptor 									0x4D 		- 	- 	- 	* 	- 	- 	*
extended_event_descriptor 								0x4E 		- 	- 	- 	* 	- 	- 	*
time_shifted_event_descriptor 							0x4F 		- 	- 	- 	* 	- 	- 	*
component_descriptor 									0x50 		- 	- 	* 	* 	- 	- 	*
mosaic_descriptor 										0x51 		- 	- 	* 	- 	- 	* 	*
stream_identifier_descriptor 							0x52 		- 	- 	- 	- 	- 	* 	-
CA_identifier_descriptor 								0x53 		- 	* 	* 	* 	- 	- 	*
content_descriptor 										0x54 		- 	- 	- 	* 	- 	- 	*
parental_rating_descriptor 								0x55 		- 	- 	- 	* 	- 	- 	*
teletext_descriptor 									0x56 		- 	- 	- 	- 	- 	* 	-
telephone_descriptor 									0x57 		- 	- 	* 	* 	- 	- 	*
local_time_offset_descriptor 							0x58 		- 	- 	- 	- 	* 	- 	-
subtitling_descriptor 									0x59 		- 	- 	- 	- 	- 	* 	-
terrestrial_delivery_system_descriptor 					0x5A 		* 	- 	- 	- 	- 	- 	-
multilingual_network_name_descriptor 					0x5B 		* 	- 	- 	- 	- 	- 	-
multilingual_bouquet_name_descriptor 					0x5C 		- 	* 	- 	- 	- 	- 	-
multilingual_service_name_descriptor 					0x5D 		- 	- 	* 	- 	- 	- 	*
multilingual_component_descriptor 						0x5E 		- 	- 	- 	* 	- 	- 	*
private_data_specifier_descriptor 						0x5F 		* 	* 	* 	* 	- 	* 	*
service_move_descriptor 								0x60 		- 	- 	- 	- 	- 	* 	-
short_smoothing_buffer_descriptor 						0x61 		- 	- 	- 	* 	- 	- 	*
frequency_list_descriptor 								0x62 		* 	- 	- 	- 	- 	- 	-
partial_transport_stream_descriptor(see note 1)			0x63 		- 	- 	- 	- 	- 	- 	*
data_broadcast_descriptor 								0x64 		- 	- 	* 	* 	- 	- 	*
scrambling_descriptor 									0x65 		- 	- 	- 	- 	- 	* 	-
data_broadcast_id_descriptor 							0x66 		- 	- 	- 	- 	- 	* 	-
transport_stream_descriptor(see note 2)					0x67 		- 	- 	- 	- 	- 	- 	-
DSNG_descriptor (see note 2) 							0x68 		- 	- 	- 	- 	- 	- 	-
PDC_descriptor 											0x69 		- 	- 	- 	* 	- 	- 	-
AC-3_descriptor (see annex D) 							0x6A 		- 	- 	- 	- 	- 	* 	-
ancillary_data_descriptor 								0x6B 		- 	- 	- 	- 	- 	* 	-
cell_list_descriptor 									0x6C 		* 	- 	- 	- 	- 	- 	-
cell_frequency_link_descriptor 							0x6D 		* 	- 	- 	- 	- 	- 	-
announcement_support_descriptor 						0x6E 		- 	- 	* 	- 	- 	- 	-
application_signalling_descriptor(see [56])				0x6F 		- 	- 	- 	- 	- 	* 	-
adaptation_field_data_descriptor 						0x70 		- 	- 	- 	- 	-	* 	-
service_identifier_descriptor (see [15]) 				0x71 		- 	- 	* 	- 	- 	- 	-
service_availability_descriptor 						0x72 		- 	- 	* 	- 	- 	- 	-
default_authority_descriptor(ETSI TS 102 323 [13])		0x73 		* 	* 	* 	- 	- 	- 	-
related_content_descriptor(ETSI TS 102 323 [13])		0x74 		- 	-	- 	- 	- 	* 	-
TVA_id_descriptor(ETSI TS 102 323 [13])					0x75 		- 	- 	- 	* 	- 	- 	-
content_identifier_descriptor(ETSI TS 102 323 [13])		0x76 		- 	- 	- 	* 	- 	- 	-
time_slice_fec_identifier_descriptor
(ETSI EN 301 192 [4]) (see note 3)						0x77 		* 	- 	- 	- 	- 	- 	-
ECM_repetition_rate_descriptor(ETSI EN 301 192 [4])		0x78 		- 	- 	- 	- 	- 	* 	-
S2_satellite_delivery_system_descriptor 				0x79 		* 	- 	- 	- 	- 	- 	-
enhanced_AC-3_descriptor(see annex D)					0x7A 		- 	- 	- 	- 	- 	* 	-
DTS® descriptor (see annex G) 							0x7B 		- 	- 	- 	-	- 	* 	-
AAC descriptor (see annex H) 							0x7C 		- 	- 	- 	- 	- 	* 	-
XAIT location descriptor (see [i.3]) 					0x7D 		* 	* 	* 	* 	* 	* 	*
FTA_content_management_descriptor 						0x7E 		* 	* 	* 	* 	- 	- 	-
extension descriptor (see note 5) 						0x7F 		* 	* 	* 	* 	* 	* 	*
user defined 											0x80 to 0xFE
forbidden 												0xFF
NOTE 1: Only found in Partial Transport Streams.
NOTE 2: Only in the TSDT (Transport Streams Description Table).
NOTE 3: May also be located in the CAT (ISO/IEC 13818-1 [18]) and INT (ETSI TS 102 006 [11]).
NOTE 4: * Possible location.
NOTE 5: See also clauses 6.3 and 6.4.
*/

static void Parser_DisplayTagInfo(unsigned int type,unsigned char tag)
{
	if(tag < 0x40)
	{
		switch(tag){
			case 0x09:
				Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (CAT PMT) : ca_descriptor ******\n",tag);
				break;
			case 0x0A:
				Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : iso639_language_descriptor ******\n",tag);
				break;
			case 0x0E:
				Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : maxinum_bitrate_descriptor ******\n",tag);
				break;
			case 0x3F:
				Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : reserved_descriptor ******\n",tag);
				break;
			default:
				Parser_TsPacketLog(type,"******Descriptor tag : 0x%x : unkown ******\n",tag);
				break;
		}
		return;
	}
	switch(tag){
		case 0x40:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT) : network_name_descriptor ******\n",tag);
			break;
		case 0x41:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT BAT) : service_list_descriptor ******\n",tag);
			break;
		case 0x42:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT BAT SDT EIT SIT) : stuffing_descriptor ******\n",tag);
			break;
		case 0x43:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT) : satellite_delivery_system_descriptor ******\n",tag);
			break;
		case 0x44:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT) : cable_delivery_system_descriptor ******\n",tag);
			break;
		case 0x45:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : VBI_data_descriptor ******\n",tag);
			break;
		case 0x46:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : VBI_teletext_descriptor ******\n",tag);
			break;
		case 0x47:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (BAT SIT) : bouquet_name_descriptor ******\n",tag);
			break;
		case 0x48:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (SDT SIT) : service_descriptor ******\n",tag);
			break;
		case 0x49:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (BAT SDT SIT) : country_availability_descriptor ******\n",tag);
			break;
		case 0x4A:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT BAT SDT EIT SIT) : linkage_descriptor ******\n",tag);
			break;
		case 0x4B:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (SDT SIT) : NVOD_reference_descriptor ******\n",tag);
			break;
		case 0x4C:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (SDT SIT) : time_shifted_service_descriptor ******\n",tag);
			break;
		case 0x4D:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (EIT SIT) : short_event_descriptor ******\n",tag);
			break;
		case 0x4E:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (EIT SIT) : extended_event_descriptor ******\n",tag);
			break;
		case 0x4F:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (EIT SIT) : time_shifted_event_descriptor ******\n",tag);
			break;
		case 0x50:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (SDT EIT SIT) : component_descriptor ******\n",tag);
			break;
		case 0x51:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (SDT PMT SIT) : mosaic_descriptor ******\n",tag);
			break;
		case 0x52:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : stream_identifier_descriptor ******\n",tag);
			break;
		case 0x53:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (BAT SDT EIT SIT) : CA_identifier_descriptor ******\n",tag);
			break;
		case 0x54:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (EIT SIT) : content_descriptor ******\n",tag);
			break;
		case 0x55:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (EIT SIT) : parental_rating_descriptor ******\n",tag);
			break;
		case 0x56:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : teletext_descriptor ******\n",tag);
			break;
		case 0x57:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (SDT EIT SIT) : telephone_descriptor ******\n",tag);
			break;
		case 0x58:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (TOT) : local_time_offset_descriptor ******\n",tag);
			break;
		case 0x59:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : subtitling_descriptor ******\n",tag);
			break;
		case 0x5A:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT) : terrestrial_delivery_system_descriptor ******\n",tag);
			break;
		case 0x5B:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT) : multilingual_network_name_descriptor ******\n",tag);
			break;
		case 0x5C:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (BAT) : multilingual_bouquet_name_descriptor ******\n",tag);
			break;
		case 0x5D:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (SDT SIT) : multilingual_service_name_descriptor ******\n",tag);
			break;
		case 0x5E:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (EIT SIT) : multilingual_component_descriptor ******\n",tag);
			break;
		case 0x5F:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT BAT SDT EIT PMT SIT) : private_data_specifier_descriptor ******\n",tag);
			break;
		case 0x60:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : service_move_descriptor ******\n",tag);
			break;
		case 0x61:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (EIT SIT) : short_smoothing_buffer_descriptor ******\n",tag);
			break;
		case 0x62:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT) : frequency_list_descriptor ******\n",tag);
			break;
		case 0x63:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (SIT) : partial_transport_stream_descriptor ******\n",tag);
			break;
		case 0x64:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (SDT EIT) : data_broadcast_descriptor ******\n",tag);
			break;
		case 0x65:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : scrambling_descriptor ******\n",tag);
			break;
		case 0x66:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : data_broadcast_id_descriptor ******\n",tag);
			break;
		case 0x67:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x : transport_stream_descriptor ******\n",tag);
			break;
		case 0x68:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x : DSNG_descriptor ******\n",tag);
			break;
		case 0x69:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (EIT) : PDC_descriptor ******\n",tag);
			break;
		case 0x6A:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : AC-3_descriptor ******\n",tag);
			break;
		case 0x6B:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : ancillary_data_descriptor ******\n",tag);
			break;
		case 0x6C:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT) : cell_list_descriptor ******\n",tag);
			break;
		case 0x6D:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT) : cell_frequency_link_descriptor ******\n",tag);
			break;
		case 0x6E:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (SDT) : announcement_support_descriptor ******\n",tag);
			break;
		case 0x6F:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : application_signalling_descriptor ******\n",tag);
			break;
		case 0x70:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : adaptation_field_data_descriptor ******\n",tag);
			break;
		case 0x71:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (SDT) : service_identifier_descriptor ******\n",tag);
			break;
		case 0x72:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (SDT) : service_availability_descriptor ******\n",tag);
			break;
		case 0x73:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT BAT SDT) : default_authority_descriptor ******\n",tag);
			break;
		case 0x74:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : related_content_descriptor ******\n",tag);
			break;
		case 0x75:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (EIT) : TVA_id_descriptor ******\n",tag);
			break;
		case 0x76:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (EIT) : content_identifier_descriptor ******\n",tag);
			break;
		case 0x77:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT) : time_slice_fec_identifier_descriptor ******\n",tag);
			break;
		case 0x78:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : ECM_repetition_rate_descriptor ******\n",tag);
			break;
		case 0x79:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT) : S2_satellite_delivery_system_descriptor ******\n",tag);
			break;
		case 0x7A:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : enhanced_AC-3_descriptor ******\n",tag);
			break;
		case 0x7B:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : DTS descriptor ******\n",tag);
			break;
		case 0x7C:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (PMT) : AAC descriptor ******\n",tag);
			break;
		case 0x7D:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT BAT SDT EIT TOT PMT SIT) : XAIT location descriptor ******\n",tag);
			break;
		case 0x7E:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT BAT SDT EIT) : FTA_content_management_descriptor ******\n",tag);
			break;
		case 0x7F:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x (NIT BAT SDT EIT TOT PMT SIT) : extension descriptor ******\n",tag);
			break;
		case 0xFF:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x : forbidden ******\n",tag);
			break;
		default:
			Parser_TsPacketLog(type,"******Descriptor tag : 0x%x : user defined ******\n",tag);
			break;
	}
}


int x09_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"Ca_system_id = 0x%x\n",((buff[2] & 0xFF) << 8) | (buff[3] & 0xFF));
	Parser_TsPacketLog(type,"Ca_pid = 0x%x\n",((buff[4] & 0x1F) << 8) | (buff[5] & 0xFF));
	if(buff[1] > 4)
	{
		Parser_TsPacketLog(type,"Descriptor_data:");
		for(i = 0;i<buff[1]-4;i++)
		{
			Parser_TsPacketLog(type," 0x%x",buff[6+i]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x0A_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"ISO_639_language_code: 0x%x : %c%c%c\n",
							((buff[2] & 0xFF) << 16) | ((buff[3] & 0xFF) << 8)
							| (buff[4] & 0xFF),buff[2],buff[3],buff[4]);
	Parser_TsPacketLog(type,"audio_type = 0x%x\n",buff[5]);
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x0E_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	if(0 != buff[1])
	{
		Parser_TsPacketLog(type,"Descriptor_data:");
		for(i = 0;i<buff[1];i++)
		{
			Parser_TsPacketLog(type," 0x%x",buff[i+2]);
		}
		Parser_TsPacketLog(type,"\n");
	}

	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x3F_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	if(0 != buff[1])
	{
		Parser_TsPacketLog(type,"Descriptor_data:");
		for(i = 0;i<buff[1];i++)
		{
			Parser_TsPacketLog(type," 0x%x",buff[i+2]);
		}
		Parser_TsPacketLog(type,"\n");
	}

	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x40_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	unsigned char *data = NULL;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	data = buff;
	data += 2;
	if(0 != buff[1])
	{
		Parser_TsPacketLog(type,"Network name: ");
		for(i = 0;i<buff[1];i++)
		{
			Parser_TsPacketLog(type,"%c",data[i]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x41_Tag_DescriptorParser*** 
***NIT BAT : Service list descriptor service_type coding***
SEE x41_Tag_DescriptorParser note 
*/
int x41_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	unsigned char *data = NULL;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	data = buff;
	data += 2;
	if(0 != buff[1])
	{
		for(i = 0;i<buff[1];i+=3)
		{
			Parser_TsPacketLog(type,"service_id = 0x%x\n",((data[i]&0xFF) << 8)| (data[i+1]&0xFF));
			Parser_TsPacketLog(type,"service_type = 0x%x\n",data[i+2]);
		}
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x42_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	if(0 != buff[1])
	{
		Parser_TsPacketLog(type,"stuffing_byte: ");
		for(i = 0;i<buff[1];i++)
		{
			Parser_TsPacketLog(type,"0x%02x",buff[i+2]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x43_Tag_DescriptorParser*** 
***NIT : Satellite delivery system descriptor polarization coding***
* 00:linear - horizontal
* 01:linear - vertical
* 10:Circular - left
* 11:Circular - right
***NIT : Cable delivery system descriptor roll-off factor coding***
* 00:α = 0,35
* 01:α = 0,25
* 10:α = 0,20
* 11:Reserved
***NIT : Cable delivery system descriptor modulation system coding***
* 0:DVB-S
* 1:DVB-S2
***NIT : Cable delivery system descriptor modulation type coding***
* 00:Auto
* 01:QPSK
* 10:8PSK
* 11:16-QAM (n/a for DVB-S2)
*/
int x43_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"frequency = 0x%x\n",((buff[2]&0xFF) << 24) | ((buff[3]&0xFF) << 16)
										| ((buff[4]&0xFF) << 8) | (buff[5]&0xFF));
	Parser_TsPacketLog(type,"orbital_position = 0x%x\n",((buff[6]&0xFF) << 8) | (buff[7]&0xFF));
	Parser_TsPacketLog(type,"west_east_flag = 0x%x\n",buff[8]&0x80);
	Parser_TsPacketLog(type,"polarization = 0x%x\n",buff[8]&0x60);
	if(0x04 == (buff[8]&0x04))
	{
		Parser_TsPacketLog(type,"roll_off = 0x%x\n",(buff[8]&0x18) >> 3);
	}
	else
	{
		Parser_TsPacketLog(type,"roll_off = 00\n");
	}
	Parser_TsPacketLog(type,"modulation_system = 0x%x\n",(buff[8]&0x04)>>2);
	Parser_TsPacketLog(type,"modulation_type = 0x%x\n",buff[8]&0x03);
	Parser_TsPacketLog(type,"symbol_rate = 0x%x\n",((buff[9]&0xFF) << 20) | ((buff[10]&0xFF) << 12)
										| ((buff[11]&0xFF) << 4) | ((buff[12]&0xF0) >> 4));
	Parser_TsPacketLog(type,"FEC_inner = 0x%x\n",buff[12]&0x0F);
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x44_Tag_DescriptorParser*** 
***NIT : Cable delivery system descriptor FEC_outer coding***
* 0000:not defined
* 0001:no outer FEC coding
* 0010:RS(204/188)
* 0011--1111:Reserved for future use
***NIT : Cable delivery system descriptor modulation coding***
* 0x00:not defined
* 0x01:16-QAM
* 0x02:32-QAM
* 0x03:64-QAM
* 0x04:128-QAM
* 0x05:256-QAM
* 0x06--0xFF:Reserved for future use
***NIT : Cable delivery system descriptor FEC_inner coding***
* 0000:not defined
* 0001:1/2 conv. code rate
* 0010:2/3 conv. code rate
* 0011:3/4 conv. code rate
* 0100:5/6 conv. code rate
* 0101:7/8 conv. code rate
* 0110:8/9 conv. code rate
* 0111:3/5 conv. code rate
* 1000:4/5 conv. code rate
* 1001:9/10 conv. code rate
* 1010--1110:Reserved for future use
* 1111:no conv. Coding
NOTE: Not all convolutional code rates apply for all modulation
schemes.
*/
int x44_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"frequency = %x\n",((buff[2]&0xFF) << 24) | ((buff[3]&0xFF) << 16)
										| ((buff[4]&0xFF) << 8) | (buff[5]&0xFF));
	Parser_TsPacketLog(type,"FEC_outer = 0x%x\n",buff[7]&0x0F);
	Parser_TsPacketLog(type,"modulation = 0x%x\n",buff[8]);
	Parser_TsPacketLog(type,"symbol_rate = %x\n",((buff[9]&0xFF) << 20) | ((buff[10]&0xFF) << 12)
										| ((buff[11]&0xFF) << 4) | ((buff[12]&0xF0) >> 4));
	Parser_TsPacketLog(type,"FEC_inner = 0x%x\n",buff[12]&0x0F);
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x45_Tag_DescriptorParser*** 
***PMT : VBI data descriptor data_service_id coding***
* 0x00:Reserved for future use
* 0x01:EBU teletext (Requires additional teletext_descriptor)
* 0x02:inverted teletext
* 0x03:reserved
* 0x04:VPS
* 0x05:WSS
* 0x06:Closed Captioning
* 0x07:monochrome 4:2:2 samples
* 0x08--0xFF:Reserved for future use
*/
int x45_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0,j = 0;
	unsigned char tmplen = 0;
	unsigned char *data = NULL;
	if(NULL == buff)
	{
		return -1;
	}
	
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	data = buff;
	data += 2;
	for(i = 0;i<buff[1];i++)
	{
		Parser_TsPacketLog(type,"data_service_id = 0x%x\n",data[0]);
		tmplen = data[1];
		Parser_TsPacketLog(type,"data_service_descriptor_length = 0x%x\n",tmplen);
		if((data[0] == 0x01) || (data[0] == 0x02) || (data[0] == 0x04)
					|| (data[0] == 0x05) || (data[0] == 0x06) || (data[0] == 0x07))
		{
			data += 2;
			for(j = 0;j<tmplen;j++)
			{
				Parser_TsPacketLog(type,"field_parity = 0x%x\n",(data[j]&0x40) >> 5);
				Parser_TsPacketLog(type,"line_offset = 0x%x\n",data[j]&0x1F);
			}
		}
		else
		{
			data += 2;
			Parser_TsPacketLog(type,"reserved = 0x%x ...\n",data[0]);
		}
		data+=tmplen;
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x46_Tag_DescriptorParser*** 
***PMT : VBI teletext descriptor teletext_type coding***
* 0x00:Reserved for future use
* 0x01:initial Teletext page
* 0x02:Teletext subtitle page
* 0x03:additional information page
* 0x04:programme schedule page
* 0x05:Teletext subtitle page for hearing impaired people
* 0x06--0x1F:Reserved for future use
NOTE:The semantics for the VBI teletext descriptor is the same as defined for the teletext descriptor(x56_Tag_DescriptorParser). The only
exception is that the VBI teletext descriptor is not to be used to associate stream_type 0x06 with the VBI standard nor
the EBU teletext standard. Decoders can only use the languages in this descriptor to select magazines and subtitles.
*/
int x46_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	unsigned char *data = NULL;
	if(NULL == buff)
	{
		return -1;
	}
	
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	data = buff;
	data += 2;
	for(i = 0;i<buff[1];i+=5)
	{
		Parser_TsPacketLog(type,"ISO_639_language_code = 0x%x : %c%c%c\n",((data[0]&0xFF) << 16) | ((buff[1]&0xFF) << 8) | (buff[2]&0xFF),
													data[0],data[1],data[2]);
		Parser_TsPacketLog(type,"teletext_type = 0x%x\n",(data[3]&0xF8) >> 3);
		Parser_TsPacketLog(type,"teletext_magazine_number = 0x%x\n",data[3]&0x07);
		Parser_TsPacketLog(type,"teletext_page_number = 0x%x\n",data[4]);
		data+=5;
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x47_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	if(0 != buff[1])
	{
		Parser_TsPacketLog(type,"Bouquet name:");
		for(i = 0;i<buff[1];i++)
		{
			Parser_TsPacketLog(type,"%c",buff[i+2]);
		}
		Parser_TsPacketLog(type,"\n");
	}

	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x48_Tag_DescriptorParser*** 
***SDT SIT : Service descriptor service_type coding***
* 0x00:Reserved for future use
* 0x01:digital television service (see note 1)
* 0x02:digital radio sound service (see note 2)
* 0x03:Teletext service
* 0x04:NVOD reference service (see note 1)
* 0x05:NVOD time-shifted service (see note 1)
* 0x06:mosaic service
* 0x07:FM radio service
* 0x08:DVB SRM service [49]
* 0x09:Reserved for future use
* 0x0A:advanced codec digital radio sound service
* 0x0B:H.264/AVC mosaic service
* 0x0C:data broadcast service
* 0x0D:reserved for Common Interface Usage (CENELEC EN 50221 [37])
* 0x0E:RCS Map (see ETSI EN 301 790 [7])
* 0x0F:RCS FLS (see ETSI EN 301 790 [7])
* 0x10:DVB MHP service
* 0x11:MPEG-2 HD digital television service
* 0x12--0x15:Reserved for future use
* 0x16:H.264/AVC SD digital television service
* 0x17:H.264/AVC SD NVOD time-shifted service
* 0x18:H.264/AVC SD NVOD reference service
* 0x19:H.264/AVC HD digital television service
* 0x1A:H.264/AVC HD NVOD time-shifted service
* 0x1B:H.264/AVC HD NVOD reference service
* 0x1C:H.264/AVC frame compatible plano-stereoscopic HD digital television service(see note 3)
* 0x1D:H.264/AVC frame compatible plano-stereoscopic HD NVOD time-shiftedservice(see note 3)
* 0x1E:H.264/AVC frame compatible plano-stereoscopic HD NVOD reference service(see note 3)
* 0x1F:HEVC digital television service (see note 4)
* 0x20--0x7F:Reserved for future use
* 0x80--0xFE:user defined
* 0xFF:Reserved for future use
NOTE 1: MPEG-2 SD material should use this type.
NOTE 2: MPEG-1 Layer 2 audio material should use this type.
NOTE 3: For information on the use of these values, see clause I.2.3 and ETSI TS 101 547-2 [54]
(3D Guidelines of Frame Compatible 3D-TV).
NOTE 4: For information on the use of these values, see clause I.2.5 and ETSI TS 101 547-4 [62]
*/
int x48_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	unsigned char *data = NULL;
	unsigned char tmplen = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"service_type = 0x%x\n",buff[2]);
	Parser_TsPacketLog(type,"service_provider_name_length = 0x%x\n",buff[3]);
	data = buff;
	data+=4;
	if(0 != buff[3])
	{
		Parser_TsPacketLog(type,"service_provider_name:");
		for(i = 0;i<buff[3];i++)
		{
			Parser_TsPacketLog(type,"%c",data[i]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	data+=i;
	tmplen = data[0];
	data+=1;
	Parser_TsPacketLog(type,"service_name_length = 0x%x\n",tmplen);
	if(0 != tmplen)
	{
		Parser_TsPacketLog(type,"service_name:");
		for(i = 0;i<tmplen;i++)
		{
			Parser_TsPacketLog(type,"%c",data[i]);
		}
		Parser_TsPacketLog(type,"\n");
	}

	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x49_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"country_availability_flag = 0x%x\n",(buff[2]&0x80) >> 7);
	for(i = 1;i<buff[1];i+=3)
	{
		Parser_TsPacketLog(type,"country_code = 0x%x\n",((buff[i+2]&0xFF) << 16) | ((buff[i+3]&0xFF) <<8) | (buff[i+4]&0xFF));
		Parser_TsPacketLog(type,"country_code : %c%c%c\n",(buff[i+2]&0xFF),(buff[i+3]&0xFF),(buff[i+4]&0xFF));
	}

	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x4A_Tag_DescriptorParser*** 
***NIT BAT SDT EIT SIT: Linkage descriptor linkage_type coding***
* 0x00:Reserved for future use
* 0x01:information service
* 0x02:EPG service
* 0x03:CA replacement service
* 0x04:TS containing complete Network/Bouquet SI
* 0x05:service replacement service
* 0x06:data broadcast service
* 0x07:RCS Map
* 0x08:mobile hand-over
* 0x09:System Software Update Service (ETSI TS 102 006 [11])
* 0x0A:TS containing SSU BAT or NIT (ETSI TS 102 006 [11])
* 0x0B:IP/MAC Notification Service (ETSI EN 301 192 [4])
* 0x0C:TS containing INT BAT or NIT (ETSI EN 301 192 [4])
* 0x0D:event linkage (see note)
* 0x0E--0x1F:extended event linkage (see note)
* 0x20--0x7F:Reserved for future use
* 0x80--0xFE:user defined
* 0xFF:Reserved for future use
* NOTE: A linkage_type with a value in the range 0x0D to 0x1F is only valid
* when the descriptor is carried in the EIT.
***NIT BAT SDT EIT SIT: Linkage descriptor hand-over_type coding***
* 0x00:Reserved for future use
* 0x01:DVB hand-over to an identical service in a neighbouring country
* 0x02:DVB hand-over to a local variation of the same service
* 0x03:DVB hand-over to an associated service
* 0x04--0x0F:Reserved for future use
***NIT BAT SDT EIT SIT: Linkage descriptor origin_type coding***
* 0x00:NIT
* 0x01:SDT
***NIT BAT SDT EIT SIT: Linkage descriptor target_id_type coding***
* 0:use transport_stream_id
* 1:use target_transport_stream_id
* 2:match any transport_stream_id (wildcard)
* 3:use user_defined_id
***NIT BAT SDT EIT SIT: Linkage descriptor link_type coding***
link_type linkage_type Type of target service
0 			0x0E 		SD
1 			0x0E 		HD
2 			0x0E 		frame compatible plano-stereoscopic H.264/AVC
3 			0x0E 		service compatible plano-stereoscopic MVC
0 			0x0F 		UHD
1 			0x0F 		service frame compatible plano-stereoscopic
2--3		0x0F 		reserved for future use
0--3 		0x10--0x1F 	reserved for future use
*/
int x4A_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	unsigned int offset = 0;
	unsigned char tmp = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"transport_stream_id = 0x%x\n",((buff[2]&0xFF)<<8) | (buff[3]&0xFF));
	Parser_TsPacketLog(type,"original_network_id = 0x%x\n",((buff[4]&0xFF)<<8) | (buff[5]&0xFF));
	Parser_TsPacketLog(type,"service_id = 0x%x\n",((buff[6]&0xFF)<<8) | (buff[7]&0xFF));
	Parser_TsPacketLog(type,"linkage_type = 0x%x\n",buff[8]);
	offset = 7;
	if(0x08 == buff[8])
	{
		Parser_TsPacketLog(type,"hand-over_type = 0x%x\n",(buff[9]&0xF0)>>4);
		Parser_TsPacketLog(type,"origin_type = 0x%x\n",buff[9]&0x01);
		offset+=1;
		if((buff[9]&0xF0) <= 0x30)
		{
			Parser_TsPacketLog(type,"network_id = 0x%x\n",((buff[10]&0xFF)<<8) | (buff[11]&0xFF));
			offset+=2;
		}
		if(0 == (buff[9]&0x01))
		{
			if((buff[9]&0xF0) <= 0x30)
			{
				Parser_TsPacketLog(type,"initial_service_id = 0x%x\n",((buff[12]&0xFF)<<8) | (buff[13]&0xFF));
			}
			else
			{
				Parser_TsPacketLog(type,"initial_service_id = 0x%x\n",((buff[10]&0xFF)<<8) | (buff[11]&0xFF));
			}
			offset+=2;
		}
	}
	else if(0x0D == buff[8])
	{
		Parser_TsPacketLog(type,"target_event_id = 0x%x\n",((buff[9]&0xFF)<<8) | (buff[10]&0xFF));
		Parser_TsPacketLog(type,"target_listed = 0x%x\n",(buff[11]&0x80)>>7);
		Parser_TsPacketLog(type,"event_simulcast = 0x%x\n",(buff[11]&0x40)>>6);
		offset+=3;
	}
	else if((buff[8] >= 0x0E) && (buff[8] <= 0x1F))
	{
		Parser_TsPacketLog(type,"loop_length = 0x%x\n",buff[9]);
		offset+=1;
		if(0 != buff[9])
		{
			for(i = 0;i<buff[9];i+=tmp)
			{
				Parser_TsPacketLog(type,"target_event_id = 0x%x\n",((buff[i+10]&0xFF)<<8) | (buff[i+11]&0xFF));
				Parser_TsPacketLog(type,"target_listed = 0x%x\n",(buff[i+12]&0x80)>>7);
				Parser_TsPacketLog(type,"event_simulcast = 0x%x\n",(buff[i+12]&0x40)>>6);
				Parser_TsPacketLog(type,"link_type = 0x%x\n",(buff[i+12]&0x30)>>4);
				Parser_TsPacketLog(type,"target_id_type = 0x%x\n",(buff[i+12]&0x0C)>>2);
				Parser_TsPacketLog(type,"original_network_id_flag = 0x%x\n",(buff[i+12]&0x02)>>1);
				Parser_TsPacketLog(type,"service_id_flag = 0x%x\n",buff[i+12]&0x01);
				if(0x0C == (buff[i+12]&0x0C))
				{
					Parser_TsPacketLog(type,"user_defined_id = 0x%x\n",((buff[i+13]&0xFF)<<8) | (buff[i+14]&0xFF));
					tmp = 5;
					offset+=5;
				}
				else
				{
					tmp = 3;
					offset+=3;
					if(0x04 == (buff[i+12]&0x0C))
					{
						Parser_TsPacketLog(type,"target_transport_stream_id = 0x%x\n",((buff[i+13]&0xFF)<<8) | (buff[i+14]&0xFF));
						tmp+=2;
						offset+=2;
					}
					if(0x02 == (buff[i+12]&0x02))
					{
						Parser_TsPacketLog(type,"target_original_network_id = 0x%x\n",((buff[i+tmp+10]&0xFF)<<8) | (buff[i+tmp+11]&0xFF));
						tmp+=2;
						offset+=2;
					}
					if(0x01 == (buff[i+12]&0x01))
					{
						Parser_TsPacketLog(type,"target_service_id = 0x%x\n",((buff[i+tmp+10]&0xFF)<<8) | (buff[i+tmp+11]&0xFF));
						tmp+=2;
						offset+=2;
					}
				}
			}
		}
	}
	if(offset < buff[1])
	{
		tmp = buff[1] - offset;
		Parser_TsPacketLog(type,"private_data:");
		for(i = 0;i<tmp;i++)
		{
			Parser_TsPacketLog(type," 0x%02x",buff[i+offset+2]);
		}
		Parser_TsPacketLog(type,"\n");
	}
		
	return buff[1];
}

int x4B_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	unsigned char *data = NULL;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	data = buff;
	data+=2;
	for(i = 0;i<buff[1];i+=6)
	{
		Parser_TsPacketLog(type,"transport_stream_id = 0x%x\n",((data[0]&0xFF) << 8) | (data[1]&0xFF));
		Parser_TsPacketLog(type,"original_network_id = 0x%x\n",((data[2]&0xFF) << 8) | (data[3]&0xFF));
		Parser_TsPacketLog(type,"service_id = 0x%x\n",((data[4]&0xFF) << 8) | (data[5]&0xFF));
		data+=6;
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x4C_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}

	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"reference_service_id = 0x%x\n",((buff[2]&0xFF) << 8) | (buff[3]&0xFF));
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x4D_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	unsigned char tmplen = 0;
	unsigned char *data = NULL;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"ISO_639_language_code = 0x%x : %c%c%c\n",((buff[2]&0xFF) << 16) | ((buff[3]&0xFF) << 8) | (buff[4]&0xFF),
													buff[2],buff[3],buff[4]);
	Parser_TsPacketLog(type,"event_name_length = 0x%x\n",buff[5]);
	data = buff;
	data+=6;
	if(0 != buff[5])
	{
		Parser_TsPacketLog(type,"event_name: ");
		for(i = 0;i<buff[5];i++)
		{
			Parser_TsPacketLog(type,"%c",data[i]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	
	tmplen = data[i];
	data+=i;
	data+=1;
	Parser_TsPacketLog(type,"text_length = 0x%x\n",tmplen);
	if(0 != tmplen)
	{
		Parser_TsPacketLog(type,"text: ");
		for(i = 0;i<tmplen;i++)
		{
			Parser_TsPacketLog(type,"%c",data[i]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x4E_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0,j = 0;
	unsigned int offset = 0;
	unsigned char tmp = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"descriptor_number = 0x%x\n",(buff[2]&0xF0) >> 4);
	Parser_TsPacketLog(type,"last_descriptor_number = 0x%x\n",buff[2]&0x0F);
	Parser_TsPacketLog(type,"ISO_639_language_code = 0x%x : %c%c%c\n",(buff[3] << 16) | (buff[4] << 8) | (buff[5]&0xFF),
													buff[3],buff[4],buff[5]);
	Parser_TsPacketLog(type,"length_of_items = 0x%x\n",buff[6]);
	if(0 != buff[6])
	{
		for(i = 0;i<buff[6];i+=offset)
		{
			Parser_TsPacketLog(type,"item_description_length = 0x%x\n",buff[i+7]);
			if(0 != buff[i+7])
			{
				Parser_TsPacketLog(type,"item_description_char: ");
				for(j = 0;j<buff[i+7];j++)
				{
					Parser_TsPacketLog(type,"%c",buff[i+j+8]);
				}
				Parser_TsPacketLog(type,"\n");
			}
			offset = j+1;
			tmp = buff[i+j+8];
			Parser_TsPacketLog(type,"item_length = 0x%x\n",tmp);
			if(0 != tmp)
			{
				Parser_TsPacketLog(type,"item_char: ");
				for(j = 0;j<tmp;j++)
				{
					Parser_TsPacketLog(type,"%c",buff[i+j+offset+8]);
				}
				Parser_TsPacketLog(type,"\n");
			}
			offset += tmp;
			offset += 1;
		}
	}
	Parser_TsPacketLog(type,"text_length = 0x%x\n",buff[i+7]);
	if(0 != buff[i+7])
	{
		Parser_TsPacketLog(type,"text_char: ");
		for(j = 0;j<buff[i+7];j++)
		{
			Parser_TsPacketLog(type,"%c",buff[i+j+8]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x4F_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}

	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"reference_service_id = 0x%x\n",((buff[2]&0xFF) << 8) | (buff[3]&0xFF));
	Parser_TsPacketLog(type,"reference_event_id = 0x%x\n",((buff[4]&0xFF) << 8) | (buff[5]&0xFF));
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x50_Tag_DescriptorParser*** 
***SDT/EIT/SIT : Component_descriptor coding***
stream_content | stream_content_ext | component_type | Description
0x0 				0x0 to 0xF 			0x00 to 0xFF 	reserved for future use
0x1 				n/a (see note 8) 	0x00 			reserved for future use
										0x01 			MPEG-2 video, 4:3 aspect ratio, 25 Hz(see note 2)
										0x02 			MPEG-2 video, 16:9 aspect ratio with pan vectors, 25 Hz (see note 2)
										0x03 			MPEG-2 video, 16:9 aspect ratio without pan vectors, 25 Hz (see note 2)
										0x04 			MPEG-2 video, > 16:9 aspect ratio, 25 Hz(see note 2)
										0x05 			MPEG-2 video, 4:3 aspect ratio, 30 Hz(see note 2)
										0x06 			MPEG-2 video, 16:9 aspect ratio with pan vectors, 30 Hz (see note 2)
										0x07 			MPEG-2 video, 16:9 aspect ratio without pan vectors, 30 Hz (see note 2)
										0x08 			MPEG-2 video, > 16:9 aspect ratio, 30 Hz(see note 2)
										0x09 			MPEG-2 high definition video, 4:3 aspect ratio,25 Hz (see note 2)
										0x0A 			MPEG-2 high definition video, 16:9 aspect ratio with pan vectors, 25 Hz (see note 2)
										0x0B 			MPEG-2 high definition video, 16:9 aspect ratio without pan vectors, 25 Hz (see note 2)
										0x0C 			MPEG-2 high definition video, > 16:9 aspect ratio, 25 Hz (see note 2)
										0x0D 			MPEG-2 high definition video, 4:3 aspect ratio,30 Hz (see note 2)
										0x0E 			MPEG-2 high definition video, 16:9 aspect ratio with pan vectors, 30 Hz (see note 2)
										0x0F 			MPEG-2 high definition video, 16:9 aspect ratio without pan vectors, 30 Hz (see note 2)
										0x10 			MPEG-2 high definition video, > 16:9 aspect ratio, 30 Hz (see note 2)
										0x11 to 0xAF 	reserved for future use
										0xB0 to 0xFE 	user defined
										0xFF 			reserved for future use 
0x2 				n/a (see note 8) 	0x00 			reserved for future use
										0x01 			MPEG-1 Layer 2 audio, single mono channel
										0x02 			MPEG-1 Layer 2 audio, dual mono channel
										0x03 			MPEG-1 Layer 2 audio, stereo (2 channel)
										0x04 			MPEG-1 Layer 2 audio, multi-lingual,multi-channel
										0x05 			MPEG-1 Layer 2 audio, surround sound
										0x06 to 0x3F 	reserved for future use
										0x40 			MPEG-1 Layer 2 audio description for the visually impaired (see note 5)
										0x41 			MPEG-1 Layer 2 audio for the hard of hearing
										0x42 			receiver-mix supplementary audio as per annex E of ETSI TS 101 154 [9]
										0x43 to 0x46 	reserved for future use
										0x47 			MPEG-1 Layer 2 audio, receiver-mix audio description
										0x48 			MPEG-1 Layer 2 audio, broadcast-mix audio description
										0x49 to 0xAF 	reserved for future use
										0xB0 to 0xFE 	user-defined
										0xFF 			reserved for future use
0x3 				n/a (see note 8) 	0x00 			reserved for future use
										0x01 			EBU Teletext subtitles
										0x02 			associated EBU Teletext
										0x03 			VBI data
										0x04 to 0x0F 	reserved for future use
										0x10 			DVB subtitles (normal) with no monitor aspect ratio criticality
										0x11 			DVB subtitles (normal) for display on 4:3 aspect ratio monitor
										0x12 			DVB subtitles (normal) for display on 16:9 aspect ratio monitor
										0x13 			DVB subtitles (normal) for display on 2.21:1 aspect ratio monitor
										0x14 			DVB subtitles (normal) for display on a high definition monitor
										0x15 			DVB subtitles (normal) with plano-stereoscopic disparity for display on a high definition monitor
										0x16 to 0x1F 	reserved for future use
										0x20 			DVB subtitles (for the hard of hearing) with no monitor aspect ratio criticality
										0x21 			DVB subtitles (for the hard of hearing) for display on 4:3 aspect ratio monitor
										0x22 			DVB subtitles (for the hard of hearing) for display on 16:9 aspect ratio monitor
										0x23 			DVB subtitles (for the hard of hearing) for display on 2.21:1 aspect ratio monitor
										0x24 			DVB subtitles (for the hard of hearing) for display on a high definition monitor
										0x25 			DVB subtitles (for the hard of hearing) with plano-stereoscopic disparity for display on a high definition monitor
										0x26 to 0x2F 	reserved for future use
										0x30 			open (in-vision) sign language interpretation for the deaf (see note 7)
										0x31 			closed sign language interpretation for the deaf (see note 7)
										0x32 to 0x3F 	reserved for future use
										0x40 			video up-sampled from standard definition source material (see note 7)
										0x41 to 0x7F 	reserved for future use
										0x80 			dependent SAOC-DE data stream
										0x81 to 0xAF 	reserved for future use
										0xB0 to 0xFE 	user defined
										0xFF 			reserved for future use
0x4 				n/a (see note 8) 	0x00 to 0x7F 	reserved for AC-3 audio modes (refer to table D.1)
										0x80 to 0xFF 	reserved for enhanced AC-3 audio modes (refer to table D.1)
0x5 				n/a (see note 8) 	0x00 			reserved for future use
										0x01 			H.264/AVC standard definition video, 4:3 aspect ratio, 25 Hz (see note 2)
										0x02 			reserved for future use
										0x03 			H.264/AVC standard definition video, 16:9 aspect ratio, 25 Hz (see note 2)
										0x04 			H.264/AVC standard definition video, > 16:9 aspect ratio, 25 Hz (see note 2)
										0x05 			H.264/AVC standard definition video, 4:3 aspect ratio, 30 Hz (see note 2)
										0x06 			reserved for future use
										0x07 			H.264/AVC standard definition video, 16:9 aspect ratio, 30 Hz (see note 2)
										0x08 			H.264/AVC standard definition video, > 16:9 aspect ratio, 30 Hz (see note 2)
										0x09 to 0x0A 	reserved for future use
										0x0B 			H.264/AVC high definition video, 16:9 aspect ratio, 25 Hz (see note 2)
										0x0C 			H.264/AVC high definition video, > 16:9 aspect ratio, 25 Hz (see note 2)
										0x0D to 0x0E 	reserved for future use
										0x0F 			H.264/AVC high definition video, 16:9 aspect ratio, 30 Hz (see note 2)
										0x10 			H.264/AVC high definition video, > 16:9 aspect ratio, 30 Hz (see note 2)
										0x11 to 0x7F 	reserved for future use
										0x80 			H.264/AVC plano-stereoscopic frame compatible high definition video, 16:9 aspect ratio, 25 Hz, Side-by-Side (see notes 2 and 3)
										0x81 			H.264/AVC plano-stereoscopic frame compatible high definition video, 16:9 aspect ratio, 25 Hz, Top-and-Bottom (see notes 2 and 3)
										0x82 			H.264/AVC plano-stereoscopic frame compatible high definition video, 16:9 aspect ratio, 30 Hz, Side-by-Side (see notes 2, 3 and 4)
										0x83 			H.264/AVC stereoscopic frame compatible high definition video, 16:9 aspect ratio, 30 Hz,Top-and-Bottom (see notes 2, 3 and 4)
										0x84 			H.264/MVC dependent view, planostereoscopic service compatible video (see note 3)
										0x85 to 0xAF 	reserved for future use
										0xB0 to 0xFE 	user-defined
										0xFF 			reserved for future use
0x6 				n/a (see note 8) 	0x00 			reserved for future use
										0x01 			HE AAC audio, single mono channel (see note 6)
										0x02 			reserved for future use
										0x03 			HE AAC audio, stereo (see note 6)
										0x04 			reserved for future use
										0x05 			HE AAC audio, surround sound (see note 6)
										0x06 to 0x3F 	reserved for future use
										0x40 			HE AAC audio description for the visually impaired (see notes 5 and 6)
										0x41 			HE AAC audio for the hard of hearing (see note 6)
										0x42 			HE AAC receiver-mix supplementary audio as per annex E of ETSI TS 101 154 [9] (see note 6)
										0x43 			HE AAC v2 audio, stereo
										0x44 			HE AAC v2 audio description for the visually impaired (see note 5)
										0x45 			HE AAC v2 audio for the hard of hearing
										0x46 			HE AAC v2 receiver-mix supplementary audio as per annex E of ETSI TS 101 154 [9]
										0x47 			HE AAC receiver-mix audio description for the visually impaired
										0x48 			HE AAC broadcast-mix audio description for the visually impaired
										0x49 			HE AAC v2 receiver-mix audio description for the visually impaired
										0x4A 			HE AAC v2 broadcast-mix audio description for the visually impaired
										0x4B to 0x9F 	reserved for future use
										0xA0 			HE AAC, or HE AAC v2 with SAOC-DE ancillary data (see notes 6 and 7)
										0xA1 to 0xAF 	reserved for future use
										0xB0 to 0xFE 	user-defined
										0xFF 			reserved for future use
0x7 				n/a (see note 8) 	0x00 to 0x7F 	reserved for DTS® and DTS-HD® audio modes (refer to annex G)
										0x80 to 0xFF 	reserved for future use
0x8 				n/a (see note 8) 	0x00 			reserved for future use
										0x01 			DVB SRM data [49]
										0x02 to 0xFF 	reserved for DVB CPCM modes [46], [i.4]
0x9 				0x0 				0x00 			HEVC Main Profile high definition video, 50 Hz (see notes 2 and 9)
										0x01 			HEVC Main 10 Profile high definition video,50 Hz (notes 2 and 9)
										0x02 			HEVC Main Profile high definition video, 60 Hz (see notes 2, 4 and 9)
										0x03 			HEVC Main 10 Profile high definition video, 60 Hz (see notes 2, 4 and 9)
										0x04 			HEVC ultra high definition video (see notes 2 and 9)
										0x05 to 0xFF 	reserved for future use
					0x1 				0x00 			AC-4 main audio, mono
										0x01 			AC-4 main audio, mono, dialogue enhancement enabled
										0x02 			AC-4 main audio, stereo
										0x03 			AC-4 main audio, stereo, dialogue enhancement enabled
										0x04 			AC-4 main audio, multichannel
										0x05 			AC-4 main audio, multichannel, dialogue enhancement enabled
										0x06 			AC-4 broadcast-mix audio description, mono,for the visually impaired
										0x07 			AC-4 broadcast-mix audio description, mono,for the visually impaired, dialogue enhancement enabled
										0x08 			AC-4 broadcast-mix audio description, stereo,for the visually impaired
										0x09 			AC-4 broadcast-mix audio description, stereo,for the visually impaired, dialogue enhancement enabled
										0x0A 			AC-4 broadcast-mix audio description,multichannel, for the visually impaired
										0x0B 			AC-4 broadcast-mix audio description,multichannel, for the visually impaired,dialogue enhancement enabled
										0x0C 			AC-4 receiver-mix audio description, mono, for the visually impaired
										0x0D 			AC-4 receiver-mix audio description, stereo, for the visually impaired
										0x0E to 0xFF 	reserved for future use
					0x2 to 0xF 			0x00 to 0xFF 	reserved for future use
0xA 				0x0 to 0xF 			0x00 to 0xFF 	reserved for future use
0xB 				0x0 to 0xE 			0x00 to 0xFF 	reserved for future use
					0xF (see note 7) 	0x00 			less than 16:9 aspect ratio
										0x01 			16:9 aspect ratio
										0x02 			greater than 16:9 aspect ratio
										0x03 			plano-stereoscopic top and bottom (TaB)frame-packing
										0x04 to 0xFF 	reserved for future use
0xC to 0xF 			n/a 				0x00 to 0xFF 	user defined
-------------------------------------------------------------------------------------------------------------
NOTE 1: The profiles and levels of the codecs mentioned in table 26 are as defined in ETSI TS 101 154 [9] and
ETSI TS 102 005 [10].
NOTE 2: In table 26, the terms "standard definition", "high definition", "ultra high definition", "25 Hz", "30 Hz",
"50 Hz", and "60 Hz" are used as defined in ETSI TS 101 154 [9] clauses 5.1 to 5.4 for MPEG-2 and 5.5
to 5.7 for H.264/AVC, and clauses 5.14.2 and 5.14.3 for HEVC respectively.
NOTE 3: See ETSI TS 101 547-3 [55] for further information on stereoscopic modes.
NOTE 4: 24 Hz video will also use this component_type.
NOTE 5: The specific audio description types indicating the use of broadcast-mix or receiver-mix audio should be
preferred over these generic types. For more details see annex J.
NOTE 6: Audio streams using AAC audio shall use the corresponding HE AAC values. The AAC profile includes
low-complexity AAC.
NOTE 7: These component descriptor values are intended to be present in combination with another component
descriptor with the same component_tag value.
• For example, two component descriptors with the same component tag value, and
stream_content/stream_content_ext/component_type values of 0x5/0xF/0x0B and 0x3/0xF/0x40
respectively, would indicate H.264/AVC high definition video, 16:9 aspect ratio, 25 Hz which has
been up-sampled from a standard definition source.
• For example, two component descriptors with the same component tag value, and
stream_content/stream_content_ext/component_type values of 0x6/0xF/0x03 and 0x6/0xF/0xA0
respectively, would indicate stereo HE AAC audio with SAOC-DE parametric data embedded as
ancillary data.
• For example, two component descriptors with the same component tag value, and
stream_content/stream_content_ext/component_type values 0x9/0x0/0x01 and 0xB/0xF/0x01
respectively, would indicate HEVC Main 10 profile high definition video at 50 Hz with a 16:9 aspect
ratio.
NOTE 8: In order to maintain backwards compatibility, the value of the stream_content_ext field is not applicable
(n/a) for stream_content values in the range 0x01 to 0x8, and is set to 0xF.
NOTE 9: For information on the use of these values, see clause I.2.5 and ETSI TS 101 547-4 [62].
*/
int x50_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"stream_content_ext = 0x%x\n",(buff[2]&0xF0) >> 4);
	Parser_TsPacketLog(type,"stream_content = 0x%x\n",buff[2]&0x0F);
	Parser_TsPacketLog(type,"component_type = 0x%x\n",buff[3]);
	Parser_TsPacketLog(type,"component_tag = 0x%x\n",buff[4]);
	Parser_TsPacketLog(type,"ISO_639_language_code = 0x%x : %c%c%c\n",(buff[5] << 16) | (buff[6] << 8) | (buff[7]&0xFF),
													buff[5],buff[6],buff[7]);
	if(buff[1]>6)
	{
		Parser_TsPacketLog(type,"Component descriptor text : ");
	
		for(i = 6;i<buff[1];i+=1)
		{
			Parser_TsPacketLog(type,"%c",buff[i+2]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x51_Tag_DescriptorParser*** 
***SDT PMT SIT : Mosaic descriptor number_of_horizontal_elementary_cells/number_of_vertical_elementary_cells coding***
* 0x00:one cell
* 0x01:two cells
* 0x02:three cells
* 0x03:four cells
* 0x04:five cells
* 0x05:six cells
* 0x06:seven cells
* 0x07:eight cells
***SDT PMT SIT : Mosaic descriptor logical_cell_presentation_info coding***
* 0x00:undefined
* 0x01:video
* 0x02:still picture (see note)
* 0x03:graphics/text
* 0x04--0x07:Reserved for future use
* NOTE: Still picture: A coded still picture consists of a video sequence
* containing exactly one coded picture which is intra-coded.
***SDT PMT SIT : Mosaic descriptor cell_linkage_info coding***
* 0x00:not defined
* 0x01:bouquet related
* 0x02:service related
* 0x03:other mosaic related
* 0x04:event related
* 0x05--0xFF:Reserved for future use
* NOTE::
* when cell_linkage_info = "0x02", this is the service_id of the service described by the cell;
* when cell_linkage_info = "0x03", this is the service_id of the mosaic service described by the cell;
* when cell_linkage_info = "0x04", this is the service_id of the service to which the event described by the cell belongs.
*/
int x51_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0,j = 0;
	unsigned char tmp = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"mosaic_entry_point = 0x%x\n",((buff[2]&0x80) >> 7));
	Parser_TsPacketLog(type,"number_of_horizontal_elementary_cells = 0x%x\n",((buff[2]&0x70) >> 4));
	Parser_TsPacketLog(type,"number_of_vertical_elementary_cells = 0x%x\n",(buff[2]&0x07));
	if(buff[1]>1)
	{
		for(i = 0;i<buff[1];i+=2)
		{
			Parser_TsPacketLog(type,"logical_cell_id = 0x%x\n",((buff[i+3]&0xFB) >> 2));
			Parser_TsPacketLog(type,"logical_cell_presentation_info = 0x%x\n",buff[i+4]&0x07);
			Parser_TsPacketLog(type,"elementary_cell_field_length = 0x%x\n",buff[i+5]);
			for(j = 0;j<buff[i+5];j++)
			{
				Parser_TsPacketLog(type,"elementary_cell_id = 0x%x\n",buff[i+j+6]&0x3F);
			}
			tmp = buff[i+5];
			Parser_TsPacketLog(type,"cell_linkage_info = 0x%x\n",buff[i+tmp+6]);
			if(0x01 == buff[i+tmp+6])
			{
				Parser_TsPacketLog(type,"bouquet_id = 0x%x\n",((buff[i+tmp+7]&0xFF) << 8) | (buff[i+tmp+8]&0xFF));
			}
			else if((0x02 == buff[i+tmp+6]) || (0x03 == buff[i+tmp+6]))
			{
				Parser_TsPacketLog(type,"original_network_id = 0x%x\n",((buff[i+tmp+7]&0xFF) << 8) | (buff[i+tmp+8]&0xFF));
				Parser_TsPacketLog(type,"transport_stream_id = 0x%x\n",((buff[i+tmp+9]&0xFF) << 8) | (buff[i+tmp+10]&0xFF));
				Parser_TsPacketLog(type,"service_id = 0x%x\n",((buff[i+tmp+11]&0xFF) << 8) | (buff[i+tmp+12]&0xFF));
			}
			else if(0x04 == buff[i+tmp+6])
			{
				Parser_TsPacketLog(type,"original_network_id = 0x%x\n",((buff[i+tmp+7]&0xFF) << 8) | (buff[i+tmp+8]&0xFF));
				Parser_TsPacketLog(type,"transport_stream_id = 0x%x\n",((buff[i+tmp+9]&0xFF) << 8) | (buff[i+tmp+10]&0xFF));
				Parser_TsPacketLog(type,"service_id = 0x%x\n",((buff[i+tmp+11]&0xFF) << 8) | (buff[i+tmp+12]&0xFF));
				Parser_TsPacketLog(type,"event_id = 0x%x\n",((buff[i+tmp+13]&0xFF) << 8) | (buff[i+tmp+14]&0xFF));
			}
		}
	}
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x52_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"component_tag = 0x%x\n",buff[2]);
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x53_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	
	for(i = 0;i<buff[1];i+=2)
	{
		Parser_TsPacketLog(type,"CA_system_id = 0x%x\n",((buff[i+2]&0xFF) << 8) | (buff[i+3]&0xFF));
	}

	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x54_Tag_DescriptorParser*** 
***EIT/SIT : Content descriptor coding***
content_nibble_level_1 	content_nibble_level_2 	Description
0x0 					0x0 to 0xF 				undefined content
												Movie/Drama:
0x1 					0x0 					movie/drama (general)
						0x1 					detective/thriller
						0x2 					adventure/western/war
						0x3 					science fiction/fantasy/horror
						0x4 					comedy
						0x5 					soap/melodrama/folkloric
						0x6 					romance
						0x7 					serious/classical/religious/historical movie/drama
						0x8 					adult movie/drama
						0x9 to 0xE 				reserved for future use
						0xF 					user defined
												News/Current affairs:
0x2 					0x0 					news/current affairs (general)
						0x1 					news/weather report
						0x2 					news magazine
						0x3 					documentary
						0x4 					discussion/interview/debate
						0x5 to 0xE 				reserved for future use
						0xF 					user defined
												Show/Game show:
0x3 					0x0 					show/game show (general)
						0x1 					game show/quiz/contest
						0x2 					variety show
						0x3 					talk show
						0x4 to 0xE 				reserved for future use
						0xF 					user defined
												Sports:
0x4 					0x0 					sports (general)
						0x1 					special events (Olympic Games, World Cup, etc.)
						0x2 					sports magazines
						0x3 					football/soccer
						0x4 					tennis/squash
						0x5 					team sports (excluding football)
						0x6 					athletics
						0x7 					motor sport
						0x8 					water sport
						0x9 					winter sports
						0xA 					equestrian
						0xB 					martial sports
						0xC to 0xE 				reserved for future use
						0xF 					user defined
												Children's/Youth programmes:
0x5 					0x0 					children's/youth programmes (general)
						0x1 					pre-school children's programmes
						0x2 					entertainment programmes for 6 to14
						0x3 					entertainment programmes for 10 to 16
						0x4 					informational/educational/school programmes
						0x5 					cartoons/puppets
						0x6 to 0xE 				reserved for future use
						0xF 					user defined
												Music/Ballet/Dance:
0x6 					0x0 					music/ballet/dance (general)
						0x1 					rock/pop
						0x2 					serious music/classical music
						0x3 					folk/traditional music
						0x4 					jazz
						0x5 					musical/opera
						0x6 					ballet
						0x7 to 0xE 				reserved for future use
						0xF 					user defined
												Arts/Culture (without music):
0x7 					0x0 					arts/culture (without music, general)
						0x1 					performing arts
						0x2 					fine arts
						0x3 					religion
						0x4 					popular culture/traditional arts
						0x5 					literature
						0x6 					film/cinema
						0x7 					experimental film/video
						0x8 					broadcasting/press
						0x9 					new media
						0xA 					arts/culture magazines
						0xB 					fashion
						0xC to 0xE 				reserved for future use
						0xF 					user defined
												Social/Political issues/Economics:
0x8 					0x0 					social/political issues/economics (general)
						0x1 					magazines/reports/documentary
						0x2 					economics/social advisory
						0x3 					remarkable people
						0x4 to 0xE 				reserved for future use
						0xF 					user defined
												Education/Science/Factual topics:
0x9 					0x0 					education/science/factual topics (general)
						0x1 					nature/animals/environment
						0x2 					technology/natural sciences
						0x3 					medicine/physiology/psychology
						0x4 					foreign countries/expeditions
						0x5 					social/spiritual sciences
						0x6 					further education
						0x7 					languages
						0x8 to 0xE 				reserved for future use
						0xF 					user defined
												Leisure hobbies:
0xA 					0x0 					leisure hobbies (general)
						0x1 					tourism/travel
						0x2 					handicraft
						0x3 					motoring
						0x4 					fitness and health
						0x5 					cooking
						0x6 					advertisement/shopping
						0x7 					gardening
						0x8 to 0xE 				Reserved for future use
						0xF 					user defined
												Special characteristics:
0xB 					0x0 					original language
						0x1 					black and white
						0x2 					unpublished
						0x3 					live broadcast
						0x4 					plano-stereoscopic
						0x5 					local or regional
						0x6 to 0xE 				Reserved for future use
						0xF 					user defined
												Reserved for future use:
0xC to 0xE 				0x0 to 0xF 				Reserved for future use
												User defined:
0xF 					0x0 to 0xF 				user defined
*/
int x54_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	
	for(i = 0;i<buff[1];i+=2)
	{
		Parser_TsPacketLog(type,"content_nibble_level_1 = 0x%x\n",((buff[i+2]&0xF0) >> 4));
		Parser_TsPacketLog(type,"content_nibble_level_2 = 0x%x\n",buff[i+2]&0x0F);
		Parser_TsPacketLog(type,"user_byte = 0x%x\n",buff[i+3]);
	}
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x55_Tag_DescriptorParser*** 
***EIT SIT : Parental rating descriptor rating coding***
* 0x00:undefined
* 0x01--0x0F:minimum age = rating + 3 years
* 0x10--0xFF:defined by the broadcaster
*/
int x55_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	unsigned char *data = NULL;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	data = buff;
	data += 2;

	for(i = 0;i<buff[1];i+=4)
	{
		Parser_TsPacketLog(type,"country_code = 0x%x : %c%c%c\n",((data[i]&0xFF) << 16) | ((data[i+1]&0xFF) << 8) | (data[i+2]&0xFF),
																		data[i],data[i+1],data[i+2]);
		Parser_TsPacketLog(type,"rating = 0x%x\n",data[i+3]);
		data+=4;
	}

	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x56_Tag_DescriptorParser*** 
***PMT : Teletext descriptor teletext_type coding***
* 0x00:Reserved for future use
* 0x01:initial Teletext page
* 0x02:Teletext subtitle page
* 0x03:additional information page
* 0x04:programme schedule page
* 0x05:Teletext subtitle page for hearing impaired people
* 0x06--0x1F:Reserved for future use
*/
int x56_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	unsigned char *data = NULL;
	if(NULL == buff)
	{
		return -1;
	}
	
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	data = buff;
	data += 2;
	for(i = 0;i<buff[1];i+=5)
	{
		Parser_TsPacketLog(type,"ISO_639_language_code = 0x%x : %c%c%c\n",((data[0]&0xFF) << 16) | ((buff[1]&0xFF) << 8) | (buff[2]&0xFF),
													data[0],data[1],data[2]);
		Parser_TsPacketLog(type,"teletext_type = 0x%x\n",(data[3]&0xF8) >> 3);
		Parser_TsPacketLog(type,"teletext_magazine_number = 0x%x\n",data[3]&0x07);
		Parser_TsPacketLog(type,"teletext_page_number = 0x%x\n",data[4]);
		data+=5;
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x57_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	unsigned char tmplen = 0;
	unsigned char *data = NULL;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"foreign_availability = 0x%x\n",(buff[2]&0x30)>>5);
	Parser_TsPacketLog(type,"connection_type = 0x%x\n",buff[2]&0x1F);
	Parser_TsPacketLog(type,"country_prefix_length = 0x%x\n",(buff[3]&0x60)>>5);
	Parser_TsPacketLog(type,"international_area_code_length = 0x%x\n",(buff[3]&0x1C)>>2);
	Parser_TsPacketLog(type,"operator_code_length = 0x%x\n",buff[3]&0x03);
	Parser_TsPacketLog(type,"national_area_code_length = 0x%x\n",(buff[4]&0x70)>>4);
	Parser_TsPacketLog(type,"core_number_length = 0x%x\n",buff[4]&0x0F);
	data = buff;
	data+=5;
	tmplen = (buff[3]&0x60)>>5;
	if( tmplen != 0)
	{
		Parser_TsPacketLog(type,"country_prefix_char: ");
		for(i = 0;i<tmplen;i++)
		{
			Parser_TsPacketLog(type,"%c",data[i]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	data+=tmplen;
	tmplen = (buff[3]&0x1C)>>2;
	if( tmplen != 0)
	{
		Parser_TsPacketLog(type,"international_area_code_char: ");
		for(i = 0;i<tmplen;i++)
		{
			Parser_TsPacketLog(type,"%c",data[i]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	data+=tmplen;
	tmplen = buff[3]&0x03;
	if( tmplen != 0)
	{
		Parser_TsPacketLog(type,"operator_code_char: ");
		for(i = 0;i<tmplen;i++)
		{
			Parser_TsPacketLog(type,"%c",data[i]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	data+=tmplen;
	tmplen = (buff[4]&0x70)>>4;
	if( tmplen != 0)
	{
		Parser_TsPacketLog(type,"national_area_code_char: ");
		for(i = 0;i<tmplen;i++)
		{
			Parser_TsPacketLog(type,"%c",data[i]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	data+=tmplen;
	tmplen = buff[4]&0x0F;
	if( tmplen != 0)
	{
		Parser_TsPacketLog(type,"core_number_char: ");
		for(i = 0;i<tmplen;i++)
		{
			Parser_TsPacketLog(type,"%c",data[i]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x58_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned char i = 0;
	unsigned short mjd = 0;
	unsigned short year = 0;
	unsigned char month = 0;
	unsigned char day = 0;
	unsigned char week = 0;
	unsigned char tmpK = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	if(0 != buff[1])
	{
		for(i = 0;i<buff[1];i+=13)
		{
			Parser_TsPacketLog(type,"country_code = 0x%x\n",((buff[i+2]&0xFF) << 16) | ((buff[i+3]&0xFF) << 8) | (buff[i+4]&0xFF));
			Parser_TsPacketLog(type,"country_code: %c%c%c\n",buff[i+2],buff[i+3],buff[i+4]);
			Parser_TsPacketLog(type,"country_region_id = 0x%x\n",(buff[i+5]&0xFB)>>2);
			Parser_TsPacketLog(type,"local_time_offset_polarity = 0x%x\n",buff[i+5]&0x01);
			Parser_TsPacketLog(type,"local_time_offset = 0x%x\n",((buff[i+6]&0xFF) << 8) | (buff[i+7]&0xFF));
			Parser_TsPacketLog(type,"local_time_offset: %d%d:%d%d\n",(buff[i+6] & 0xF0) >> 4,(buff[i+6] & 0x0F),
												(buff[i+7] & 0xF0)>>4,(buff[i+7] & 0xF0));
			Parser_TsPacketLog(type,"time_of_change = 0x%02x%02x%02x%02x%02x\n",buff[i+8],buff[i+9],
															buff[i+10],buff[i+11],buff[i+12]);
			mjd = ((buff[i+8] & 0xFF) << 8) | (buff[i+9] & 0xFF);
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
			Parser_TsPacketLog(type,"time : %d/%02d/%02d %d%d:%d%d:%d%d weekday: %d\n",year,month,day,
												(buff[i+10]& 0xF0)>>4,(buff[i+10]& 0x0F),
												(buff[i+11]& 0xF0)>>4,(buff[i+11]& 0x0F),
												(buff[i+12]& 0xF0)>>4,(buff[i+12]& 0x0F),week);										
			Parser_TsPacketLog(type,"next_time_offset = 0x%02x%02x\n",buff[i+13],buff[i+14]);
			Parser_TsPacketLog(type,"next_time_offset: %d%d:%d%d\n",(buff[i+13] & 0xF0) >> 4,(buff[i+13] & 0x0F),
										(buff[i+14] & 0xF0)>>4,(buff[i+14] & 0xF0));
		}
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x59_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);

	for(i = 0;i<buff[1];i+=8)
	{
		Parser_TsPacketLog(type,"ISO_639_language_code = 0x%x : %c%c%c\n",((buff[i+2]&0xFF) << 16) | ((buff[i+3]&0xFF) << 8)
													| (buff[i+4]&0xFF),buff[i+2],buff[i+3],buff[i+4]);
		Parser_TsPacketLog(type,"subtitling_type = 0x%x\n",buff[i+5]);
		Parser_TsPacketLog(type,"composition_page_id = 0x%x\n",((buff[i+6]&0xFF) << 8)|(buff[i+7]&0xFF));
		Parser_TsPacketLog(type,"ancillary_page_id = 0x%x\n",((buff[i+8]&0xFF) << 8)|(buff[i+9]&0xFF));
	}

	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x5A_Tag_DescriptorParser*** 
***NIT : Terrestrial delivery system descriptor bandwidth coding***
* 000:8 MHz
* 001:7 MHz
* 010:6 MHz
* 011:5 MHz
* 100--111:Reserved for future use
***NIT : Terrestrial delivery system descriptor priority coding***
* 0:LP (low priority)
* 1:HP (high priority)
***NIT : Terrestrial delivery system descriptor constellation coding***
* 00:QPSK
* 01:16-QAM
* 10:64-QAM
* 11:Reserved for future use
***NIT : Terrestrial delivery system descriptor hierarchy_information coding***
* 000:non-hierarchical, native interleaver
* 001:α = 1, native interleaver
* 010:α = 2, native interleaver
* 011:α = 4, native interleaver
* 100:non-hierarchical, in-depth interleaver
* 101:α = 1, in-depth interleaver
* 110:α = 2, in-depth interleaver
* 111:α = 4, in-depth interleaver
***NIT : Terrestrial delivery system descriptor code_rate coding***
* 000:1/2
* 001:2/3
* 010:3/4
* 011:5/6
* 100:7/8
* 101--111:Reserved for future use
***NIT : Terrestrial delivery system descriptor guard_interval coding***
* 00:1/32
* 01:1/16
* 10:1/8
* 11:1/4
***NIT : Terrestrial delivery system descriptor transmission_mode coding***
* 00:2k mode
* 01:8k mode
* 10:4k mode
* 11:Reserved for future use
*/
int x5A_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"centre_frequency = 0x%x\n",((buff[2]&0xFF) << 24) | ((buff[3]&0xFF) << 16)
										| ((buff[4]&0xFF) << 8) | (buff[5]&0xFF));
	Parser_TsPacketLog(type,"bandwidth = 0x%x\n",(buff[6]&0xE0)>>5);
	Parser_TsPacketLog(type,"priority = 0x%x\n",(buff[6]&0x10)>>4);
	Parser_TsPacketLog(type,"Time_Slicing_indicator = 0x%x\n",(buff[6]&0x08)>>3);
	Parser_TsPacketLog(type,"MPE-FEC_indicator = 0x%x\n",(buff[6]&0x04)>>2);
	Parser_TsPacketLog(type,"constellation = 0x%x\n",(buff[7]&0xC0)>>6);
	Parser_TsPacketLog(type,"hierarchy_information = 0x%x\n",(buff[7]&0x38)>>3);
	Parser_TsPacketLog(type,"code_rate-HP_stream = 0x%x\n",buff[7]&0x07);
	Parser_TsPacketLog(type,"code_rate-LP_stream = 0x%x\n",(buff[8]&0xE0)>>5);
	Parser_TsPacketLog(type,"guard_interval = 0x%x\n",(buff[8]&0x18)>>3);
	Parser_TsPacketLog(type,"transmission_mode = 0x%x\n",(buff[8]&0x06)>>1);
	Parser_TsPacketLog(type,"other_frequency_flag = 0x%x\n",buff[8]&0x01);
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x5B_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0,j = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	
	if(0 != buff[1])
	{
		for(i = 0;i<buff[1];i+=j)
		{
			j = 0;
			Parser_TsPacketLog(type,"ISO_639_language_code = 0x%x\n",((buff[i+2]&0xFF) <<16)|((buff[i+3]&0xFF) <<8)|(buff[i+4]&0xFF));
			Parser_TsPacketLog(type,"ISO_639_language_code : %c%c%c\n",buff[i+2],buff[i+3],buff[i+4]);
			Parser_TsPacketLog(type,"network_name_length = 0x%x\n",buff[i+5]);
			if(0 != buff[i+5])
			{
				Parser_TsPacketLog(type,"network_name: ");
				for(j = 0;j<buff[i+5];j++)
				{
					Parser_TsPacketLog(type,"%c",buff[i+j+6]);
				}
				Parser_TsPacketLog(type,"\n");
			}
			j+=4;
		}
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x5C_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0,j = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	
	if(0 != buff[1])
	{
		for(i = 0;i<buff[1];i+=j)
		{
			j = 0;
			Parser_TsPacketLog(type,"ISO_639_language_code = 0x%x\n",((buff[i+2]&0xFF) <<16)|((buff[i+3]&0xFF) <<8)|(buff[i+4]&0xFF));
			Parser_TsPacketLog(type,"ISO_639_language_code : %c%c%c\n",buff[i+2],buff[i+3],buff[i+4]);
			Parser_TsPacketLog(type,"bouquet_name_length = 0x%x\n",buff[i+5]);
			if(0 != buff[i+5])
			{
				Parser_TsPacketLog(type,"bouquet_name: ");
				for(j = 0;j<buff[i+5];j++)
				{
					Parser_TsPacketLog(type,"%c",buff[i+j+6]);
				}
				Parser_TsPacketLog(type,"\n");
			}
			j+=4;
		}
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x5D_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0,j = 0;
	unsigned int offset = 0,length = 0;
	unsigned char *data = NULL;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	data = buff;
	data+=2;
	if(0 != buff[1])
	{
		for(i = 0;i<buff[1];i+=offset)
		{
			Parser_TsPacketLog(type,"ISO_639_language_code = 0x%x\n",((data[0]&0xFF) <<16)|((data[1]&0xFF) <<8)|(data[2]&0xFF));
			Parser_TsPacketLog(type,"ISO_639_language_code : %c%c%c\n",data[0],data[1],data[2]);
			Parser_TsPacketLog(type,"service_provider_name_length = 0x%x\n",data[3]);
			offset = data[3];
			data+=4;
			if(0 != offset)
			{
				Parser_TsPacketLog(type,"service_provider_name: ");
				for(j = 0;j<offset;j++)
				{
					Parser_TsPacketLog(type,"%c",data[j]);
				}
				Parser_TsPacketLog(type,"\n");
			}
			data+=offset;
			offset+=4;
			length = data[0];
			Parser_TsPacketLog(type,"service_name_length = 0x%x\n",length);
			data+=1;
			if(0 != length)
			{
				Parser_TsPacketLog(type,"service_name: ");
				for(j = 0;j<length;j++)
				{
					Parser_TsPacketLog(type,"%c",data[j]);
				}
				Parser_TsPacketLog(type,"\n");
			}
			offset+=length;
			offset+=1;
			data+=length;
		}
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x5E_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0,j = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"component_tag = 0x%x\n",buff[2]);
	
	if(buff[1]>1)
	{
		for(i = 1;i<buff[1];i+=j)
		{
			j = 0;
			Parser_TsPacketLog(type,"ISO_639_language_code = 0x%x\n",((buff[i+2]&0xFF) <<16)|((buff[i+3]&0xFF) <<8)|(buff[i+4]&0xFF));
			Parser_TsPacketLog(type,"ISO_639_language_code : %c%c%c\n",buff[i+2],buff[i+3],buff[i+4]);
			Parser_TsPacketLog(type,"text_description_length = 0x%x\n",buff[i+5]);
			if(0 != buff[i+5])
			{
				Parser_TsPacketLog(type,"text_description: ");
				for(j = 0;j<buff[i+5];j++)
				{
					Parser_TsPacketLog(type,"%c",buff[i+j+6]);
				}
				Parser_TsPacketLog(type,"\n");
			}
			j+=4;
		}
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x5F_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"private_data_specifier = 0x%x\n",((buff[2]&0xFF) << 24)
										|((buff[3]&0xFF) << 16)|((buff[4]&0xFF) << 8)|(buff[5]&0xFF));
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x60_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"new_original_network_id = 0x%x",((buff[2]&0xFF) << 8)| (buff[3]&0xFF));
	Parser_TsPacketLog(type,"new_transport_stream_id = 0x%x",((buff[4]&0xFF) << 8)| (buff[5]&0xFF));
	Parser_TsPacketLog(type,"new_service_id = 0x%x",((buff[6]&0xFF) << 8)| (buff[7]&0xFF));
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x61_Tag_DescriptorParser*** 
***EIT SIT : short_smoothing_buffer_descriptor sb_size coding***
* 0:DVB_reserved
* 1:1536
* 2:DVB_reserved
* 3:DVB_reserved
***EIT SIT : short_smoothing_buffer_descriptor sb_leak_rate coding***
* 0:DVB_reserved
* 1:0.0009
* 2:0.0018
* 3:0.0036
* 4:0.0072
* 5:0.0108
* 6:0.0144
* 7:0.0216
* 8:0.0288
* 9:0.075
* 10:0.5
* 11:0.5625
* 12:0.8437
* 13:1.0
* 14:1.1250
* 15:1.5
* 16:1.6875
* 17:2.0
* 18:2.2500
* 19:2.5
* 20:3.0
* 21:3.3750
* 22:3.5
* 23:4.0
* 24:4.5
* 25:5.0
* 26:5.5
* 27:6.0
* 28:6.5
* 29:6.7500
* 30--32:((value) - 16) × 0.5 (7,0 Mbit/s, 7,5 Mbit/s, 8,0 Mbit/s)
* 33--37:((value) - 24) (9 Mbit/s, 10 Mbit/s, 11 Mbit/s, 12 Mbit/s, 13 Mbit/s)
* 38:13.5
* 39--43:((value) - 25) (14 Mbit/s, 15 Mbit/s, 16 Mbit/s, 17 Mbit/s, 18 Mbit/s)
* 44--47:((value) - 34) × 2 (20 Mbit/s, 22 Mbit/s, 24 Mbit/s, 26 Mbit/s)
* 48:27
* 49--55:((value) - 35) × 2 (28 Mbit/s, 30 Mbit/s, 32 Mbit/s to 40 Mbit/s)
* 56:44
* 57:48
* 58:54
* 59:72
* 60:108
* 61--63:DVB_reserved
*/
int x61_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"sb_size = 0x%x",(buff[2]&0xC0) >> 6);
	Parser_TsPacketLog(type,"sb_leak_rate = 0x%x",buff[2]&0x3F);
	if(buff[1]>1)
	{
		for(i = 1;i<buff[1];i++)
		{
			Parser_TsPacketLog(type,"DVB_reserved = 0x%x\n",buff[i+2]);
		}
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x62_Tag_DescriptorParser*** 
***NIT : coding_type coding***
* 00:not defined
* 01:satellite
* 10:cable
* 11:terrestrial
*/
int x62_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"coding_type = 0x%x\n",buff[2]&0x03);
	if(buff[1]>1)
	{
		for(i = 1;i<buff[1];i+=4)
		{
			Parser_TsPacketLog(type,"centre_frequency = 0x%x\n",((buff[i+2]&0xFF) << 24) | ((buff[i+3]&0xFF) << 16)
										| ((buff[i+4]&0xFF) << 8) | (buff[i+5]&0xFF));
		}
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x63_Tag_DescriptorParser*** 
***SIT : Partial Transport Stream (TS) descriptor coding***
*/
int x63_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"peak_rate = 0x%x\n",((buff[2]&0x3F) << 16)|((buff[3]&0xFF) << 8)|(buff[4]&0xFF));
	Parser_TsPacketLog(type,"minimum_overall_smoothing_rate = 0x%x\n",((buff[5]&0x3F) << 16)|((buff[6]&0xFF) << 8)|(buff[7]&0xFF));
	Parser_TsPacketLog(type,"maximum_overall_smoothing_buffer = 0x%x\n",((buff[8]&0x3F) << 8)|(buff[9]&0xFF));
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x64_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	unsigned char selector_length = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"data_broadcast_id = 0x%x\n",(buff[2] << 8)| buff[3]);
	Parser_TsPacketLog(type,"component_tag = 0x%x\n",buff[4]);
	selector_length = buff[5];
	Parser_TsPacketLog(type,"selector_length = 0x%x\n",selector_length);
	if(0 != selector_length)
	{
		Parser_TsPacketLog(type,"selector_data:");
		for(i = 0;i<selector_length;i++)
		{
			Parser_TsPacketLog(type," %02x",buff[i+6]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	
	Parser_TsPacketLog(type,"ISO_639_language_code = 0x%x\n",((buff[selector_length+6]&0xFF) <<16)
									| ((buff[selector_length+7]&0xFF) <<8) | (buff[selector_length+8]&0xFF));
	Parser_TsPacketLog(type,"ISO_639_language_code : %c%c%c\n",buff[selector_length+6],buff[selector_length+7],buff[selector_length+8]);
	Parser_TsPacketLog(type,"text_length = 0x%x\n",buff[selector_length+9]);
	if(0 != buff[selector_length+9])
	{
		Parser_TsPacketLog(type,"text_char: ");
		for(i = 0;i<buff[selector_length+9];i++)
		{
			Parser_TsPacketLog(type,"%c",buff[selector_length+10+i]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x65_Tag_DescriptorParser*** 
***PMT : scrambling_mode coding***
* 0x00:Reserved for future use
* 0x01:This value indicates use of DVB-CSA1. It is the default mode and shall be used
* when the scrambling descriptor is not present in the program map section
* 0x02:This value indicates use of DVB-CSA2
* 0x03:This value indicates use of DVB-CSA3 in standard mode
* 0x04:This value indicates use of DVB-CSA3 in minimally enhanced mode
* 0x05:This value indicates use of DVB-CSA3 in fully enhanced mode
* 0x06--0x0F:Reserved for future use
* 0x10:This value indicates use of DVB-CISSA version 1
* 0x11--0x1F:Reserved for future DVB-CISSA versions
* 0x20--0x6F:Reserved for future use
* 0x70--0x7F:ATIS defined (ATIS-0800006, see annex J)
* 0x80--0xFE:User defined
* 0xFF:Reserved for future use
*/
int x65_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"scrambling_mode = 0x%x\n",buff[2]);
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x66_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"data_broadcast_id = 0x%x\n",(buff[2] << 8)| buff[3]);
	if(buff[1]>2)
	{
		Parser_TsPacketLog(type,"id_selector_byte:");
		for(i = 2;i<buff[1];i++)
		{
			Parser_TsPacketLog(type," 0x%x",buff[i+2]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x67_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	if(0 != buff[1])
	{
		Parser_TsPacketLog(type,"Transport stream descriptor:");
		for(i = 0;i<buff[1];i++)
		{
			Parser_TsPacketLog(type," 0x%x",buff[i+2]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x68_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	if(0 != buff[1])
	{
		Parser_TsPacketLog(type,"DSNG descriptor:");
		for(i = 0;i<buff[1];i++)
		{
			Parser_TsPacketLog(type," 0x%x",buff[i+2]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x69_Tag_DescriptorParser*** 
***EIT : PDC_descriptor programme_identification_label coding***
* 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
* M       L M     L M           L  M              L
*    day     month      hour            minute
*/
int x69_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"programme_identification_label = 0x%x\n",((buff[2]&0x0F) << 16)
										|((buff[3]&0xFF) << 8)|(buff[4]&0xFF));
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x6B_Tag_DescriptorParser*** 
***PMT : Ancillary_data_identifier coding***
* b0:DVD-Video Ancillary Data (ETSI TS 101 154 [9])
* b1:Extended Ancillary Data (ETSI TS 101 154 [9])
* b2:Announcement Switching Data (ETSI TS 101 154 [9])
* b3:DAB Ancillary Data (ETSI EN 300 401 [2])
* b4:Scale Factor Error Check (ScF-CRC) (ETSI TS 101 154 [9])
* b5:MPEG-4 ancillary data (ETSI TS 101 154 [9], clause C.5)
* b6:RDS via UECP (ETSI TS 101 154 [9])
* b7:Reserved for future use
*/
int x6B_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"ancillary_data_identifier = 0x%x\n",buff[2]);

	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x6C_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0,j = 0;
	unsigned char subcell_info_loop_length = 0;
	unsigned char offset = 0; 
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	
	for(i = 0;i<buff[1];i+=offset)
	{
		Parser_TsPacketLog(type,"cell_id = 0x%x\n",((buff[i+2]&0xFF) << 8) | (buff[i+3]&0xFF));
		Parser_TsPacketLog(type,"cell_latitude = 0x%x\n",((buff[i+4]&0xFF) << 8) | (buff[i+5]&0xFF));
		Parser_TsPacketLog(type,"cell_longitude = 0x%x\n",((buff[i+6]&0xFF) << 8) | (buff[i+7]&0xFF));
		Parser_TsPacketLog(type,"cell_extent_of_latitude = 0x%x\n",((buff[i+8]&0xFF) << 4) | ((buff[i+9]&0xF0) >> 4));
		Parser_TsPacketLog(type,"cell_extent_of_longitude = 0x%x\n",((buff[i+9]&0x0F) << 8) | (buff[i+10]&0xFF));
		subcell_info_loop_length = buff[i+11]&0xFF;
		Parser_TsPacketLog(type,"subcell_info_loop_length = 0x%x\n",subcell_info_loop_length);
		for(j = 0;j<subcell_info_loop_length;j+=8)
		{
			Parser_TsPacketLog(type,"cell_id_extension = 0x%x\n",buff[i+j+12]&0xFF);
			Parser_TsPacketLog(type,"subcell_latitude = 0x%x\n",((buff[i+j+13]&0xFF) << 8) | (buff[i+j+14]&0xFF));
			Parser_TsPacketLog(type,"subcell_longitude = 0x%x\n",((buff[i+j+15]&0xFF) << 8) | (buff[i+j+16]&0xFF));
			Parser_TsPacketLog(type,"subcell_extent_of_latitude = 0x%x\n",((buff[i+j+17]&0xFF) << 4) | ((buff[i+j+18]&0xF0) >> 4));
			Parser_TsPacketLog(type,"subcell_extent_of_longitude = 0x%x\n",((buff[i+j+18]&0x0F) << 8) | (buff[i+j+19]&0xFF));
		}
		offset = subcell_info_loop_length+10;
	}

	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x6D_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0,j = 0;
	unsigned char subcell_info_loop_length = 0;
	unsigned char offset = 0; 
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	
	for(i = 0;i<buff[1];i+=offset)
	{
		Parser_TsPacketLog(type,"cell_id = 0x%x\n",((buff[i+2]&0xFF) << 8) | (buff[i+3]&0xFF));
		Parser_TsPacketLog(type,"frequency = 0x%x\n",((buff[i+4]&0xFF) << 24) | ((buff[i+5]&0xFF) << 16)
												|((buff[i+6]&0xFF) << 8) | (buff[i+7]&0xFF));
		subcell_info_loop_length = buff[i+8]&0xFF;
		Parser_TsPacketLog(type,"subcell_info_loop_length = 0x%x\n",subcell_info_loop_length);
		for(j = 0;j<subcell_info_loop_length;j+=5)
		{
			Parser_TsPacketLog(type,"cell_id_extension = 0x%x\n",buff[i+j+9]&0xFF);
			Parser_TsPacketLog(type,"transposer_frequency = 0x%x\n",((buff[i+j+10]&0xFF) << 24) | ((buff[i+j+11]&0xFF) << 16)
												|((buff[i+j+12]&0xFF) << 8) | (buff[i+j+13]&0xFF));
		}
		offset = subcell_info_loop_length+7;
	}

	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x6E_Tag_DescriptorParser***
***SDT : Announcement_support_indicator coding***
* b0:Emergency alarm
* b1:Road Traffic flash
* b2:Public Transport flash
* b3:Warning message
* b4:News flash
* b5:Weather flash
* b6:Event announcement
* b7:Personal call
* b8--b15:Reserved for future use
***SDT : announcement_type coding***
* 0000:Emergency alarm
* 0001:Road Traffic flash
* 0010:Public Transport flash
* 0011:Warning message
* 0100:News flash
* 0101:Weather flash
* 0110:Event announcement
* 0111:Personal call
* 1000--1111:Reserved for future use
***SDT : reference type coding***
* 000:Announcement is broadcast in the usual audio stream of the service
* 001:Announcement is broadcast in a separate audio stream that is part of the service
* 010:Announcement is broadcast by means of a different service within the same transport stream
* 011:Announcement is broadcast by means of a different service within a different transport stream
* 100--111:Reserved for future use
*/
int x6E_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	unsigned char reference_type = 0;
	unsigned char offset = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"announcement_support_indicator = 0x%x\n",((buff[2]&0xFF) << 8) | (buff[3]&0xFF));
	for(i = 2;i<buff[1];i+=offset)
	{
		Parser_TsPacketLog(type,"announcement_type = 0x%x\n",buff[4]&0xF0);
		reference_type = buff[4]&0x07;
		Parser_TsPacketLog(type,"reference_type = 0x%x\n",reference_type);
		if((0x01 == reference_type) || (0x02 == reference_type) || (0x03 == reference_type))
		{
			Parser_TsPacketLog(type,"original_network_id = 0x%x\n",((buff[5]&0xFF) << 8) | (buff[6]&0xFF));
			Parser_TsPacketLog(type,"transport_stream_id = 0x%x\n",((buff[7]&0xFF) << 8) | (buff[8]&0xFF));
			Parser_TsPacketLog(type,"service_id = 0x%x\n",((buff[9]&0xFF) << 8) | (buff[10]&0xFF));
			Parser_TsPacketLog(type,"component_tag = 0x%x\n",buff[11]);
		}
		else
		{
			
		}
	}

	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x70_Tag_DescriptorParser***
***PMT : Adaptation field data identifier coding***
* b0:announcement switching data field (ETSI TS 101 154 [9])
* b1:AU_information data field (ETSI TS 101 154 [9])
* b2:PVR_assist_information_data_field (ETSI TS 101 154 [9])
* b3:tsap_timeline (DVB BlueBook A167-2 [i.10])
* b4--b7: Reserved for future use
*/
int x70_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"adaptation_field_data_identifier = 0x%x\n",buff[2]);

	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x72_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	unsigned char *data = NULL;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"availability_flag = 0x%x\n",(buff[2]&0x80) >> 7);
	data = buff;
	data+=2;
	if(buff[1] > 1)
	{
		for(i = 1;i<buff[1];i+=2)
		{
			Parser_TsPacketLog(type,"cell_id = 0x%x\n",((data[i]&0xFF) << 8)| (data[i+1]&0xFF));
		}
	}
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x79_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"scrambling_sequence_selector = 0x%x\n",(buff[2]&0x80)>>7);
	Parser_TsPacketLog(type,"multiple_input_stream_flag = 0x%x\n",(buff[2]&0x40)>>6);
	Parser_TsPacketLog(type,"backwards_compatibility_indicator = 0x%x\n",(buff[2]&0x20)>>5);
	if(0x80 == (buff[2]&0x80))
	{
		Parser_TsPacketLog(type,"scrambling_sequence_index = 0x%x\n",((buff[3]&0xFF) << 16)|((buff[4]&0xFF) << 8)|(buff[5]&0xFF));
	}
	if(0x40 == (buff[2]&0x80))
	{
		if(0x80 == (buff[2]&0x80))
		{
			Parser_TsPacketLog(type,"input_stream_identifier = 0x%x\n",buff[6]&0xFF);
		}
		else
		{
			Parser_TsPacketLog(type,"input_stream_identifier = 0x%x\n",buff[3]&0xFF);
		}
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

/***x7E_Tag_DescriptorParser***
***NIT BAT SDT EIT : FTA content management descriptor control_remote_access_over_internet coding***
* 00:Redistribution over the Internet is enabled.
* 01:Redistribution over the Internet is enabled but only within a managed domain.
* 10:Redistribution over the Internet is enabled but only within a managed domain and after a certain short period of time (e.g. 24 hours).
* 11:Redistribution over the Internet is not allowed with the following exception:
*    Redistribution over the Internet within a managed domain is enabled after a specified long (possibly indefinite) period of time.
*/
int x7E_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"user_defined = 0x%x\n",(buff[2]&0x80)>>7);
	Parser_TsPacketLog(type,"do_not_scramble = 0x%x\n",(buff[2]&0x08)>>3);
	Parser_TsPacketLog(type,"control_remote_access_over_internet = 0x%x\n",(buff[2]&0x06)>>1);
	Parser_TsPacketLog(type,"do_not_apply_revocation = 0x%x\n",buff[2]&0x01);
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x7F_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	Parser_TsPacketLog(type,"Descriptor_length = 0x%x\n",buff[1]);
	Parser_TsPacketLog(type,"descriptor_tag_extension = 0x%x\n",buff[2]);

	if(buff[1]>1)
	{
		Parser_TsPacketLog(type,"selector_byte:");
		for(i = 1;i<buff[1];i++)
		{
			Parser_TsPacketLog(type," 0x%02x",buff[i+2]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

int x00_Tag_DescriptorParser(unsigned int type,unsigned char* buff)
{
	unsigned int i = 0;
	if(NULL == buff)
	{
		return -1;
	}
	Parser_DisplayTagInfo(type,buff[0]);
	if(0 != buff[1])
	{
		Parser_TsPacketLog(type,"user_private_define_byte:");
		for(i = 0;i<buff[1];i++)
		{
			Parser_TsPacketLog(type," 0x%02x",buff[i+2]);
		}
		Parser_TsPacketLog(type,"\n");
	}
	
	Parser_TsPacketLog(type,"******END******\n");
	return buff[1];
}

