#ifndef SPI_MISC_H__
#define SPI_MISC_H__

/* Debug function */
#ifdef _DEBUG
inline void SpiTrace( LPCSTR format, ... )
{
	TCHAR msg[ MAX_PATH ];
	va_list argList;
	va_start( argList, format );
	wvsprintf( msg, format, argList );
	va_end( argList );
	OutputDebugString( msg );
}

#endif	// _DEBUG

/* ----------------------------------------------------------------------------------------------------------- */
/* Auto Local Heap */
class LocalHeap
{
private:
	HLOCAL m_hLocal;
	LocalHeap( const LocalHeap& obj );	// コピー禁止
	const LocalHeap& operator = ( const LocalHeap& other );

public:
	// 構築・消滅
	LocalHeap() : m_hLocal( NULL )
	{}

	~LocalHeap()
	{
		if( m_hLocal != NULL )
			Free();
	}

	// アトリビュート
	void* GetData() const
	{
		if( m_hLocal == NULL )
			return NULL;
		return LocalLock( m_hLocal );
	}

	// オペレーション
	void* Alloc( SIZE_T size, DWORD flag = LHND )
	{
		m_hLocal = ::LocalAlloc( flag, size );
		if( flag & LMEM_MOVEABLE )
			return ::LocalLock( m_hLocal );
		else
			return m_hLocal;
	}

	void* ReAlloc( SIZE_T size, DWORD flag = LHND )
	{
		HLOCAL hLocal = ::LocalReAlloc( m_hLocal, size, flag );
		if( hLocal == NULL )
			return NULL;
		m_hLocal = hLocal;
		if( flag & LMEM_MOVEABLE )
			return ::LocalLock( m_hLocal );
		else
			return m_hLocal;
	}

	void Free()
	{
		::LocalFree( m_hLocal );
		m_hLocal = NULL;
	}

	void* Attach( HLOCAL hLocal )
	{
		if( m_hLocal != NULL )
			Free();

		m_hLocal = hLocal;
		return ::LocalLock( m_hLocal );
	}

	HLOCAL Detach()
	{
		HLOCAL hLocal = m_hLocal;
		::LocalUnlock( m_hLocal );
		m_hLocal = NULL;
		return hLocal;
	}
};

#endif // SPI_MISC_H__