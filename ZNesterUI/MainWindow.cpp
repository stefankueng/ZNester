#include "pch.h"

#include "MainWindow.h"

#include <shobjidl_core.h>
#include <wrl.h>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "../ZNester/ZNester.h"
#include "ConfigDlg.h"
#include "ResString.h"
#include "resource.h"

using Microsoft::WRL::ComPtr;

INT_PTR CALLBACK about( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

CMainWindow::CMainWindow( HINSTANCE hInst, const WNDCLASSEX* wcx )
	: CWindow( hInst, wcx )
{
}

bool CMainWindow::registerAndCreateWindow()
{
	WNDCLASSEX wcx;

	// Fill in the window class structure with default parameters
	wcx.cbSize      = sizeof( WNDCLASSEX );
	wcx.style       = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = CWindow::stWinMsgHandler;
	wcx.cbClsExtra  = 0;
	wcx.cbWndExtra  = 0;
	wcx.hInstance   = hResource;
	ResString clsName( hResource, IDS_APP_TITLE );
	wcx.lpszClassName = clsName;
	wcx.hIconSm       = LoadIcon( wcx.hInstance, MAKEINTRESOURCE( IDI_ZNESTERUI ) );
	wcx.hIcon         = LoadIcon( wcx.hInstance, MAKEINTRESOURCE( IDI_ZNESTERUI ) );
	wcx.hCursor       = LoadCursor( nullptr, IDC_ARROW );
	wcx.hbrBackground = reinterpret_cast<HBRUSH>( ( COLOR_WINDOW + 1 ) );
	wcx.lpszMenuName  = MAKEINTRESOURCEW( IDC_ZNESTERUI );

	if ( RegisterWindow( &wcx ) )
	{
		if ( Create( WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE, nullptr ) )
		{
			UpdateWindow( m_hwnd );
			return true;
		}
	}
	return false;
}

LRESULT CALLBACK CMainWindow::WinMsgHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_CREATE:
		{
			m_hwnd = hwnd;
		}
		break;
		case WM_COMMAND:
		{
			return doCommand( LOWORD( wParam ), lParam );
		}
		case WM_ERASEBKGND:
			return TRUE;
		case WM_PAINT:
		{
			PAINTSTRUCT ps{};
			RECT        rect{};

			::GetClientRect( *this, &rect );
			HDC hdc = BeginPaint( hwnd, &ps );
			{
				std::scoped_lock lock( m_polygonMutex );

				Gdiplus::Bitmap  bmp( rect.right, rect.bottom );
				auto             memG = std::unique_ptr<Gdiplus::Graphics>( Gdiplus::Graphics::FromImage( &bmp ) );
				memG->Clear( Gdiplus::Color::White );
				auto                     binPen = Gdiplus::Pen( Gdiplus::Color( 0, 0, 0 ) );

				std::vector<SVGPolygon>* polys  = &m_polygons;
				if ( !m_debugPolys.empty() )
					polys = &m_debugPolys;
				if ( !m_polygonsPlaced.empty() )
					polys = &m_polygonsPlaced;
				float maxX = 0.0;
				float minX = FLT_MAX;
				float maxY = 0.0;
				float minY = FLT_MAX;
				for ( const auto& bin : m_binsPlaced )
				{
					for ( const auto& pt : bin.points )
					{
						maxX = std::max( pt.x, maxX );
						minX = std::min( pt.x, minX );
						maxY = std::max( pt.y, maxY );
						minY = std::min( pt.y, minY );
					}
				}
				for ( const auto& polygon : *polys )
				{
					for ( const auto& pt : polygon.points )
					{
						maxX = std::max( pt.x, maxX );
						minX = std::min( pt.x, minX );
						maxY = std::max( pt.y, maxY );
						minY = std::min( pt.y, minY );
					}
				}

				auto            scaleX = ( rect.right - 0.0f ) / ( maxX - minX );
				auto            scaleY = ( rect.bottom - 0.0f ) / ( maxY - minY );
				auto            scale  = std::min( scaleX, scaleY );
				Gdiplus::Matrix transformMatrix;
				transformMatrix.Translate( static_cast<float>( m_scalePt.x ), static_cast<float>( m_scalePt.y ) );
				transformMatrix.Scale( m_scale, m_scale );
				memG->SetTransform( &transformMatrix );

				memG->ScaleTransform( scale, scale );
				memG->TranslateTransform( ( 0.0f - minX ), ( 0.0f - minY ) );

				for ( const auto& bin : m_binsPlaced )
				{
					auto   binPoints = std::make_unique<Gdiplus::PointF[]>( bin.points.size() );
					size_t i         = 0;
					for ( const auto& pt : bin.points )
					{
						binPoints[i].X = pt.x;
						binPoints[i].Y = pt.y;
						++i;
					}
					memG->DrawPolygon( &binPen, binPoints.get(), static_cast<int>( m_bin.points.size() ) );
				}

				auto        pen = std::make_unique<Gdiplus::Pen>( Gdiplus::Color( 255, 0, 0 ), 1.0f / m_scale / scale );
				const float ptRad   = 1.5f / m_scale / scale;

				int         polyNum = 0;
				for ( const auto& polygon : *polys )
				{
					++polyNum;
					auto points = std::make_unique<Gdiplus::PointF[]>( polygon.points.size() );
					int  j      = 0;
					for ( const auto& pt : polygon.points )
					{
						points[j].X = pt.x;
						points[j].Y = pt.y;
						++j;
						maxX = std::max( pt.x, maxX );
						maxY = std::max( pt.y, maxY );
						if ( m_bShowPolygonPoints )
							memG->DrawEllipse( pen.get(), pt.x - ptRad, pt.y - ptRad, 2.0f * ptRad, 2.0f * ptRad );
					}

					memG->DrawPolygon( pen.get(), points.get(), static_cast<int>( polygon.points.size() ) );
					if ( !m_debugPolys.empty() )
					{
						switch ( polyNum % 3 )
						{
							case 0:
								pen = std::make_unique<Gdiplus::Pen>( Gdiplus::Color( 128, 255, 0, 0 ),
																	  1.0f / m_scale / scale );
								break;
							case 1:
								pen = std::make_unique<Gdiplus::Pen>( Gdiplus::Color( 128, 0, 100, 0 ),
																	  1.0f / m_scale / scale );
								break;
							case 2:
								pen = std::make_unique<Gdiplus::Pen>( Gdiplus::Color( 128, 0, 0, 150 ),
																	  1.0f / m_scale / scale );
								break;
							case 3:
								pen = std::make_unique<Gdiplus::Pen>( Gdiplus::Color( 128, 0, 100, 100 ),
																	  1.0f / m_scale / scale );
								break;
							case 4:
								pen = std::make_unique<Gdiplus::Pen>( Gdiplus::Color( 128, 100, 0, 100 ),
																	  1.0f / m_scale / scale );
								break;
							case 5:
								pen = std::make_unique<Gdiplus::Pen>( Gdiplus::Color( 128, 100, 100, 0 ),
																	  1.0f / m_scale / scale );
								break;
						}
					}
				}
				if ( !m_debugMarkers.empty() )
				{
					auto dbgMarkerPen =
						std::make_unique<Gdiplus::Pen>( Gdiplus::Color( 255, 0, 0 ), 1.0f / m_scale / scale );
					const float dbgMarkerRad = 2.0f / m_scale / scale;
					for ( const auto& pt : m_debugMarkers )
					{
						memG->DrawEllipse( dbgMarkerPen.get(), pt.x - dbgMarkerRad, pt.y - dbgMarkerRad,
										   2.0f * dbgMarkerRad, 2.0f * dbgMarkerRad );
					}
				}

				if ( !m_timeString.empty() )
				{
					memG->ResetTransform();
					Gdiplus::FontFamily fontFamily( L"Arial" );
					Gdiplus::Font       font( &fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPoint );
					Gdiplus::SolidBrush solidBrush( Gdiplus::Color( 0, 80, 0 ) );
					memG->DrawString( m_timeString.c_str(), -1, &font,
									  { 20.0f, static_cast<float>( rect.top + 20.0f ) }, nullptr, &solidBrush );
				}
				if ( !m_totalTimeString.empty() )
				{
					memG->ResetTransform();
					Gdiplus::FontFamily fontFamily( L"Arial" );
					Gdiplus::Font       font( &fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPoint );
					Gdiplus::SolidBrush solidBrush( Gdiplus::Color( 0, 80, 0 ) );
					memG->DrawString( m_totalTimeString.c_str(), -1, &font,
									  { rect.right - 500.0f, static_cast<float>( rect.top + 10.0f ) }, nullptr,
									  &solidBrush );
				}
				if ( !m_infoString.empty() )
				{
					memG->ResetTransform();
					Gdiplus::FontFamily fontFamily( L"Arial" );
					Gdiplus::Font       font( &fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPoint );
					Gdiplus::SolidBrush solidBrush( Gdiplus::Color( 0, 80, 0 ) );
					memG->DrawString( m_infoString.c_str(), -1, &font,
									  { rect.right - 500.0f, static_cast<float>( rect.top + 50.0f ) }, nullptr,
									  &solidBrush );
				}
				Gdiplus::Graphics g( hdc );
				g.DrawImage( &bmp, 0.0f, 0.0f );
			}
			EndPaint( hwnd, &ps );
		}
		break;
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi       = reinterpret_cast<MINMAXINFO*>( lParam );
			mmi->ptMinTrackSize.x = 640;
			mmi->ptMinTrackSize.y = 400;
			return 0;
		}
		case WM_SIZE:
		{
		}
		break;
		case WM_RBUTTONDOWN:
		{
			clearZoom();
			InvalidateRect( *this, nullptr, FALSE );
		}
		break;
		case WM_LBUTTONDOWN:
		{
			m_downStartPt.x = GET_X_LPARAM( lParam );
			m_downStartPt.y = GET_Y_LPARAM( lParam );
		}
		break;
		case WM_LBUTTONUP:
		{
			m_downStartPt = {};
		}
		break;
		case WM_MOUSEMOVE:
		{
			if ( m_downStartPt.x != 0 && m_downStartPt.y != 0 && wParam & MK_LBUTTON )
			{
				POINT pt{ GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) };
				auto  deltaX   = pt.x - m_downStartPt.x;
				auto  deltaY   = pt.y - m_downStartPt.y;
				m_scalePt.x   += deltaX;
				m_scalePt.y   += deltaY;
				m_downStartPt  = pt;
				InvalidateRect( *this, nullptr, FALSE );
			}
		}
		break;
		case WM_MOUSEWHEEL:
		{
			auto  zDelta = GET_WHEEL_DELTA_WPARAM( wParam );
			POINT pt{ GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) };
			ScreenToClient( *this, &pt );

			auto oldZoomFocus = Gdiplus::PointF( ( pt.x - m_scalePt.x ) / m_scale, ( pt.y - m_scalePt.y ) / m_scale );

			// zoom
			m_scale += ( zDelta / 600.0f );
			m_scale  = std::max( 1.0f, m_scale );

			// determine where the focus has moved as part of the zoom
			auto newZoomFocusX = oldZoomFocus.X * m_scale + m_scalePt.x;
			auto newZoomFocusY = oldZoomFocus.Y * m_scale + m_scalePt.y;

			// adjust offset based on displacement and update windows stuff
			m_scalePt.x -= static_cast<long>( newZoomFocusX - pt.x );
			m_scalePt.y -= static_cast<long>( newZoomFocusY - pt.y );

			InvalidateRect( *this, nullptr, FALSE );
		}
		break;
		case WM_DESTROY:
			bWindowClosed = TRUE;
			PostQuitMessage( 0 );
			break;
		case WM_CLOSE:
			if ( m_nester.isNesting() )
			{
				m_nester.stopNest();
				m_timeString.clear();
				InvalidateRect( m_hwnd, nullptr, false );
				return 0;
			}
			::DestroyWindow( m_hwnd );
			break;
		default:
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}

	return 0;
}

