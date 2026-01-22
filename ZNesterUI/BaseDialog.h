#pragma once

#include <memory>

/**
 * A base window class.
 * Provides separate window message handlers for every window object based on
 * this class.
 */
class CDialog
{
public:
	CDialog()
		: m_hResource( nullptr )
		, m_hwnd( nullptr )
		, m_bPseudoModal( false )
		, m_bPseudoEnded( false )
		, m_iPseudoRet( 0 )
		, m_hToolTips( nullptr )
	{
	}

	virtual ~CDialog() = default;

	INT_PTR     doModal( HINSTANCE hInstance, int resID, HWND hWndParent );
	INT_PTR     doModal( HINSTANCE hInstance, LPCDLGTEMPLATE pDlgTemplate, HWND hWndParent );
	INT_PTR     doModal( HINSTANCE hInstance, int resID, HWND hWndParent, UINT idAccel );
	void        showModeless( HINSTANCE hInstance, int resID, HWND hWndParent, bool show = true );
	void        showModeless( HINSTANCE hInstance, LPCDLGTEMPLATE pDlgTemplate, HWND hWndParent, bool show = true );
	static BOOL IsDialogMessage( LPMSG lpMsg );
	HWND        create( HINSTANCE hInstance, int resID, HWND hWndParent );
	BOOL        endDialog( HWND hDlg, INT_PTR nResult );
	void        addToolTip( UINT ctrlID, LPCWSTR text );
	void        addToolTip( HWND hWnd, LPCWSTR text ) const;
	bool        isCursorOverWindowBorder();
	static void RefreshCursor();
	void        showEditBalloon( UINT nId, LPCWSTR title, LPCWSTR text, int icon = TTI_ERROR );
	/**
	 * Sets the transparency of the window.
	 * \remark note that this also sets the WS_EX_LAYERED style!
	 */
	void setTransparency( BYTE alpha, COLORREF color = 0xFF000000 );

	/**
	 * Wrapper around the CWnd::EnableWindow() method, but
	 * makes sure that a control that has the focus is not disabled
	 * before the focus is passed on to the next control.
	 */
	bool                       dialogEnableWindow( UINT nID, bool bEnable );
	int                        getDlgItemTextLength( UINT nId );
	std::unique_ptr<wchar_t[]> GetDlgItemText( UINT nId );

	virtual LRESULT CALLBACK   dlgFunc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam ) = 0;
	virtual bool               preTranslateMessage( MSG* pMsg );

	operator HWND()
	{
		return m_hwnd;
	}

	operator HWND() const
	{
		return m_hwnd;
	}

	HWND getToolTipHwnd() const
	{
		return m_hToolTips;
	}

protected:
	HINSTANCE m_hResource;
	HWND      m_hwnd;

	void      initDialog( HWND hwndDlg, UINT iconID, bool bPosition = true ) const;
	/**
	 * Adjusts the size of a checkbox or radio button control.
	 * Since we always make the size of those bigger than 'necessary'
	 * for making sure that translated strings can fit in those too,
	 * this method can reduce the size of those controls again to only
	 * fit the text.
	 */
	RECT adjustControlSize( UINT nID );

	// the real message handler
	static INT_PTR CALLBACK stDlgFunc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

	// returns a pointer the dialog (stored as the WindowLong)
	static CDialog* GetObjectFromWindow( HWND hWnd )
	{
		return reinterpret_cast<CDialog*>( GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
	}

private:
	bool    m_bPseudoModal;
	bool    m_bPseudoEnded;
	INT_PTR m_iPseudoRet;
	HWND    m_hToolTips;
};
