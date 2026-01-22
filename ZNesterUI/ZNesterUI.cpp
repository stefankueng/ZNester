#include "pch.h"

#include "ZNesterUI.h"

#include "MainWindow.h"

#pragma warning( push )
#pragma warning( disable : 4458 )  // declaration of 'xxx' hides class member

namespace Gdiplus
{
	using std::max;
	using std::min;
};

#include <gdiplus.h>

#pragma warning( pop )

int APIENTRY wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
					   _In_ int /*nCmdShow*/ )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );

	CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED );

	INITCOMMONCONTROLSEX used = { sizeof( INITCOMMONCONTROLSEX ),
								  ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_WIN95_CLASSES };
	InitCommonControlsEx( &used );

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR                    gdiplusToken = 0;
	GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, nullptr );

	MSG msg;
	{
		CMainWindow mainWindow( hInstance );
		mainWindow.registerAndCreateWindow();

		HACCEL hAccelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDC_ZNESTERUI ) );

		// Main message loop:
		while ( GetMessage( &msg, nullptr, 0, 0 ) )
		{
			if ( !TranslateAccelerator( mainWindow, hAccelTable, &msg ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
	}
	Gdiplus::GdiplusShutdown( gdiplusToken );

	return static_cast<int>( msg.wParam );
}
