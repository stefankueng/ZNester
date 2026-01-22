#include "pch.h"

#include "BaseDialog.h"

#include <CommCtrl.h>
#include <WindowsX.h>

static HWND gHDlgCurrent = nullptr;

INT_PTR     CDialog::doModal( HINSTANCE hInstance, int resID, HWND hWndParent )
{
	m_bPseudoModal = false;
	m_hResource    = hInstance;
	return DialogBoxParam( hInstance, MAKEINTRESOURCE( resID ), hWndParent, &CDialog::stDlgFunc,
						   reinterpret_cast<LPARAM>( this ) );
}

INT_PTR CDialog::doModal( HINSTANCE hInstance, LPCDLGTEMPLATE pDlgTemplate, HWND hWndParent )
{
	m_bPseudoModal = false;
	m_hResource    = hInstance;
	return DialogBoxIndirectParam( hInstance, pDlgTemplate, hWndParent, &CDialog::stDlgFunc,
								   reinterpret_cast<LPARAM>( this ) );
}

INT_PTR CDialog::doModal( HINSTANCE hInstance, int resID, HWND hWndParent, UINT idAccel )
{
	m_bPseudoModal = true;
	m_bPseudoEnded = false;
	m_hResource    = hInstance;
	m_hwnd         = CreateDialogParam( hInstance, MAKEINTRESOURCE( resID ), hWndParent, &CDialog::stDlgFunc,
										reinterpret_cast<LPARAM>( this ) );

	// deactivate the parent window
	if ( hWndParent )
		::EnableWindow( hWndParent, FALSE );

	ShowWindow( m_hwnd, SW_SHOW );
	::BringWindowToTop( m_hwnd );
	::SetForegroundWindow( m_hwnd );

	// Main message loop:
	MSG    msg         = { nullptr };
	HACCEL hAccelTable = LoadAccelerators( m_hResource, MAKEINTRESOURCE( idAccel ) );
	BOOL   bRet        = TRUE;
	while ( !m_bPseudoEnded && ( ( bRet = GetMessage( &msg, nullptr, 0, 0 ) ) != 0 ) )
	{
		if ( bRet == -1 )
		{
			// handle the error and possibly exit
			break;
		}
		else
		{
			if ( !preTranslateMessage( &msg ) )
			{
				if ( !TranslateAccelerator( m_hwnd, hAccelTable, &msg ) && !::IsDialogMessage( m_hwnd, &msg ) )
				{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
			}
		}
	}
	if ( msg.message == WM_QUIT )
		PostQuitMessage( static_cast<int>( msg.wParam ) );
	// re-enable the parent window
	if ( hWndParent )
		::EnableWindow( hWndParent, TRUE );
	DestroyWindow( m_hwnd );
	if ( m_bPseudoModal )
		return m_iPseudoRet;
	return msg.wParam;
}

BOOL CDialog::endDialog( HWND hDlg, INT_PTR nResult )
{
	if ( m_bPseudoModal )
	{
		m_bPseudoEnded = true;
		m_iPseudoRet   = nResult;
	}
	return ::EndDialog( hDlg, nResult );
}

HWND CDialog::create( HINSTANCE hInstance, int resID, HWND hWndParent )
{
	m_bPseudoModal = true;
	m_hResource    = hInstance;
	m_hwnd         = CreateDialogParam( hInstance, MAKEINTRESOURCE( resID ), hWndParent, &CDialog::stDlgFunc,
										reinterpret_cast<LPARAM>( this ) );
	return m_hwnd;
}

void CDialog::showModeless( HINSTANCE hInstance, int resID, HWND hWndParent, bool show /* = true*/ )
{
	if ( m_hwnd == nullptr )
	{
		m_hResource = hInstance;
		m_hwnd      = CreateDialogParam( hInstance, MAKEINTRESOURCE( resID ), hWndParent, &CDialog::stDlgFunc,
										 reinterpret_cast<LPARAM>( this ) );
	}
	if ( show )
	{
		ShowWindow( m_hwnd, SW_SHOW );
		SetFocus( m_hwnd );
	}
}

void CDialog::showModeless( HINSTANCE hInstance, LPCDLGTEMPLATE pDlgTemplate, HWND hWndParent, bool show /* = true*/ )
{
	if ( m_hwnd == nullptr )
	{
		m_hResource = hInstance;
		m_hwnd      = CreateDialogIndirectParam( hInstance, pDlgTemplate, hWndParent, &CDialog::stDlgFunc,
												 reinterpret_cast<LPARAM>( this ) );
	}
	if ( show )
	{
		ShowWindow( m_hwnd, SW_SHOW );
		SetFocus( m_hwnd );
	}
}

void CDialog::initDialog( HWND hwndDlg, UINT iconID, bool bPosition /* = true*/ ) const
{
	RECT            rc, rcDlg, rcOwner;
	WINDOWPLACEMENT placement;
	placement.length = sizeof( WINDOWPLACEMENT );

	HWND hwndOwner   = ::GetParent( hwndDlg );
	GetWindowPlacement( hwndOwner, &placement );
	if ( ( hwndOwner == nullptr ) || ( placement.showCmd == SW_SHOWMINIMIZED ) ||
		 ( placement.showCmd == SW_SHOWMINNOACTIVE ) )
		hwndOwner = ::GetDesktopWindow();

	GetWindowRect( hwndOwner, &rcOwner );
	GetWindowRect( hwndDlg, &rcDlg );
	CopyRect( &rc, &rcOwner );

	OffsetRect( &rcDlg, -rcDlg.left, -rcDlg.top );
	OffsetRect( &rc, -rc.left, -rc.top );
	OffsetRect( &rc, -rcDlg.right, -rcDlg.bottom );

	if ( bPosition )
		SetWindowPos( hwndDlg, HWND_TOP, rcOwner.left + ( rc.right / 2 ), rcOwner.top + ( rc.bottom / 2 ), 0, 0,
					  SWP_NOSIZE | SWP_SHOWWINDOW );
	HICON hIcon = static_cast<HICON>(
		::LoadImage( m_hResource, MAKEINTRESOURCE( iconID ), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED ) );
	::SendMessage( hwndDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>( hIcon ) );
	::SendMessage( hwndDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>( hIcon ) );
}

void CDialog::addToolTip( UINT ctrlID, LPCWSTR text )
{
	TOOLINFO tt;
	tt.cbSize   = sizeof( TOOLINFO );
	tt.uFlags   = TTF_IDISHWND | TTF_SUBCLASS;
	tt.hwnd     = text == LPSTR_TEXTCALLBACK ? *this : GetDlgItem( *this, ctrlID );
	tt.uId      = reinterpret_cast<UINT_PTR>( GetDlgItem( *this, ctrlID ) );
	tt.lpszText = const_cast<LPWSTR>( text );

	SendMessage( m_hToolTips, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>( &tt ) );
}

void CDialog::addToolTip( HWND hWnd, LPCWSTR text ) const
{
	TOOLINFO tt;
	tt.cbSize   = sizeof( TOOLINFO );
	tt.uFlags   = TTF_IDISHWND | TTF_SUBCLASS;
	tt.hwnd     = hWnd;
	tt.uId      = reinterpret_cast<UINT_PTR>( hWnd );
	tt.lpszText = const_cast<LPWSTR>( text );

	SendMessage( m_hToolTips, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>( &tt ) );
}

INT_PTR CALLBACK CDialog::stDlgFunc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static bool bInDlgProc = false;
	if ( bInDlgProc )
		return FALSE;

	CDialog* pWnd;
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			// get the pointer to the window from lpCreateParams
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );
			pWnd         = reinterpret_cast<CDialog*>( lParam );
			pWnd->m_hwnd = hwndDlg;
			// create the tooltip control
			pWnd->m_hToolTips = CreateWindowEx( 0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
												CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndDlg,
												nullptr, pWnd->m_hResource, nullptr );

			SetWindowPos( pWnd->m_hToolTips, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
			SendMessage( pWnd->m_hToolTips, TTM_SETMAXTIPWIDTH, 0, 600 );
			SendMessage( pWnd->m_hToolTips, TTM_ACTIVATE, TRUE, 0 );
		}
		break;
	}
	// get the pointer to the window
	pWnd = GetObjectFromWindow( hwndDlg );

	// if we have the pointer, go to the message handler of the window
	if ( pWnd )
	{
		LRESULT lRes = pWnd->dlgFunc( hwndDlg, uMsg, wParam, lParam );
		switch ( uMsg )
		{
			case WM_ACTIVATE:
				if ( 0 == wParam )  // becoming inactive
					gHDlgCurrent = nullptr;
				else                // becoming active
					gHDlgCurrent = hwndDlg;
				break;
			default:
				break;
		}
		SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, lRes );
		return lRes;
	}
	else
		return 0;
}