LRESULT CMainWindow::doCommand( int id, LPARAM /*lParam*/ )
{
	switch ( id )
	{
		case IDM_ABOUT:
			DialogBox( hResource, MAKEINTRESOURCE( IDD_ABOUTBOX ), *this, about );
			break;
		case ID_VIEW_SHOWPOLYGONPOINTS:
		{
			m_bShowPolygonPoints = !m_bShowPolygonPoints;
			auto hMenuMain       = GetMenu( *this );
			CheckMenuItem( hMenuMain, ID_VIEW_SHOWPOLYGONPOINTS,
						   MF_BYCOMMAND | ( m_bShowPolygonPoints ? MF_CHECKED : 0 ) );
			InvalidateRect( *this, nullptr, TRUE );
		}
		break;
		case ID_VIEW_RESETZOOM:
			clearZoom();
			InvalidateRect( *this, nullptr, TRUE );
			break;
		case ID_NEST_STARTNESTING:
		{
			ConfigDlg dlg( *this );
			if ( dlg.doModal( hResource, IDD_CONFIGDLG, *this ) != IDOK )
				return false;

			m_bin.points.clear();
			m_debugPolys.clear();
			m_binsPlaced.clear();
			m_timeString.clear();
			m_totalTimeString.clear();

			ZPolygon bin;
			if ( dlg.m_binPath.empty() )
			{
				bin.emplace_back( 0, 0 );
				bin.emplace_back( static_cast<double>( dlg.m_width ), 0 );
				bin.emplace_back( static_cast<double>( dlg.m_width ), static_cast<double>( dlg.m_height ) );
				bin.emplace_back( 0, static_cast<double>( dlg.m_height ) );

				for ( const auto& pt : bin )
					m_bin.points.emplace_back( static_cast<float>( pt.x() ), static_cast<float>( pt.y() ) );
				m_binsPlaced.push_back( m_bin );
			}
			else
			{
				m_svg.parseFile( dlg.m_binPath );
				auto polys = m_svg.getPolygons();
				if ( polys.empty() )
					return false;
				if ( polys.size() > 1 )
					OutputDebugString( L"multiple bins found!!!\n" );
				m_bin = polys[0];
				for ( const auto& pt : m_bin.points )
					bin.emplace_back( static_cast<double>( pt.x ), static_cast<double>( pt.y ) );
				auto bounds = bin.bounds();
				for ( auto& pt : m_bin.points )
				{
					pt.x -= static_cast<float>( bounds.x() );
					pt.y -= static_cast<float>( bounds.y() );
				}
				m_binsPlaced.push_back( m_bin );
			}
			m_svg.parseFile( dlg.m_objPath );
			m_polygons = m_svg.getPolygons();

			std::deque<ZPolygon> polygons;
			size_t               pid = 1;
			for ( const auto& [poly] : m_polygons )
			{
				ZPolygon p;
				for ( const auto& [ptx, pty] : poly )
				{
					p.emplace_back( ptx, pty );
				}
				p.setId( pid++ );
				p.setRotations( dlg.m_rotations );
				p.setCopies( dlg.m_copies );
				polygons.push_back( p );
			}

			ZNesterConfig config;
			config.binDistance  = static_cast<double>( dlg.m_binDistance );
			config.partDistance = static_cast<double>( dlg.m_partDistance );
			config.useHoles     = dlg.m_useHoles;

			m_iterationBegin    = std::chrono::steady_clock::now();
			m_begin             = m_iterationBegin;
			m_nester.setLogCallback(
				[&]( eZLogLevel level, const std::string& msg )
				{
					auto text = std::format( "{0} : {1}\n", static_cast<int>( level ), msg.c_str() );
					OutputDebugStringA( text.c_str() );
				} );
			m_nester.setDebugDisplayCallback(
				[&]( const std::deque<ZPolygon>& polys, const std::deque<ZPoint>& markers )
				{
					{
						std::scoped_lock lock( m_polygonMutex );
						m_polygonsPlaced.clear();
						m_debugPolys.clear();
						m_debugMarkers.clear();
						for ( const auto& poly : polys )
						{
							SVGPolygon svgPoly;
							for ( const auto& pt : poly )
							{
								svgPoly.points.emplace_back( static_cast<float>( pt.x() ),
															 static_cast<float>( pt.y() ) );
							}
							m_debugPolys.push_back( svgPoly );
						}
						for ( const auto& pt : markers )
						{
							SVGPoint svgPoint;
							svgPoint.x = static_cast<float>( pt.x() );
							svgPoint.y = static_cast<float>( pt.y() );
							m_debugMarkers.push_back( svgPoint );
						}
					}
					InvalidateRect( m_hwnd, nullptr, false );

					MSG msg = {};
					while ( GetMessage( &msg, nullptr, 0, 0 ) )
					{
						if ( msg.message == WM_LBUTTONDOWN )
							break;
						if ( msg.message == WM_KEYUP )
							break;
						if ( msg.message == WM_QUIT )
							break;
						TranslateMessage( &msg );
						DispatchMessage( &msg );
					}
				} );
			m_nester.setCallback(
				[&]( const std::deque<ZPlacement>& positions, double binArea )
				{
					std::scoped_lock lock( m_polygonMutex );
					m_debugPolys.clear();
					m_debugMarkers.clear();
					if ( !positions.empty() )
					{
						m_polygonsPlaced.clear();
						m_binsPlaced.clear();
						m_infoString.clear();
						float minX = FLT_MAX;
						float maxX = 0.0;
						float minY = FLT_MAX;
						for ( const auto& pt : m_bin.points )
						{
							minX = std::min( pt.x, minX );
							minY = std::min( pt.y, minY );
							maxX = std::max( pt.x, maxX );
						}

						// arrange the bins from left to right, aligned at the top
						float              binX = 0.0f;
						float              binY = 0.0f;
						std::vector<float> binOffsets;
						SVGPolygon         binPoly;
						float              rightX = binX;
						for ( const auto& pt : m_bin.points )
						{
							binPoly.points.emplace_back( pt.x - minX + binX, pt.y - minY + binY );
							rightX = std::max( rightX, pt.x - minX + binX );
						}
						m_binsPlaced.push_back( binPoly );
						binOffsets.push_back( binX );
						binX = rightX;

						// now go through all placed parts
						size_t       binIndex = 0;
						std::wstring infoString;
						for ( const auto& binPositions : positions )
						{
							if ( binIndex == m_binsPlaced.size() )
							{
								binPoly.points.clear();
								for ( const auto& pt : m_bin.points )
								{
									binPoly.points.emplace_back( ( binIndex * rightX ) + pt.x - minX,
																 pt.y - minY + binY );
								}
								m_binsPlaced.push_back( binPoly );
							}
							size_t polyId = 1;

							for ( const auto& [poly] : m_polygons )
							{
								auto startIt = binPositions.begin();

								auto found   = std::find_if( startIt, binPositions.end(),
															 [&]( const auto& p ) { return p.id == polyId; } );
								while ( found != binPositions.end() )
								{
									SVGPolygon polyPlaced;
									for ( const auto& pt : poly )
									{
										auto radAngle = found->rotation;
										auto x        = pt.x * cos( radAngle ) - pt.y * sin( radAngle );
										auto y        = pt.x * sin( radAngle ) + pt.y * cos( radAngle );

										x             = x + found->x + ( binIndex * rightX );
										y             = y + found->y;

										polyPlaced.points.emplace_back( static_cast<float>( x ),
																		static_cast<float>( y ) );
									}
									m_polygonsPlaced.push_back( polyPlaced );
									startIt = ++found;
									found   = std::find_if( startIt, binPositions.end(),
															[&]( const auto& p ) { return p.id == polyId; } );
								}
								++polyId;
							}
							++binIndex;
							auto info =
								std::format( L"bin {0}: fill-width {1:.1f}mm, {2:.2f}%", binIndex,
											 binPositions.bounds.width(), binPositions.hullArea * 100.0 / binArea );
							if ( !infoString.empty() )
								infoString += L"\n";
							infoString += info;
						}
						m_infoString = infoString;
					}

					std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
					auto                                  milliSeconds =
						std::chrono::duration_cast<std::chrono::milliseconds>( end - m_iterationBegin ).count();
					m_timeString = std::format( L"{0} ms", milliSeconds );
					auto totalMilliSeconds =
						std::chrono::duration_cast<std::chrono::milliseconds>( end - m_begin ).count();
					m_totalTimeString = std::format( L"Total time: {:.1f} s", totalMilliSeconds / 1000.0 );
					m_iterationBegin  = end;
					InvalidateRect( m_hwnd, nullptr, false );
				} );

			clearZoom();
			InvalidateRect( *this, nullptr, TRUE );
			m_nester.doNest( bin, polygons, config );
		}
		break;
		case ID_NEST_STOPNESTING:
		{
			m_nester.stopNest();
			m_timeString.clear();
			InvalidateRect( m_hwnd, nullptr, false );
		}
		break;
		case IDM_EXIT:
			DestroyWindow( *this );
			break;
		default:
			break;
	}
	return 1;
}

void CMainWindow::clearZoom()
{
	m_scale     = 1.0f;
	m_scalePt.x = 0;
	m_scalePt.y = 0;
};

// Message handler for about box.
INT_PTR CALLBACK about( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	UNREFERENCED_PARAMETER( lParam );
	switch ( message )
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if ( LOWORD( wParam ) == IDOK || LOWORD( wParam ) == IDCANCEL )
			{
				EndDialog( hDlg, LOWORD( wParam ) );
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}
