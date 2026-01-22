#pragma once
#include <string>

/**
 * \ingroup Utils
 * A base window class.
 * Provides separate window message handlers for every window object based on
 * this class.
 */
class CWindow
{
public:
	virtual bool RegisterWindow( UINT style, HICON hIcon, HCURSOR hCursor, HBRUSH hbrBackground, LPCWSTR lpszMenuName,
								 LPCWSTR lpszClassName, HICON hIconSm );
	virtual bool RegisterWindow( CONST WNDCLASSEX* wcx );

	/// static message handler to put in WNDCLASSEX structure
	static LRESULT CALLBACK stWinMsgHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	/**
	 * Sets the window title.
	 */
	void SetWindowTitle( const std::wstring& sTitle )
	{
		sWindowTitle = sTitle;
	};

	void SetRegistryPath( const std::wstring& sPath )
	{
		size_t slashPos = sPath.find_last_of( '\\' );
		sRegistryPath   = sPath.substr( 0, slashPos );
		sRegistryValue  = sPath.substr( slashPos + 1 );
	}

	/**
	 * Sets the transparency of the window.
	 * \remark note that this also sets the WS_EX_LAYERED style!
	 */
	void         SetTransparency( BYTE alpha, COLORREF color = 0xFF000000 );

	virtual bool Create();
	virtual bool Create( DWORD dwStyles, HWND hParent = nullptr, RECT* rect = nullptr );
	virtual bool CreateEx( DWORD dwExStyles, DWORD dwStyles, HWND hParent = nullptr, RECT* rect = nullptr,
						   LPCWSTR classname = nullptr, HMENU hMenu = nullptr );

	// void MsgLoop();
	bool IsWindowClosed() const
	{
		return bWindowClosed;
	};

	operator HWND()
	{
		return m_hwnd;
	}

	operator HWND() const
	{
		return m_hwnd;
	}

protected:
	HINSTANCE    hResource;
	HWND         m_hwnd;
	HWND         m_hParent;
	bool         bWindowClosed;
	std::wstring sClassName;
	std::wstring sWindowTitle;
	std::wstring sRegistryPath;
	std::wstring sRegistryValue;
	bool         bWindowRestored;
	bool         bRegisterWindowCalled;
	WNDPROC      prevWndProc;

	// constructor
	CWindow( HINSTANCE hInst, CONST WNDCLASSEX* wcx = nullptr )
		: hResource( nullptr )
		, m_hwnd( nullptr )
		, m_hParent( nullptr )
		, bWindowClosed( FALSE )
		, bWindowRestored( false )
		, bRegisterWindowCalled( false )
		, prevWndProc( nullptr )
	{
		hResource = hInst;
		if ( wcx != nullptr )
			CWindow::RegisterWindow( wcx );
	};

	virtual ~CWindow() = default;

	// the real message handler
	virtual LRESULT CALLBACK WinMsgHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) = 0;

	// returns a pointer the window (stored as the WindowLong)
	static CWindow* GetObjectFromWindow( HWND hWnd )
	{
		return reinterpret_cast<CWindow*>( GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
	}
};
