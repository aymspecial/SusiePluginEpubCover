#include "Unzmini.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void
getExtName( char* extName, char* fileName )
{
	auto len = strlen( fileName );
	auto iStart = len - 1;

	for( ; ; iStart-- )
	{
		if( fileName[ iStart ] == '.' )
		{
			iStart++;
			break;
		}

		if( iStart == 0 )
			return;
	}

	for( unsigned int i = 0; i < len; i++ )
	{
		extName[ i ] = fileName[ iStart + i ];
	}
}


int
do_extract_currentfile( unzFile uf, const int* popt_extract_without_path, char* write_filename,
	int* popt_overwrite )
{
	char filename_inzip[ 256 ];
	char* filename_withoutpath;
	char* p;
	int err = UNZ_OK;
	FILE* fout = NULL;
	void* buf;
	uInt size_buf;

	unz_file_info64 file_info;
	uLong ratio = 0;

	strcpy( filename_inzip, write_filename );
	err = unzGetCurrentFileInfo64( uf, &file_info, filename_inzip, sizeof( filename_inzip ), NULL, 0, NULL, 0 );

	if( err != UNZ_OK )
	{
		SpiTrace( "error %d with zipfile in unzGetCurrentFileInfo\n", err );
		return err;
	}

	size_buf = WRITEBUFFERSIZE;
	buf = (void*)malloc( size_buf );
	if( buf == NULL )
	{
		SpiTrace( "Error allocating memory\n" );
		return UNZ_INTERNALERROR;
	}

	p = filename_withoutpath = filename_inzip;
	while( ( *p ) != '\0' )
	{
		if( ( ( *p ) == '/' ) || ( ( *p ) == '\\' ) )
			filename_withoutpath = p + 1;
		p++;
	}
	char tempPath[ MAX_PATH ];
	::GetTempPath( MAX_PATH, tempPath );

	int skip = 0;

	strcpy( write_filename, tempPath );
	strcat( write_filename, filename_withoutpath );

	err = unzOpenCurrentFilePassword( uf, NULL );
	if( err != UNZ_OK )
	{
		SpiTrace( "error %d with zipfile in unzOpenCurrentFilePassword\n", err );
	}

	if( ( skip == 0 ) && ( err == UNZ_OK ) )
	{
		fout = fopen64( write_filename, "wb" );

		/* some zipfile don't contain directory alone before file */
		if( ( fout == NULL ) && ( ( *popt_extract_without_path ) == 0 ) &&
			( filename_withoutpath != (char*)filename_inzip ) )
		{
			char c = *( filename_withoutpath - 1 );
			*( filename_withoutpath - 1 ) = '\0';
			*( filename_withoutpath - 1 ) = c;
			fout = fopen64( write_filename, "wb" );
		}

		if( fout == NULL )
		{
			SpiTrace( "error opening %s\n", write_filename );
		}
	}

	if( fout != NULL )
	{
		SpiTrace( " extracting: %s\n", write_filename );

		do
		{
			err = unzReadCurrentFile( uf, buf, size_buf );
			if( err < 0 )
			{
				SpiTrace( "error %d with zipfile in unzReadCurrentFile\n", err );
				break;
			}
			if( err > 0 )
				if( fwrite( buf, err, 1, fout ) != 1 )
				{
					SpiTrace( "error in writing extracted file\n" );
					err = UNZ_ERRNO;
					break;
				}
		}
		while( err > 0 );

		if( fout )
			fclose( fout );
	}

	if( err == UNZ_OK )
	{
		err = unzCloseCurrentFile( uf );
		if( err != UNZ_OK )
		{
			SpiTrace( "error %d with zipfile in unzCloseCurrentFile\n", err );
		}
	}
	else
		unzCloseCurrentFile( uf ); /* don't lose the error */

	free( buf );
	return err;
}

int
do_extract_onefile( unzFile uf, const char* inFilename, char* tempFileName,
	int opt_extract_without_path,
	int opt_overwrite )
{
	int err = UNZ_OK;
	if( unzLocateFile( uf, inFilename, CASESENSITIVITY ) != UNZ_OK )
	{
		SpiTrace( "file %s not found in the zipfile\n", inFilename );
		return 2;
	}

	strcpy( tempFileName, inFilename );

	if( do_extract_currentfile( uf, &opt_extract_without_path, tempFileName,
		&opt_overwrite ) == UNZ_OK )
		return 0;
	else
		return 1;
}

