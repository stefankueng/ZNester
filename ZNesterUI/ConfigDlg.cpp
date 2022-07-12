#include "pch.h"

#include "ConfigDlg.h"

#include <Shlwapi.h>
#include <shobjidl_core.h>
#include <wrl/client.h>

#include <cassert>

#include "ResString.h"
#include "resource.h"
#include "stringtok.h"

ConfigDlg::ConfigDlg( HWND hParent )
	: m_width( 1500 )
	, m_height( 1000 )
	, m_copies( 1 )
	, m_partDistance( 1 )
	, m_binDistance( 4 )
	, m_rotations( 4 )
	, m_useHoles( true )
	, m_hParent( hParent )
{
}

LRESULT ConfigDlg::dlgFunc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	UNREFERENCED_PARAMETER( lParam );
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			initDialog( hwndDlg, IDI_ZNESTERUI );
			CheckRadioButton( *this, IDC_RECTBINRADIO, IDC_IRREGULARBINRADIO, IDC_RECTBINRADIO );
			SetDlgItemText( *this, IDC_WIDTH, L"1500" );
			SetDlgItemText( *this, IDC_HEIGHT, L"1000" );
			SetDlgItemText( *this, IDC_COPIES, L"1" );
			SetDlgItemText( *this, IDC_PARTDIST, L"2" );
			SetDlgItemText( *this, IDC_BINDIST, L"4" );
			SetDlgItemText( *this, IDC_ROTATIONS, L"4" );
			CheckDlgButton( *this, IDC_FILLHOLESCHECK, BST_CHECKED );
			enableControls();
		}
			return TRUE;
		case WM_COMMAND:
			return doCommand( LOWORD( wParam ) );
		default:
			return FALSE;
	}
}

LRESULT ConfigDlg::doCommand( int id )
{
	switch ( id )
	{
		case IDOK:
		{
			wchar_t buf[1024]{};
			m_width	 = 0;
			m_height = 0;
			m_binPath.clear();
			m_objPath.clear();
			m_copies	   = 0;
			m_partDistance = 0;
			m_binDistance  = 0;
			if ( IsDlgButtonChecked( *this, IDC_RECTBINRADIO ) )
			{
				GetWindowText( GetDlgItem( *this, IDC_WIDTH ), buf, _countof( buf ) );
				m_width = _wtoi( buf );
				GetWindowText( GetDlgItem( *this, IDC_HEIGHT ), buf, _countof( buf ) );
				m_height = _wtoi( buf );
			}
			else
			{
				m_binPath = GetDlgItemText( IDC_BINPATH ).get();
			}
			m_objPath = GetDlgItemText( IDC_OBJPATH ).get();

			GetWindowText( GetDlgItem( *this, IDC_COPIES ), buf, _countof( buf ) );
			m_copies = _wtoi( buf );
			GetWindowText( GetDlgItem( *this, IDC_PARTDIST ), buf, _countof( buf ) );
			m_partDistance = _wtoi( buf );
			GetWindowText( GetDlgItem( *this, IDC_BINDIST ), buf, _countof( buf ) );
			m_binDistance = _wtoi( buf );
			GetWindowText( GetDlgItem( *this, IDC_ROTATIONS ), buf, _countof( buf ) );
			m_rotations = _wtoi( buf );
			m_useHoles	= !!IsDlgButtonChecked( *this, IDC_FILLHOLESCHECK );

			if ( !PathFileExists( m_objPath.c_str() ) || PathIsDirectory( m_objPath.c_str() ) )
			{
				showEditBalloon( IDC_OBJPATH, L"Invalid path", L"please enter a valid path" );
				return 0;
			}
			if ( !m_binPath.empty() &&
				 ( !PathFileExists( m_binPath.c_str() ) || PathIsDirectory( m_binPath.c_str() ) ) )
			{
				showEditBalloon( IDC_BINPATH, L"Invalid path", L"please enter a valid path" );
				return 0;
			}
			if ( m_copies == 0 )
			{
				showEditBalloon( IDC_COPIES, L"wrong number of copies", L"value must be greater than zero" );
				return 0;
			}
		}
			[[fallthrough]];
		case IDCANCEL:
			endDialog( *this, id );
			break;
		case IDC_RECTBINRADIO:
		case IDC_IRREGULARBINRADIO:
			enableControls();
			break;
		case IDC_BINPATHBROWSE:
		{
			auto paths = askForFiles( { L"SVG files|*.svg" } );
			if ( !paths.empty() )
			{
				SetDlgItemText( *this, IDC_BINPATH, paths[0].c_str() );
			}
		}
		break;
		case IDC_OBJPATHBROWSE:
		{
			auto paths = askForFiles( { L"SVG files|*.svg" } );
			if ( !paths.empty() )
			{
				SetDlgItemText( *this, IDC_OBJPATH, paths[0].c_str() );
			}
		}
		break;
	}
	return 1;
}

