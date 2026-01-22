#pragma once
#include <chrono>
#include <mutex>
#include <vector>

#include "../ZNester/ZNester.h"
#include "BaseWindow.h"
#include "SVG.h"

class CMainWindow : public CWindow
{
public:
	explicit CMainWindow( HINSTANCE hInst, CONST WNDCLASSEX* wcx = nullptr );

	bool registerAndCreateWindow();

private:
	LRESULT CALLBACK                      WinMsgHandler( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) override;
	LRESULT                               doCommand( int id, LPARAM lParam );
	void                                  clearZoom();

	SVG                                   m_svg;
	std::vector<SVGPolygon>               m_polygons;
	std::vector<SVGPolygon>               m_polygonsPlaced;
	std::vector<SVGPolygon>               m_binsPlaced;
	std::vector<SVGPolygon>               m_debugPolys;
	std::vector<SVGPoint>                 m_debugMarkers;
	SVGPolygon                            m_bin;
	std::wstring                          m_timeString;
	std::wstring                          m_totalTimeString;
	std::wstring                          m_infoString;
	ZNester                               m_nester;
	float                                 m_scale = 1.0f;
	POINT                                 m_scalePt{};
	POINT                                 m_downStartPt{};
	bool                                  m_bShowPolygonPoints = false;
	std::chrono::steady_clock::time_point m_iterationBegin;
	std::chrono::steady_clock::time_point m_begin;
	std::mutex                            m_polygonMutex;
};
