#include <windows.h>
#include "spi00in.h"
#include "spimisc.h"
#include "Unzmini.h"

BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	return TRUE;
}

int __stdcall GetPluginInfo( int infono, LPSTR buf, int buflen )
{
	if( infono < 0 || infono >= ( sizeof( pluginfo ) / sizeof( char* ) ) )
		return 0;

	auto pRet = lstrcpyn( buf, pluginfo[ infono ], buflen );

	return lstrlen( buf );
}

int __stdcall IsSupported( LPSTR filename, DWORD dw )
{
	return TRUE;
}

int __stdcall GetPictureInfo( LPSTR buf, long len, unsigned int flag, struct PictureInfo* lpInfo )
{
	return SPI_ALL_RIGHT;
}

int __stdcall GetPreview(
	LPSTR buf, long len, unsigned int flag,
	HANDLE* pHBInfo, HANDLE* pHBm,
	SPI_PROGRESS lpPrgressCallback, long lData )
{
	return SPI_NO_FUNCTION;
}

int __stdcall GetPicture(
	LPSTR buf, long len, unsigned int flag,
	HANDLE* pHBInfo, HANDLE* pHBm,
	SPI_PROGRESS lpPrgressCallback, long lData )
{
	if( flag != 0 )
		return SPI_NOT_SUPPORT;

	SpiTrace( "%s\n", buf );

	auto unz = new Unzmini( buf );

	// zip �����̏�ʃt�@�C�������擾
	int ret = unz->GetTopImageName();

	// ��ԏ�ʂ̃t�@�C������ Zip �t�@�C������
	ret = unz->GetBitmap( pHBInfo, pHBm );

	//// �I������
	delete unz;
	
	return SPI_ALL_RIGHT;
}
