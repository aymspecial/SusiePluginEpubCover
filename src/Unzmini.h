#pragma once
#define _X86_
#include <vector>
#include <string>
#include <direct.h>
#include <io.h>
#include <fileapi.h>

#include "minizip/unzip.h"
#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)
#define USEWIN32IOAPI
#include "minizip/iowin32.h"

#include "spimisc.h"
#include "spi00in.h"
#include "Unzmini.h"

class Unzmini
{
private:
	char zipFile[ MAX_PATH ] { 0 };
	char thumbFile[ MAX_PATH ] { 0 };
public:
	Unzmini( const char* _zipFile )
	{
		strcpy_s( zipFile, _zipFile );
	};

	int GetTopImageName();
	int GetBitmap( HANDLE* pHBInfo, HANDLE* pHBm );
};