bool CDialog::preTranslateMessage( MSG* /*pMsg*/ )
{
	return false;
}

bool CDialog::isCursorOverWindowBorder()
{
	RECT wrc, crc;
	GetWindowRect( *this, &wrc );
	GetClientRect( *this, &crc );
	MapWindowPoints( *this, nullptr, reinterpret_cast<LPPOINT>( &crc ), 2 );
	DWORD pos = GetMessagePos();
	POINT pt;
	pt.x = GET_X_LPARAM( pos );
	pt.y = GET_Y_LPARAM( pos );
	return ( PtInRect( &wrc, pt ) && !PtInRect( &crc, pt ) );
}

/**
 * Wrapper around the CWnd::EnableWindow() method, but
 * makes sure that a control that has the focus is not disabled
 * before the focus is passed on to the next control.
 */
bool CDialog::dialogEnableWindow( UINT nID, bool bEnable )
{
	HWND hwndDlgItem = GetDlgItem( *this, nID );
	if ( hwndDlgItem == nullptr )
		return false;
	if ( bEnable )
		return !!EnableWindow( hwndDlgItem, bEnable );
	if ( GetFocus() == hwndDlgItem )
	{
		SendMessage( *this, WM_NEXTDLGCTL, 0, false );
	}
	return !!EnableWindow( hwndDlgItem, bEnable );
}

