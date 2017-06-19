/** \file utils.cpp
	* Fonctions utilitaires
	*
	* \author levieux guillaume
	* \date 2005
	*/


#ifdef MSVC
#include <windows.h>
#include <Psapi.h>
#endif

#include "ny_utils.h"
#include <GL/glew.h>
#include <GL/gl.h>
#include <cstdio>

#include <boost/locale/encoding_utf.hpp>



using boost::locale::conv::utf_to_utf;

#define CHEMIN_LOG "log.txt"

/****************************************************************************/
/*GETFILESIZE*/
/*****************************************************************************/
uint32 getFileSize(const char* name)
{
	#ifndef SEEK_END
	#define SEEK_END 2
	#endif

	FILE* File = fopen(name, "rb");
	if(!File)
		return 0;

	fseek(File, 0, SEEK_END);
	uint32 eof_ftell = ftell(File);
	fclose(File);
	return eof_ftell;
}

uint16 fgetu16 (FILE * fe)
{
	if(!fe)
		return 0;

	uint16 res = 0;

	for(int i=0;i<2;i++)
	{
		if(feof(fe))
			return 0;

		res <<= 8;
		res |= fgetc(fe);
	}


	return res;
}

uint32 fgetu32 (FILE * fe)
{
	if(!fe)
		return 0;

	uint32 res = 0;

	for(int i=0;i<4;i++)
	{
		if(feof(fe))
			return 0;

		res <<= 8;
		res |= fgetc(fe);
	}


	return res;
}


bool isPowerOfTwo(uint32 value)
{
	uint16 count = 0;
	while(value)
	{
		if(value & 0x0001)
			count++;
		value>>=1;
	}

	if(count == 1)
		return true;

	return false;
}

unsigned long getMemoryUsage(void)
{
    //TODONY Get Process Memory
    #ifdef MSVC
	PROCESS_MEMORY_COUNTERS pmc;
	if ( GetProcessMemoryInfo( GetCurrentProcess(), &pmc, sizeof(pmc)) )
	{
		return (unsigned long)  pmc.WorkingSetSize;
	}
	#endif
	return 0;
}

/*string ws_to_s(wstring in)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return string(converter.to_bytes(in.c_str()));
}*/

std::wstring utf8_to_wstring(const std::string& str)
{
    return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
}

std::string wstring_to_utf8(const std::wstring& str)
{
    return utf_to_utf<char>(str.c_str(), str.c_str() + str.size());
}