std::vector<std::wstring> ConfigDlg::askForFiles( const std::vector<std::wstring>& filters )
{
	Microsoft::WRL::ComPtr<IFileOpenDialog> pfd;
	HRESULT									hr =
		CoCreateInstance( CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( pfd.GetAddressOf() ) );
	if ( FAILED( hr ) )
		return {};

	// Set the dialog options, if it fails don't continue as we don't
	// know what options we're blending. Shouldn't ever fail anyway.
	DWORD dwOptions = 0;
	hr				= pfd->GetOptions( &dwOptions );

	// allow the user to enter a filename that does not exist yet
	dwOptions &= ~( FOS_FILEMUSTEXIST );

	// If we can't set our options, we have no idea what's happening
	// so don't continue.
	hr = pfd->SetOptions( dwOptions | FOS_ALLOWMULTISELECT );
	if ( FAILED( hr ) )
		return {};
	// Set the standard title.
	ResString rTitle( m_hResource, IDS_APP_TITLE );
	pfd->SetTitle( rTitle );

	if ( !filters.empty() )
	{
		auto					  filterSpec	  = std::make_unique<COMDLG_FILTERSPEC[]>( filters.size() );
		int						  filterSpecIndex = 0;
		std::vector<std::wstring> filterStore;
		for ( const auto& filter : filters )
		{
			std::vector<std::wstring> vec;
			stringtok( vec, filter, L"|" );
			assert( vec.size() == 2 );
			filterStore.push_back( vec[0] );
			filterSpec[filterSpecIndex].pszName = filterStore.back().c_str();
			filterStore.push_back( vec[1] );
			filterSpec[filterSpecIndex].pszSpec = filterStore.back().c_str();
			++filterSpecIndex;
		}
		pfd->SetFileTypes( static_cast<UINT>( filters.size() ), filterSpec.get() );
		pfd->SetFileTypeIndex( 1 );
	}

	// Show the open file dialog, not much we can do if that doesn't work.
	hr = pfd->Show( *this );
	if ( hr == HRESULT_FROM_WIN32( ERROR_CANCELLED ) )	// Expected error
		return {};
	if ( FAILED( hr ) )
		return {};

	Microsoft::WRL::ComPtr<IShellItemArray> psiaResults;
	hr = pfd->GetResults( psiaResults.GetAddressOf() );
	if ( FAILED( hr ) )
		return {};

	DWORD count = 0;
	hr			= psiaResults->GetCount( &count );
	std::vector<std::wstring> paths;
	paths.reserve( count );
	for ( decltype( count ) i = 0; i < count; ++i )
	{
		Microsoft::WRL::ComPtr<IShellItem> psiResult;
		hr = psiaResults->GetItemAt( i, psiResult.GetAddressOf() );
		if ( SUCCEEDED( hr ) )
		{
			PWSTR pszPath = nullptr;
			hr			  = psiResult->GetDisplayName( SIGDN_FILESYSPATH, &pszPath );
			if ( SUCCEEDED( hr ) )
			{
				paths.push_back( pszPath );
				CoTaskMemFree( pszPath );
			}
		}
	}

	return paths;
}

void ConfigDlg::enableControls()
{
	auto rectBin = IsDlgButtonChecked( *this, IDC_RECTBINRADIO );
	dialogEnableWindow( IDC_WIDTH, rectBin );
	dialogEnableWindow( IDC_HEIGHT, rectBin );
	dialogEnableWindow( IDC_BINPATH, !rectBin );
	dialogEnableWindow( IDC_BINPATHBROWSE, !rectBin );
}