int CDialog::getDlgItemTextLength( UINT nId )
{
	HWND hWnd = GetDlgItem( *this, nId );
	return GetWindowTextLength( hWnd );
}

std::unique_ptr<wchar_t[]> CDialog::GetDlgItemText( UINT nId )
{
	int len = getDlgItemTextLength( nId );
	len++;
	auto buf = std::make_unique<wchar_t[]>( len );
	::GetDlgItemText( *this, nId, buf.get(), len );
	return buf;
}

void CDialog::RefreshCursor()
{
	POINT pt;
	GetCursorPos( &pt );
	SetCursorPos( pt.x, pt.y );
}

void CDialog::showEditBalloon( UINT nId, LPCWSTR title, LPCWSTR text, int icon /*= TTI_ERROR*/ )
{
	EDITBALLOONTIP ebt = { 0 };
	ebt.cbStruct       = sizeof( EDITBALLOONTIP );
	ebt.pszTitle       = title;
	ebt.pszText        = text;
	ebt.ttiIcon        = icon;
	if ( !::SendMessage( GetDlgItem( *this, nId ), EM_SHOWBALLOONTIP, 0, reinterpret_cast<LPARAM>( &ebt ) ) )
	{
		UINT uType = MB_ICONERROR;
		switch ( icon )
		{
			case TTI_ERROR:
			case TTI_ERROR_LARGE:
				uType = MB_ICONERROR;
				break;
			case TTI_WARNING:
			case TTI_WARNING_LARGE:
				uType = MB_ICONWARNING;
				break;
			case TTI_INFO:
			case TTI_INFO_LARGE:
				uType = MB_ICONINFORMATION;
				break;
			case TTI_NONE:
				uType = 0;
				break;
		}
		::MessageBox( *this, text, title, uType );
	}
}

void CDialog::setTransparency( BYTE alpha, COLORREF color /*= 0xFF000000*/ )
{
	if ( alpha == 255 )
	{
		LONG_PTR exStyle  = GetWindowLongPtr( *this, GWL_EXSTYLE );
		exStyle          &= ~WS_EX_LAYERED;
		SetWindowLongPtr( *this, GWL_EXSTYLE, exStyle );
	}
	else
	{
		LONG_PTR exStyle  = GetWindowLongPtr( *this, GWL_EXSTYLE );
		exStyle          |= WS_EX_LAYERED;
		SetWindowLongPtr( *this, GWL_EXSTYLE, exStyle );
	}
	COLORREF col   = color;
	DWORD    flags = LWA_ALPHA;
	if ( col & 0xFF000000 )
	{
		col   = RGB( 255, 255, 255 );
		flags = LWA_ALPHA;
	}
	else
	{
		flags = LWA_COLORKEY;
	}
	SetLayeredWindowAttributes( *this, col, alpha, flags );
}

BOOL CDialog::IsDialogMessage( LPMSG lpMsg )
{
	if ( gHDlgCurrent )
	{
		return ::IsDialogMessage( gHDlgCurrent, lpMsg );
	}
	return FALSE;
}

RECT CDialog::adjustControlSize( UINT nID )
{
	HWND hwndDlgItem = GetDlgItem( *this, nID );
	// adjust the size of the control to fit its content
	auto sControlText = GetDlgItemText( nID );
	// next step: find the rectangle the control text needs to
	// be displayed

	HDC  hDC = GetWindowDC( *this );
	RECT controlRect;
	GetWindowRect( hwndDlgItem, &controlRect );
	::MapWindowPoints( nullptr, *this, reinterpret_cast<LPPOINT>( &controlRect ), 2 );
	RECT controlRectOrig = controlRect;
	if ( hDC )
	{
		HFONT hFont    = GetWindowFont( hwndDlgItem );
		HFONT hOldFont = static_cast<HFONT>( SelectObject( hDC, hFont ) );
		if ( DrawText( hDC, sControlText.get(), -1, &controlRect,
					   DT_WORDBREAK | DT_EDITCONTROL | DT_EXPANDTABS | DT_LEFT | DT_CALCRECT ) )
		{
			// now we have the rectangle the control really needs
			if ( ( controlRectOrig.right - controlRectOrig.left ) > ( controlRect.right - controlRect.left ) )
			{
				// we're dealing with radio buttons and check boxes,
				// which means we have to add a little space for the checkbox
				const int checkWidth  = GetSystemMetrics( SM_CXMENUCHECK ) + 2 * GetSystemMetrics( SM_CXEDGE ) + 3;
				controlRectOrig.right = controlRectOrig.left + ( controlRect.right - controlRect.left ) + checkWidth;
				MoveWindow( hwndDlgItem, controlRectOrig.left, controlRectOrig.top,
							controlRectOrig.right - controlRectOrig.left, controlRectOrig.bottom - controlRectOrig.top,
							TRUE );
			}
		}
		SelectObject( hDC, hOldFont );
		ReleaseDC( *this, hDC );
	}
	return controlRectOrig;
}