int
Unzmini::extractOpfFile( char* opfFile, char* opfPath )
{
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64A( &ffunc );

	auto uf = unzOpen2_64( zipFile, &ffunc );
	if( uf == NULL )
	{
		SpiTrace( "Cannot open : %s\n", zipFile );
		return SPI_FILE_READ_ERROR;
	}
	SpiTrace( "%s opened\n", zipFile );

	unz_global_info64 gi;
	int err;

	err = unzGetGlobalInfo64( uf, &gi );
	if( err != UNZ_OK )
	{
		SpiTrace( "error %d with zipfile in unzGetGlobalInfo \n", err );
		return SPI_FILE_READ_ERROR;
	}

	// 一通りファイル一覧を舐める
	bool bFirst = true;
	for( int i = 0; i < gi.number_entry; i++ )
	{
		char inFileName[ MAX_PATH ];

		unz_file_info64 file_info;

		err = unzGetCurrentFileInfo64( uf, &file_info, inFileName, sizeof( inFileName ), NULL, 0, NULL, 0 );
		if( err != UNZ_OK )
		{
			SpiTrace( "error %d with zipfile in unzGetCurrentFileInfo\n", err );
			break;
		}

		char extName[ MAX_PATH ]{ 0 };
		char tempName[ MAX_PATH ]{ 0 };

		getExtName( extName, inFileName );
		if( !strcmp( extName, "opf" ) )
		{
			auto ret = do_extract_onefile( uf, inFileName, tempName, 1, 1 );
			if( ret != 0 )
				return SPI_FILE_READ_ERROR;

			strcpy_s( opfFile, MAX_PATH, tempName );

			// opf のパスを取り出す
			strcpy( opfPath, inFileName );
			for( int ip = strlen( opfPath ) - 1; 0 <= ip; ip-- )
			{
				if( opfPath[ ip ] == '/' )
				{
					opfPath[ ip + 1 ] = '\0';
					break;
				}
				opfPath[ ip ] = '\0';
			}

			break;
		}

		if( ( i + 1 ) < gi.number_entry )
		{
			err = unzGoToNextFile( uf );
			if( err != UNZ_OK )
			{
				SpiTrace( "error %d with zipfile in unzGoToNextFile\n", err );
				break;
			}
		}
	}



	unzClose( uf );

	return SPI_ALL_RIGHT;
}
#include "tinyxml2.h"
using namespace tinyxml2;

int
Unzmini::getCoverImageName( char* imageName, char* opfFile, char* opfPath )
{
	int ret = SPI_NOT_SUPPORT;

	tinyxml2::XMLDocument doc;
	doc.LoadFile( opfFile );
	auto error = doc.ErrorID();

	// Trial #1 metadata->meta name="cover" となっている content を探す
	const char* nameAttr = NULL;
	const char* coverIdName = NULL;
	XMLElement* metaElement = doc.FirstChildElement( "package" )->FirstChildElement( "metadata" )->FirstChildElement( "meta" );
	if( metaElement == NULL )
		goto TRIAL2;

	do
	{
		nameAttr = metaElement->Attribute( "name" );
		if( nameAttr && !strcmp( nameAttr, "cover" ) )
		{
			coverIdName = metaElement->Attribute( "content" );
			break;
		}
		metaElement = metaElement->NextSiblingElement( "meta" );
	}
	while( metaElement != NULL );


TRIAL2:
	// Trial #2 package->manifest->item の中で properties="cover-image" となっている href を探す
	if ( coverIdName == NULL )
		coverIdName = "cover";

	XMLElement * manifestElement = doc.FirstChildElement( "package" )->FirstChildElement( "manifest" );

	int itemCount = manifestElement->ChildElementCount();
	XMLElement * itemElem = manifestElement = manifestElement->FirstChildElement( "item" );

	for( int i = 0; i < itemCount; i++ )
	{
		auto propAttr = itemElem->Attribute( "properties" );
		auto idAttr = itemElem->Attribute( "id" );
		if( ( propAttr && !strcmp( propAttr, coverIdName ) ) ||
			( idAttr && ( !strcmp( idAttr, coverIdName ) || !strcmp( idAttr, "cover" ) ) ) )
		{
			char extName[ MAX_PATH ]{ 0 };

			strcpy( imageName, opfPath );
			strcat( imageName, itemElem->Attribute( "href" ) );

			getExtName( extName, imageName );
			if( strcmp( extName, "jpg" ) && strcmp( extName, "jpeg" ) && strcmp( extName, "gif" ) &&
				strcmp( extName, "png" ) )
				break;

			return SPI_ALL_RIGHT;
		}
		itemElem = itemElem->NextSiblingElement();
	}

	// '<manifest>' -> トップ HTML に含まれる ImageName 

	return ret;
}


int
Unzmini::getCoverNameFromOpf()
{
	int ret = SPI_NOT_SUPPORT;

	// *.opf ファイル名の取得 'content.opf' が優先
	char opfFile[ MAX_PATH ]{ 0 };
	char opfPath[ MAX_PATH ]{ 0 };
	ret = extractOpfFile( opfFile, opfPath );

	if( ret != SPI_ALL_RIGHT )  // 失敗すればエラーで抜ける
		return ret;

	ret = getCoverImageName( thumbFile, opfFile, opfPath );

	if( ret != SPI_ALL_RIGHT )  // 成功すれば成功で抜ける
		return ret;

	return ret;
}

