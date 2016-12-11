#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "TsPacket_Parser.h"


static unsigned int display_type = 0;


void Parser_SetDisplayTagInfo(unsigned int type)
{
	display_type = type;
}

int Parser_TsPacketLog(unsigned int type,const char  *format, ...)
{
	int ret = 0;
	char buffer[MAXBUFFER_LOG] = {0};

	va_list list;
	va_start(list, format);
	ret = vsprintf(&buffer[0], format, list);
	va_end(list);
	if (ret < 0) 
	{
		return ret;
	}
	if((DISPLAY_VALID & display_type) != DISPLAY_VALID)
	{
		return ret;
	}
	if((display_type & type) == type)
	{
		ret = printf("%s", buffer);
	}
	return ret;
}