int
Unzmini::GetCoverImageName()
{
	int ret = -1;


	ret = getCoverNameFromOpf();

	// Opf を見てもわからなければ最後に画像ファイル名を並べ替えて一番若いのを CoverImage とする
	if( ret != SPI_ALL_RIGHT )
		ret = getTopNamedImageName();

	return ret;
}

int
Unzmini::GetBitmap( HANDLE* pHBInfo, HANDLE* pHBm )
{
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64A( &ffunc );

	auto uf = unzOpen2_64( zipFile, &ffunc );
	if( uf == NULL )
	{
		SpiTrace( "Cannot open : %s\n", zipFile );
		return NULL;
	}
	SpiTrace( "%s opened\n", zipFile );

	char tempFileName[ MAX_PATH ];
	auto ret = do_extract_onefile( uf, thumbFile, tempFileName, 1, 1 );
	unzClose( uf );

	unsigned char* pixels;
	int width;
	int height;
	int bpp;

	// ファイルを読み込み、画像データを取り出す
	pixels = stbi_load( tempFileName, &width, &height, &bpp, 3 );

	// BMP情報のセット
	const DWORD bmpInfoSize = sizeof( BITMAPINFOHEADER );
	LocalHeap lhInfo;
	BITMAPINFO* pInfo = reinterpret_cast<BITMAPINFO*>( lhInfo.Alloc( bmpInfoSize ) );
	if( pInfo == NULL )
		return SPI_NO_MEMORY;

	int outWidth = ( width + 3 ) / 4 * 4;
	pInfo->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	pInfo->bmiHeader.biWidth = outWidth;
	pInfo->bmiHeader.biHeight = -height;  // top-down image
	pInfo->bmiHeader.biPlanes = 1;
	pInfo->bmiHeader.biBitCount = 24;     // RGB image
	pInfo->bmiHeader.biCompression = BI_RGB;

	DWORD imageSize = outWidth * height * 3;

	// イメージの展開
	LocalHeap lhImage;
	BYTE* newPixels = reinterpret_cast<BYTE*>( lhImage.Alloc( imageSize ) );
	if( newPixels == NULL )
		return SPI_NO_MEMORY;

	for( int ih = 0; ih < height; ih++ )
	{
		for( int iw = 0; iw < width; iw++ )
		{
			unsigned int pOld = ( width * ih + iw ) * 3;
			unsigned int pNew = ( outWidth * ih + iw ) * 3;
			newPixels[ pNew + 0 ] = pixels[ pOld + 2 ];
			newPixels[ pNew + 1 ] = pixels[ pOld + 1 ];
			newPixels[ pNew + 2 ] = pixels[ pOld + 0 ];
		}
	}

	// メモリをアンロック or 解放
	*pHBInfo = lhInfo.Detach();	// Susieに返すハンドルをセット
	*pHBm = lhImage.Detach();

	stbi_image_free( pixels );
	_unlink( tempFileName );

	return SPI_ALL_RIGHT;
}

int Unzmini::getTopNamedImageName()
{
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64A( &ffunc );

	auto uf = unzOpen2_64( zipFile, &ffunc );
	if( uf == NULL )
	{
		SpiTrace( "Cannot open : %s\n", zipFile );
		return SPI_FILE_READ_ERROR;
	}
	SpiTrace( "%s opened\n", zipFile );

	unz_global_info64 gi;
	int err;

	err = unzGetGlobalInfo64( uf, &gi );
	if( err != UNZ_OK )
	{
		SpiTrace( "error %d with zipfile in unzGetGlobalInfo \n", err );
		return SPI_FILE_READ_ERROR;
	}

	// 一通りファイル一覧を舐めてトップの名前を探す
	bool bFirst = true;
	for( int i = 0; i < gi.number_entry; i++ )
	{
		char inFileName[ MAX_PATH ];

		unz_file_info64 file_info;

		err = unzGetCurrentFileInfo64( uf, &file_info, inFileName, sizeof( inFileName ), NULL, 0, NULL, 0 );
		if( err != UNZ_OK )
		{
			SpiTrace( "error %d with zipfile in unzGetCurrentFileInfo\n", err );
			break;
		}

		char extName[ MAX_PATH ]{ 0 };

		getExtName( extName, inFileName );

		if( !strcmp( extName, "png" ) || !strcmp( extName, "jpg" ) || !strcmp( extName, "jpeg" ) ||
			!strcmp( extName, "gif" ) || !strcmp( extName, "tiff" ) )
		{
			// 最初だけ仮トップとする
			if( bFirst )
			{
				strcpy_s( thumbFile, inFileName );
				bFirst = false;
			}
			else if( strcmp( inFileName, thumbFile ) < 0 )
			{
				strcpy_s( thumbFile, inFileName );
				SpiTrace( "change top:%s\n", thumbFile );
			}
		}

		if( ( i + 1 ) < gi.number_entry )
		{
			err = unzGoToNextFile( uf );
			if( err != UNZ_OK )
			{
				SpiTrace( "error %d with zipfile in unzGoToNextFile\n", err );
				break;
			}
		}
	}

	unzClose( uf );

	return SPI_ALL_RIGHT;
}
