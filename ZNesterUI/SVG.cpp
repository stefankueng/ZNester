#include "pch.h"

#include "SVG.h"

#include <wrl.h>

#include <map>

#include "stringtok.h"

using Microsoft::WRL::ComPtr;
#include <comutil.h>

#pragma comment( lib, "GdiPlus.lib" )
#pragma comment( lib, "comsuppw.lib" )

struct COLORTABLE
{
	LPCWSTR	 name;
	COLORREF color;
};

constexpr COLORTABLE S_COLORTABLE[] = {
	{ L"aliceblue", RGB( 240, 248, 255 ) },
	{ L"antiquewhite", RGB( 250, 235, 215 ) },
	{ L"aqua", RGB( 0, 255, 255 ) },
	{ L"aquamarine", RGB( 127, 255, 212 ) },
	{ L"azure", RGB( 240, 255, 255 ) },
	{ L"beige", RGB( 245, 245, 220 ) },
	{ L"bisque", RGB( 255, 228, 196 ) },
	{ L"black", RGB( 0, 0, 0 ) },
	{ L"blanchedalmond", RGB( 255, 235, 205 ) },
	{ L"blue", RGB( 0, 0, 255 ) },
	{ L"blueviolet", RGB( 138, 43, 226 ) },
	{ L"brown", RGB( 165, 42, 42 ) },
	{ L"burlywood", RGB( 222, 184, 135 ) },
	{ L"cadetblue", RGB( 95, 158, 160 ) },
	{ L"chartreuse", RGB( 127, 255, 0 ) },
	{ L"chocolate", RGB( 210, 105, 30 ) },
	{ L"coral", RGB( 255, 127, 80 ) },
	{ L"cornflowerblue", RGB( 100, 149, 237 ) },
	{ L"cornsilk", RGB( 255, 248, 220 ) },
	{ L"crimson", RGB( 220, 20, 60 ) },
	{ L"cyan", RGB( 0, 255, 255 ) },
	{ L"darkblue", RGB( 0, 0, 139 ) },
	{ L"darkcyan", RGB( 0, 139, 139 ) },
	{ L"darkgoldenrod", RGB( 184, 134, 11 ) },
	{ L"darkgray", RGB( 169, 169, 169 ) },
	{ L"darkgreen", RGB( 0, 100, 0 ) },
	{ L"darkgrey", RGB( 169, 169, 169 ) },
	{ L"darkkhaki", RGB( 189, 183, 107 ) },
	{ L"darkmagenta", RGB( 139, 0, 139 ) },
	{ L"darkolivegreen", RGB( 85, 107, 47 ) },
	{ L"darkorange", RGB( 255, 140, 0 ) },
	{ L"darkorchid", RGB( 153, 50, 204 ) },
	{ L"darkred", RGB( 139, 0, 0 ) },
	{ L"darksalmon", RGB( 233, 150, 122 ) },
	{ L"darkseagreen", RGB( 143, 188, 143 ) },
	{ L"darkslateblue", RGB( 72, 61, 139 ) },
	{ L"darkslategray", RGB( 47, 79, 79 ) },
	{ L"darkslategrey", RGB( 47, 79, 79 ) },
	{ L"darkturquoise", RGB( 0, 206, 209 ) },
	{ L"darkviolet", RGB( 148, 0, 211 ) },
	{ L"deeppink", RGB( 255, 20, 147 ) },
	{ L"deepskyblue", RGB( 0, 191, 255 ) },
	{ L"dimgray", RGB( 105, 105, 105 ) },
	{ L"dimgrey", RGB( 105, 105, 105 ) },
	{ L"dodgerblue", RGB( 30, 144, 255 ) },
	{ L"firebrick", RGB( 178, 34, 34 ) },
	{ L"floralwhite", RGB( 255, 250, 240 ) },
	{ L"forestgreen", RGB( 34, 139, 34 ) },
	{ L"fuchsia", RGB( 255, 0, 255 ) },
	{ L"gainsboro", RGB( 220, 220, 220 ) },
	{ L"ghostwhite", RGB( 248, 248, 255 ) },
	{ L"gold", RGB( 255, 215, 0 ) },
	{ L"goldenrod", RGB( 218, 165, 32 ) },
	{ L"gray", RGB( 128, 128, 128 ) },
	{ L"grey", RGB( 128, 128, 128 ) },
	{ L"green", RGB( 0, 128, 0 ) },
	{ L"greenyellow", RGB( 173, 255, 47 ) },
	{ L"honeydew", RGB( 240, 255, 240 ) },
	{ L"hotpink", RGB( 255, 105, 180 ) },
	{ L"indianred", RGB( 205, 92, 92 ) },
	{ L"indigo", RGB( 75, 0, 130 ) },
	{ L"ivory", RGB( 255, 255, 240 ) },
	{ L"khaki", RGB( 240, 230, 140 ) },
	{ L"lavender", RGB( 230, 230, 250 ) },
	{ L"lavenderblush", RGB( 255, 240, 245 ) },
	{ L"lawngreen", RGB( 124, 252, 0 ) },
	{ L"lemonchiffon", RGB( 255, 250, 205 ) },
	{ L"lightblue", RGB( 173, 216, 230 ) },
	{ L"lightcoral", RGB( 240, 128, 128 ) },
	{ L"lightcyan", RGB( 224, 255, 255 ) },
	{ L"lightgoldenrodyellow", RGB( 250, 250, 210 ) },
	{ L"lightgray", RGB( 211, 211, 211 ) },
	{ L"lightgreen", RGB( 144, 238, 144 ) },
	{ L"lightgrey", RGB( 211, 211, 211 ) },

	{ L"lightpink", RGB( 255, 182, 193 ) },
	{ L"lightsalmon", RGB( 255, 160, 122 ) },
	{ L"lightseagreen", RGB( 32, 178, 170 ) },
	{ L"lightskyblue", RGB( 135, 206, 250 ) },
	{ L"lightslategray", RGB( 119, 136, 153 ) },
	{ L"lightslategrey", RGB( 119, 136, 153 ) },
	{ L"lightsteelblue", RGB( 176, 196, 222 ) },
	{ L"lightyellow", RGB( 255, 255, 224 ) },
	{ L"lime", RGB( 0, 255, 0 ) },
	{ L"limegreen", RGB( 50, 205, 50 ) },
	{ L"linen", RGB( 250, 240, 230 ) },
	{ L"magenta", RGB( 255, 0, 255 ) },
	{ L"maroon", RGB( 128, 0, 0 ) },
	{ L"mediumaquamarine", RGB( 102, 205, 170 ) },
	{ L"mediumblue", RGB( 0, 0, 205 ) },
	{ L"mediumorchid", RGB( 186, 85, 211 ) },
	{ L"mediumpurple", RGB( 147, 112, 219 ) },
	{ L"mediumseagreen", RGB( 60, 179, 113 ) },
	{ L"mediumslateblue", RGB( 123, 104, 238 ) },
	{ L"mediumspringgreen", RGB( 0, 250, 154 ) },
	{ L"mediumturquoise", RGB( 72, 209, 204 ) },
	{ L"mediumvioletred", RGB( 199, 21, 133 ) },
	{ L"midnightblue", RGB( 25, 25, 112 ) },
	{ L"mintcream", RGB( 245, 255, 250 ) },
	{ L"mistyrose", RGB( 255, 228, 225 ) },
	{ L"moccasin", RGB( 255, 228, 181 ) },
	{ L"navajowhite", RGB( 255, 222, 173 ) },
	{ L"navy", RGB( 0, 0, 128 ) },
	{ L"oldlace", RGB( 253, 245, 230 ) },
	{ L"olive", RGB( 128, 128, 0 ) },
	{ L"olivedrab", RGB( 107, 142, 35 ) },
	{ L"orange", RGB( 255, 165, 0 ) },
	{ L"orangered", RGB( 255, 69, 0 ) },
	{ L"orchid", RGB( 218, 112, 214 ) },
	{ L"palegoldenrod", RGB( 238, 232, 170 ) },
	{ L"palegreen", RGB( 152, 251, 152 ) },
	{ L"paleturquoise", RGB( 175, 238, 238 ) },
	{ L"palevioletred", RGB( 219, 112, 147 ) },
	{ L"papayawhip", RGB( 255, 239, 213 ) },
	{ L"peachpuff", RGB( 255, 218, 185 ) },
	{ L"peru", RGB( 205, 133, 63 ) },
	{ L"pink", RGB( 255, 192, 203 ) },
	{ L"plum", RGB( 221, 160, 221 ) },
	{ L"powderblue", RGB( 176, 224, 230 ) },
	{ L"purple", RGB( 128, 0, 128 ) },
	{ L"red", RGB( 255, 0, 0 ) },
	{ L"rosybrown", RGB( 188, 143, 143 ) },
	{ L"royalblue", RGB( 65, 105, 225 ) },
	{ L"saddlebrown", RGB( 139, 69, 19 ) },
	{ L"salmon", RGB( 250, 128, 114 ) },
	{ L"sandybrown", RGB( 244, 164, 96 ) },
	{ L"seagreen", RGB( 46, 139, 87 ) },
	{ L"seashell", RGB( 255, 245, 238 ) },
	{ L"sienna", RGB( 160, 82, 45 ) },
	{ L"silver", RGB( 192, 192, 192 ) },
	{ L"skyblue", RGB( 135, 206, 235 ) },
	{ L"slateblue", RGB( 106, 90, 205 ) },
	{ L"slategray", RGB( 112, 128, 144 ) },
	{ L"slategrey", RGB( 112, 128, 144 ) },
	{ L"snow", RGB( 255, 250, 250 ) },
	{ L"springgreen", RGB( 0, 255, 127 ) },
	{ L"steelblue", RGB( 70, 130, 180 ) },
	{ L"tan", RGB( 210, 180, 140 ) },
	{ L"teal", RGB( 0, 128, 128 ) },
	{ L"thistle", RGB( 216, 191, 216 ) },
	{ L"tomato", RGB( 255, 99, 71 ) },
	{ L"turquoise", RGB( 64, 224, 208 ) },
	{ L"violet", RGB( 238, 130, 238 ) },
	{ L"wheat", RGB( 245, 222, 179 ) },
	{ L"white", RGB( 255, 255, 255 ) },
	{ L"whitesmoke", RGB( 245, 245, 245 ) },
	{ L"yellow", RGB( 255, 255, 0 ) },
	{ L"yellowgreen", RGB( 154, 205, 50 ) },
};

// trim from start
std::wstring& ltrim( std::wstring& s )
{
	s.erase( s.begin(), std::ranges::find_if( s, []( wint_t c ) { return !iswspace( c ); } ) );
	return s;
}

// trim from end
std::wstring& rtrim( std::wstring& s )
{
	s.erase( std::find_if( s.rbegin(), s.rend(), []( wint_t c ) { return !iswspace( c ); } ).base(), s.end() );
	return s;
}

std::wstring& trim( std::wstring& s )
{
	return ltrim( rtrim( s ) );
}

inline float pow2( float x )
{
	return x * x;
}

float toFloat( const std::wstring& lpData, float defaultValue = 0.0f )
{
	if ( lpData.empty() )
		return defaultValue;
	return static_cast<float>( _wtof( lpData.c_str() ) );
}

int toInt( const std::wstring& lpData )
{
	return _wtoi( lpData.c_str() );
}

int lookupColorTable( LPCTSTR lpName, COLORREF& color )
{
	for ( size_t i = 0; i < _countof( S_COLORTABLE ); i++ )
	{
		if ( _wcsicmp( lpName, S_COLORTABLE[i].name ) == 0 )
		{
			color = S_COLORTABLE[i].color;
			return static_cast<int>( i );
		}
	}
	return -1;
}

void searchReplace( std::wstring& str, const std::wstring& toReplace, const std::wstring& replaceWith )
{
	std::wstring			result;
	std::wstring::size_type pos = 0;
	for ( ;; )
	{
		std::wstring::size_type next = str.find( toReplace, pos );
		result.append( str, pos, next - pos );
		if ( next != std::wstring::npos )
		{
			result.append( replaceWith );
			pos = next + toReplace.size();
		}
		else
		{
			break;	// exit loop
		}
	}
	str = std::move( result );
}

void makeDictFromString( LPCTSTR lpString, std::map<std::wstring, std::wstring>& rDict )
{
	std::wstring strKey, strValue;
	bool		 bKey = true;
	for ( ; *lpString; lpString++ )
	{
		if ( *lpString == L'\n' || *lpString == L'\r' )
		{
			continue;
		}
		else if ( *lpString == L':' )
		{
			bKey = false;
		}
		else if ( *lpString == L';' )
		{
			if ( !strKey.empty() )
			{
				bKey		  = true;
				strKey		  = trim( strKey );
				strValue	  = trim( strValue );
				rDict[strKey] = strValue;
				strKey.clear();
				strValue.clear();
			}
		}
		else
		{
			if ( bKey )
			{
				strKey += *lpString;
			}
			else
			{
				strValue += *lpString;
			}
		}
	}

	if ( !strKey.empty() )
	{
		strKey		  = trim( strKey );
		strValue	  = trim( strValue );
		rDict[strKey] = strValue;
	}
}

HRESULT recursiveBuildVectorImage( SVG& image, ComPtr<IXMLDOMNode> node );

void	SVG::clear()
{
	m_elements.clear();
}

bool SVG::parseFile( const std::wstring& path )
{
	clear();
	// initialize XML Parser
	ComPtr<IXMLDOMDocument> document;
	HRESULT					hResult =
		CoCreateInstance( CLSID_DOMDocument, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( document.GetAddressOf() ) );
	if ( FAILED( hResult ) )
		return false;

	document->put_resolveExternals( VARIANT_FALSE );  // DTD?
	document->put_validateOnParse( VARIANT_FALSE );	  // validate?
	document->put_async( VARIANT_FALSE );			  // set flag to wait for parser

	// load SVG document
	VARIANT_BOOL verbResult;
	// hResult=document->loadXML(_bstr_t((char*)&rawXML[0]),&verbResult);
	_variant_t fName( path.c_str() );
	hResult = document->load( fName, &verbResult );
	if ( FAILED( hResult ) || !verbResult )
		return false;

	// get root element
	ComPtr<IXMLDOMElement> root;
	document->get_documentElement( root.GetAddressOf() );

	ComPtr<IXMLDOMNodeList> resultList;
	if ( SUCCEEDED( document->getElementsByTagName( _bstr_t( L"svg" ), resultList.GetAddressOf() ) ) && resultList )
	{
		ComPtr<IXMLDOMNode> nodeSVG;
		if ( SUCCEEDED( resultList->get_item( 0, nodeSVG.GetAddressOf() ) ) )
		{
			recursiveBuildVectorImage( *this, nodeSVG );
		}
	}
	return S_OK;
}
void SVG::render( Gdiplus::Graphics& graphics ) const
{
	Gdiplus::SmoothingMode		sm	 = graphics.GetSmoothingMode();
	Gdiplus::PixelOffsetMode	pom	 = graphics.GetPixelOffsetMode();
	Gdiplus::CompositingMode	comm = graphics.GetCompositingMode();
	Gdiplus::CompositingQuality comq = graphics.GetCompositingQuality();

	graphics.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );
	graphics.SetPixelOffsetMode( Gdiplus::PixelOffsetModeHighQuality );
	graphics.SetCompositingMode( Gdiplus::CompositingModeSourceOver );
	graphics.SetCompositingQuality(
		Gdiplus::CompositingQualityDefault );  // changing the CompositionQualityMode may affect the composited color

	for ( const auto& element : m_elements )
	{
		element->render( graphics, m_canvasWidth, m_canvasHeight );
	}

	graphics.SetSmoothingMode( sm );
	graphics.SetPixelOffsetMode( pom );
	graphics.SetCompositingMode( comm );
	graphics.SetCompositingQuality( comq );
}

void SVG::render( Gdiplus::Graphics& graphics, float left, float top, float scaleX, float scaleY ) const
{
	Gdiplus::Matrix matOld, matTrans;
	matTrans.Translate( left, top );
	matTrans.Scale( scaleX, scaleY );
	graphics.GetTransform( &matOld );
	graphics.SetTransform( &matTrans );

	render( graphics );

	graphics.SetTransform( &matOld );
}

std::vector<SVGPolygon> SVG::getPolygons() const
{
	std::vector<SVGPolygon> polygons;
	for ( const auto& element : m_elements )
	{
		auto polys = element->getPolygons();
		polygons.insert( polygons.end(), polys.begin(), polys.end() );
	}
	return polygons;
}

void SVG::setCanvasSize( float width, float height )
{
	m_canvasWidth  = width;
	m_canvasHeight = height;
}

float SVG::getCanvasWidth() const
{
	return m_canvasWidth;
}

float SVG::getCanvasHeight() const
{
	return m_canvasHeight;
}

SVGElementRect* SVG::addRect( float x, float y, float width, float height, float rx, float ry )
{
	auto pRect = std::make_unique<SVGElementRect>();
	auto ptr   = pRect.get();
	pRect->setParams( x, y, width, height, rx, ry );
	m_elements.push_back( std::move( pRect ) );
	return ptr;
}

SVGElementCircle* SVG::addCircle( float x, float y, float r )
{
	auto pCircle = std::make_unique<SVGElementCircle>();
	auto ptr	 = pCircle.get();
	pCircle->setParams( x, y, r );
	m_elements.push_back( std::move( pCircle ) );
	return ptr;
}

SVGElementPath* SVG::addPath()
{
	auto pPath = std::make_unique<SVGElementPath>();
	auto ptr   = pPath.get();
	m_elements.push_back( std::move( pPath ) );
	return ptr;
}

SVGElementGroup* SVG::addGroup()
{
	auto pGroup = std::make_unique<SVGElementGroup>();
	auto ptr	= pGroup.get();
	m_elements.push_back( std::move( pGroup ) );
	return ptr;
}

void SVGElement::setFill( std::unique_ptr<Gdiplus::Brush>&& brush )
{
	m_fill = std::move( brush );
}

void SVGElement::setStroke( std::unique_ptr<Gdiplus::Pen>&& stroke )
{
	m_stroke = std::move( stroke );
}

void SVGElement::pushRotation( float angle )
{
	m_matrix.Rotate( angle );
}

void SVGElement::pushRotation( float angle, float cx, float cy )
{
	Gdiplus::PointF center( cx, cy );
	m_matrix.RotateAt( angle, center );
}

void SVGElement::pushScale( float sx, float sy )
{
	m_matrix.Scale( sx, sy );
}

void SVGElement::pushTranslation( float tx, float ty )
{
	m_matrix.Translate( tx, ty );
}

void SVGElement::pushSkewX( float angle )
{
	Gdiplus::Matrix mat( 1.0f, 0.0f, tanf( angle ), 1.0f, 0.0f, 0.0f );
	m_matrix.Multiply( &mat );
}

void SVGElement::pushSkewY( float angle )
{
	Gdiplus::Matrix mat( 1.0f, tanf( angle ), 0.0f, 1.0f, 0.0f, 0.0f );
	m_matrix.Multiply( &mat );
}

void SVGElement::pushMatrix( float a, float b, float c, float d, float e, float f )
{
	Gdiplus::Matrix mat( a, b, c, d, e, f );
	m_matrix.Multiply( &mat );
}

void SVGElement::setTransform( const Gdiplus::Matrix& mat )
{
	m_matrix.Reset();
	m_matrix.Multiply( &mat );
}

void SVGElementRect::setParams( float x, float y, float width, float height, float rx, float ry )
{
	m_x		 = x;
	m_y		 = y;
	m_width	 = width;
	m_height = height;
	m_rx	 = rx;
	m_ry	 = ry;
}

void SVGElementRect::render( Gdiplus::Graphics& graphics, float, float )
{
	// copy & multiply : representing matrix stack
	Gdiplus::Matrix matOld;
	graphics.GetTransform( &matOld );
	graphics.MultiplyTransform( &m_matrix );

	float rx = m_rx, ry = m_ry;
	if ( rx > 0.0f || ry > 0.0f )
	{
		// drawing a rounded rectangle with path as combination of arcs
		if ( rx == 0.0f )
		{
			rx = ry;
		}
		else if ( ry == 0.0f )
		{
			ry = rx;
		}
		Gdiplus::GraphicsPath path;
		path.SetFillMode( Gdiplus::FillModeWinding );
		path.AddArc( m_x + m_width - rx, m_y, rx, ry, 270, 90 );
		path.AddArc( m_x + m_width - rx, m_y + m_height - ry, rx, ry, 0, 90 );
		path.AddArc( m_x, m_y + m_height - ry, rx, ry, 90, 90 );
		path.AddArc( m_x, m_y, rx, ry, 180, 90 );
		path.CloseFigure();

		if ( m_fill )
		{
			graphics.FillPath( m_fill.get(), &path );
		}
		if ( m_stroke )
		{
			graphics.DrawPath( m_stroke.get(), &path );
		}
	}
	else
	{
		// draw a pure rectangle
		if ( m_fill )
		{
			graphics.FillRectangle( m_fill.get(), m_x, m_y, m_width, m_height );
		}
		if ( m_stroke )
		{
			graphics.DrawRectangle( m_stroke.get(), m_x, m_y, m_width, m_height );
		}
	}
	graphics.SetTransform( &matOld );
}

std::vector<SVGPolygon> SVGElementRect::getPolygons() const
{
	SVGPolygon poly;
	float	   rx = m_rx, ry = m_ry;
	if ( rx > 0.0f || ry > 0.0f )
	{
		if ( rx == 0.0f )
		{
			rx = ry;
		}
		else if ( ry == 0.0f )
		{
			ry = rx;
		}
		Gdiplus::GraphicsPath path;
		path.SetFillMode( Gdiplus::FillModeWinding );
		path.AddArc( m_x + m_width - rx, m_y, rx, ry, 270, 90 );
		path.AddArc( m_x + m_width - rx, m_y + m_height - ry, rx, ry, 0, 90 );
		path.AddArc( m_x, m_y + m_height - ry, rx, ry, 90, 90 );
		path.AddArc( m_x, m_y, rx, ry, 180, 90 );
		path.CloseFigure();
		path.Flatten( nullptr, 1.0 );
		auto count	= path.GetPointCount();
		auto points = std::make_unique<Gdiplus::PointF[]>( count );
		path.GetPathPoints( points.get(), count );
		m_matrix.TransformPoints( points.get(), count );
		for ( int i = 0; i < count; ++i )
		{
			poly.points.emplace_back( points[i].X, points[i].Y );
		}
	}
	else
	{
		poly.points.emplace_back( m_x, m_y );
		poly.points.emplace_back( m_x, m_y + m_height );
		poly.points.emplace_back( m_x + m_width, m_y + m_height );
		poly.points.emplace_back( m_x + m_width, m_y );
	}
	return { poly };
}

void SVGElementCircle::setParams( float x, float y, float r )
{
	m_x = x;
	m_y = y;
	m_r = r;
}

void SVGElementCircle::render( Gdiplus::Graphics& graphics, float, float )
{
	// copy & multiply : representing matrix stack
	Gdiplus::Matrix matOld;
	graphics.GetTransform( &matOld );
	float e[6];
	matOld.GetElements( e );
	Gdiplus::Matrix mat;
	mat.SetElements( e[0], e[1], e[2], e[3], e[4], e[5] );
	mat.Multiply( &m_matrix );

	graphics.SetTransform( &mat );

	// draw a circle
	if ( m_fill )
	{
		graphics.FillEllipse( m_fill.get(), m_x - m_r, m_y - m_r, m_r * 2.0f, m_r * 2.0f );
	}
	if ( m_stroke )
	{
		graphics.DrawEllipse( m_stroke.get(), m_x - m_r, m_y - m_r, m_r * 2.0f, m_r * 2.0f );
	}

	graphics.SetTransform( &matOld );
}

std::vector<SVGPolygon> SVGElementCircle::getPolygons() const
{
	SVGPolygon			  poly;
	Gdiplus::GraphicsPath path;
	path.SetFillMode( Gdiplus::FillModeWinding );
	path.AddEllipse( m_x - m_r, m_y - m_r, m_r * 2.0f, m_r * 2.0f );
	path.Flatten( nullptr, 1.0 );
	auto count	= path.GetPointCount();
	auto points = std::make_unique<Gdiplus::PointF[]>( count );
	path.GetPathPoints( points.get(), count );
	m_matrix.TransformPoints( points.get(), count );
	for ( int i = 0; i < count; ++i )
	{
		poly.points.emplace_back( points[i].X, points[i].Y );
	}

	return { poly };
}

float SVGElementPath::getAngle( const Gdiplus::PointF& a, const Gdiplus::PointF& b )
{
	// should be atan2?
	float prod = a.X * b.X + a.Y * b.Y;
	float div  = sqrtf( ( pow2( a.X ) + pow2( a.Y ) ) * ( pow2( b.X ) + pow2( b.Y ) ) );
	float sign = ( a.X * b.Y - a.Y * b.X ) > 0.0f ? 1.0f : -1.0f;
	return acosf( prod / div ) * sign * 180.0f / static_cast<float>( M_PI );
}

SVGElementPath::SVGElementPath()
	: m_bFirst( true )
{
	m_path.SetFillMode( Gdiplus::FillModeAlternate );
}

void SVGElementPath::addString( LPCWSTR lpText, const Gdiplus::FontFamily* lpFontFamily, Gdiplus::FontStyle style,
								float fontSize, float x, float y )
{
	m_path.AddString( lpText, -1, lpFontFamily, style, fontSize, Gdiplus::PointF( x, y ),
					  Gdiplus::StringFormat::GenericTypographic() );
}

void SVGElementPath::lineTo( float x, float y, bool bAbsolute )
{
	Gdiplus::PointF currentPos( x, y );
	if ( !bAbsolute )
	{
		currentPos = currentPos + m_lastPos;
	}
	if ( m_bFirst )
	{
		m_firstPos = currentPos;
	}
	m_bFirst = false;

	m_path.AddLine( m_lastPos, currentPos );

	m_lastPos = currentPos;
	m_lastC2  = m_lastPos;
}

void SVGElementPath::horizontalLineTo( float x, bool bAbsolute )
{
	Gdiplus::PointF currentPos( x, m_lastPos.Y );
	if ( !bAbsolute )
	{
		currentPos.X += m_lastPos.X;
	}
	if ( m_bFirst )
	{
		m_firstPos = currentPos;
	}
	m_bFirst = false;

	m_path.AddLine( m_lastPos, currentPos );

	m_lastPos = currentPos;
	m_lastC2  = m_lastPos;
}

void SVGElementPath::verticalLineTo( float y, bool bAbsolute )
{
	Gdiplus::PointF currentPos( m_lastPos.X, y );
	if ( !bAbsolute )
	{
		currentPos.Y += m_lastPos.Y;
	}
	if ( m_bFirst )
	{
		m_firstPos = currentPos;
	}
	m_bFirst = false;

	m_path.AddLine( m_lastPos, currentPos );

	m_lastPos = currentPos;
	m_lastC2  = m_lastPos;
}

void SVGElementPath::moveTo( float x, float y, bool bAbsolute )
{
	Gdiplus::PointF currentPos( x, y );
	if ( !bAbsolute )
	{
		currentPos = currentPos + m_lastPos;
	}
	if ( m_bFirst )
	{
		m_firstPos = currentPos;
	}
	m_bFirst  = false;

	m_lastPos = currentPos;
	m_lastC2  = m_lastPos;
}

void SVGElementPath::bezier( float x1, float y1, float x2, float y2, float endX, float endY, bool bAbsolute )
{
	Gdiplus::PointF c1( x1, y1 );
	Gdiplus::PointF c2( x2, y2 );
	Gdiplus::PointF end( endX, endY );
	if ( !m_bFirst && !bAbsolute )
	{
		c1	= c1 + m_lastPos;
		c2	= c2 + m_lastPos;
		end = end + m_lastPos;
	}
	if ( m_bFirst )
	{
		m_firstPos = m_lastPos;
	}
	m_bFirst = false;

	m_path.AddBezier( m_lastPos, c1, c2, end );

	m_lastC2  = c2;
	m_lastPos = end;
}

void SVGElementPath::smoothCubicBezier( float x2, float y2, float endX, float endY, bool bAbsolute )
{
	Gdiplus::PointF c2( x2, y2 );
	Gdiplus::PointF end( endX, endY );
	if ( !m_bFirst && !bAbsolute )
	{
		c2	= c2 + m_lastPos;
		end = end + m_lastPos;
	}
	if ( m_bFirst )
	{
		m_firstPos = m_lastPos;
	}
	m_bFirst = false;
	Gdiplus::PointF c1(
		m_lastPos.X * 2.0f - m_lastC2.X,
		m_lastPos.Y * 2.0f - m_lastC2.Y );	// reflection of the second control point of the previous curve

	m_path.AddBezier( m_lastPos, c1, c2, end );

	m_lastC2  = c2;
	m_lastPos = end;
}

void SVGElementPath::quadraticBezier( float x1, float y1, float endX, float endY, bool bAbsolute )
{
	Gdiplus::PointF c1( x1, y1 );
	Gdiplus::PointF end( endX, endY );
	if ( !m_bFirst && !bAbsolute )
	{
		c1	= c1 + m_lastPos;
		end = end + m_lastPos;
	}
	if ( m_bFirst )
	{
		m_firstPos = m_lastPos;
	}
	m_bFirst = false;

	m_path.AddBezier( m_lastPos, c1, c1, end );

	m_lastC2  = c1;
	m_lastPos = end;
}

void SVGElementPath::smoothQuadraticBezier( float endX, float endY, bool bAbsolute )
{
	Gdiplus::PointF end( endX, endY );
	if ( !m_bFirst && !bAbsolute )
	{
		end = end + m_lastPos;
	}
	if ( m_bFirst )
	{
		m_firstPos = m_lastPos;
	}
	m_bFirst = false;
	Gdiplus::PointF c1( m_lastPos.X * 2.0f - m_lastC2.X,
						m_lastPos.Y * 2.0f - m_lastC2.Y );	// reflection of the control point of the previous curve

	m_path.AddBezier( m_lastPos, c1, c1, end );

	m_lastC2  = c1;
	m_lastPos = end;
}

void SVGElementPath::arcTo( float rx, float ry, float xAxisRotation, int largeARCFlag, int sweepFlag, float x2,
							float y2, bool bAbsolute )
{
	rx = fabsf( rx );
	ry = fabsf( ry );
	if ( rx < 1.0e-5f || ry < 1.0e-5f )
	{
		// Ensure radii are non-zero
		lineTo( x2, y2, bAbsolute );
		return;
	}

	if ( !m_bFirst && !bAbsolute )
	{
		x2 += m_lastPos.X;
		y2 += m_lastPos.Y;
	}
	if ( m_bFirst )
	{
		m_firstPos = m_lastPos;
	}
	m_bFirst   = false;

	float x1   = m_lastPos.X;
	float y1   = m_lastPos.Y;

	float cPhi = cosf( xAxisRotation );
	float sPhi = sinf( xAxisRotation );

	// step1
	float dx	= x1 - x2;
	float dy	= y1 - y2;
	float x1Phi = ( cPhi * dx + sPhi * dy ) * 0.5f;
	float y1Phi = ( -sPhi * dx + cPhi * dy ) * 0.5f;

	// Ensure radii are large enough
	float lambda = pow2( x1Phi / rx ) + pow2( y1Phi / ry );
	if ( lambda > 1.0f )
	{
		rx *= sqrtf( lambda );
		ry *= sqrtf( lambda );
	}

	// step2
	float sign	= ( largeARCFlag == sweepFlag ) ? -1.0f : 1.0f;
	float scale = sqrtf( ( pow2( rx * ry ) - pow2( rx * y1Phi ) - pow2( ry * x1Phi ) ) /
						 ( pow2( rx * y1Phi ) + pow2( ry * x1Phi ) ) );
	float cxPhi = sign * scale * rx * y1Phi / ry;
	float cyPhi = -sign * scale * ry * x1Phi / rx;

	// step3
	float cx = ( cPhi * cxPhi - sPhi * cyPhi ) + ( x1 + x2 ) * 0.5f;
	float cy = ( sPhi * cxPhi + cPhi * cyPhi ) + ( y1 + y2 ) * 0.5f;

	// step4
	Gdiplus::PointF u( ( x1Phi - cxPhi ) / rx, ( y1Phi - cyPhi ) / ry );
	Gdiplus::PointF v( ( -x1Phi - cxPhi ) / rx, ( -y1Phi - cyPhi ) / ry );
	float			theta	   = getAngle( Gdiplus::PointF( 1.0f, 0.0f ), u );
	float			deltaTheta = getAngle( u, v );

	if ( sweepFlag == 0 && deltaTheta > 0.0f )
		deltaTheta -= 360.0f;
	else if ( sweepFlag == 1 && deltaTheta < 0.0f )
		deltaTheta += 360.0f;

	// add a path
	m_path.AddArc( cx - rx, cy - ry, rx * 2.0f, ry * 2.0f, theta, deltaTheta );

	m_lastPos.X = x2;
	m_lastPos.Y = y2;
	m_lastC2	= m_lastPos;
}

void SVGElementPath::startPath()
{
	m_path.StartFigure();

	m_bFirst = true;
	//_lastPos=_firstPos;
	//_lastC2=_lastPos;
}

void SVGElementPath::closePath()
{
	m_path.CloseFigure();

	m_bFirst  = true;
	m_lastPos = m_firstPos;
	m_lastC2  = m_lastPos;
}

bool SVGElementPath::isClosed() const
{
	return m_bFirst;
}

const Gdiplus::PointF& SVGElementPath::getLastPos() const
{
	return m_lastPos;
}

void SVGElementPath::render( Gdiplus::Graphics& graphics, float, float )
{
	// copy & multiply : representing matrix stack
	Gdiplus::Matrix matOld;
	graphics.GetTransform( &matOld );
	float e[6];
	matOld.GetElements( e );
	Gdiplus::Matrix mat;
	mat.SetElements( e[0], e[1], e[2], e[3], e[4], e[5] );
	mat.Multiply( &m_matrix );

	graphics.SetTransform( &mat );

	if ( m_fill )
	{
		graphics.FillPath( m_fill.get(), &m_path );
	}
	if ( m_stroke )
	{
		graphics.DrawPath( m_stroke.get(), &m_path );
	}

	graphics.SetTransform( &matOld );
}

std::vector<SVGPolygon> SVGElementPath::getPolygons() const
{
	std::vector<SVGPolygon> polys;
	SVGPolygon				poly;
	auto					path = std::unique_ptr<Gdiplus::GraphicsPath>( m_path.Clone() );
	path->Flatten( nullptr, 1.0 );
	auto count	= path->GetPointCount();
	auto points = std::make_unique<Gdiplus::PointF[]>( count );
	auto types	= std::make_unique<BYTE[]>( count );
	path->GetPathTypes( types.get(), count );
	path->GetPathPoints( points.get(), count );
	m_matrix.TransformPoints( points.get(), count );

	for ( int i = 0; i < count; ++i )
	{
		if ( i > 0 && types[i] == 0 )
		{
			polys.push_back( poly );
			poly.points.clear();
		}
		poly.points.emplace_back( points[i].X, points[i].Y );
	}
	polys.push_back( poly );

	return polys;
}

void SVGElementPath::getBounds( Gdiplus::RectF& bounds ) const
{
	m_path.GetBounds( &bounds, nullptr, nullptr );
}

void SVGElementGroup::setOpacity( float opacity )
{
	m_opacity = opacity;
}

void SVGElementGroup::render( Gdiplus::Graphics& parentGraphics, float cX, float cY )
{
	// temporary copy parent transform
	Gdiplus::Matrix matTransform;
	parentGraphics.GetTransform( &matTransform );
	float matElements[6];
	matTransform.GetElements( matElements );
	float canvasX = cX * matElements[0] + cY * matElements[1];
	float canvasY = cX * matElements[2] + cY * matElements[3];

	m_svg.setCanvasSize( canvasX, canvasY );
	UINT			  width = static_cast<UINT>( canvasX + 0.5 ), height = static_cast<UINT>( canvasY + 0.5 );
	Gdiplus::Bitmap*  pBitmap = new Gdiplus::Bitmap( width, height );
	Gdiplus::Graphics graphics( pBitmap );
	graphics.SetTransform( &matTransform );

	// render children
	m_svg.render( graphics );

	// paste on parent graphics object
	Gdiplus::ColorMatrix cmat = {
		1.0f, 0.0f, 0.0f, 0.0f,		 0.0f,	// Red
		0.0f, 1.0f, 0.0f, 0.0f,		 0.0f,	// Green
		0.0f, 0.0f, 1.0f, 0.0f,		 0.0f,	// Blue
		0.0f, 0.0f, 0.0f, m_opacity, 0.0f,	// Alpha
		0.0f, 0.0f, 0.0f, 0.0f,		 1.0f	// must be 1
	};
	Gdiplus::ImageAttributes imgAttr;
	imgAttr.SetColorMatrix( &cmat );

	Gdiplus::RectF rect( 0.0f, 0.0f, canvasX, canvasY );

	parentGraphics.ResetTransform();
	parentGraphics.DrawImage( pBitmap, rect, 0.0f, 0.0f, canvasX, canvasY, Gdiplus::UnitPixel, &imgAttr );

	parentGraphics.SetTransform( &matTransform );
	delete pBitmap;
}

std::vector<SVGPolygon> SVGElementGroup::getPolygons() const
{
	return m_svg.getPolygons();
}

SVG& SVGElementGroup::getInternalSVG()
{
	return m_svg;
}

void domMakeAttributeDict( ComPtr<IXMLDOMElement> element, std::map<std::wstring, std::wstring>& rDict )
{
	ComPtr<IXMLDOMNamedNodeMap> attributes;
	element->get_attributes( attributes.GetAddressOf() );

	while ( attributes )
	{
		ComPtr<IXMLDOMNode> attribute;
		attributes->nextNode( attribute.GetAddressOf() );
		if ( !attribute )
			break;

		_bstr_t strKey;
		attribute->get_nodeName( strKey.GetAddress() );
		_variant_t strValue;
		attribute->get_nodeValue( &strValue );
		strValue.ChangeType( VT_BSTR );

		rDict[static_cast<LPCWSTR>( strKey )] = strValue.bstrVal;
	}
}

_bstr_t domGetString( ComPtr<IXMLDOMElement> element, const std::wstring& key, const std::wstring& defaultValue = L"" )
{
	_variant_t value;
	_bstr_t	   bKey = key.c_str();
	if ( SUCCEEDED( element->getAttribute( bKey, &value ) ) )
	{
		if ( value.vt != VT_NULL && value.vt != VT_EMPTY )
			value.ChangeType( VT_BSTR );
		if ( value.bstrVal )
			return value.bstrVal;
		else
			return L"";
	}
	else
	{
		return defaultValue.c_str();
	}
}

float domGetFloat( ComPtr<IXMLDOMElement> element, const std::wstring& key, float defaultValue = 0.0f )
{
	_variant_t value;
	if ( SUCCEEDED( element->getAttribute( _bstr_t( key.c_str() ), &value ) ) )
	{
		// value.ChangeType(VT_R4);
		// return value.fltVal;
		value.ChangeType( VT_BSTR );
		if ( value.bstrVal )
		{
			return toFloat( ( LPCWSTR )value.bstrVal );
		}
		else
		{
			return defaultValue;
		}
	}
	else
	{
		return defaultValue;
	}
}

HRESULT domGetElementById( ComPtr<IXMLDOMNode> parent, const std::wstring& lpID, ComPtr<IXMLDOMNode>& p )
{
	DOMNodeType type;
	if ( SUCCEEDED( parent->get_nodeType( &type ) ) )
	{
		if ( type == NODE_ELEMENT )
		{
			ComPtr<IXMLDOMElement> element;
			parent.As( &element );
			auto strID = domGetString( element, L"id" );
			if ( wcscmp( strID, lpID.c_str() ) == 0 )
			{
				p = parent;
				return S_OK;
			}
		}
	}

	ComPtr<IXMLDOMNodeList> children;
	if ( SUCCEEDED( parent->get_childNodes( &children ) ) )
	{
		while ( true )
		{
			ComPtr<IXMLDOMNode> child;
			if ( FAILED( children->nextNode( &child ) ) || !child )
				break;

			HRESULT hr = domGetElementById( child, lpID, p );
			if ( hr == S_OK )
				return S_OK;
		}
	}
	return S_FALSE;
}

float parseDistance( const std::wstring& strValue )
{
	float scale = 1.0f;
	if ( strValue.find( L"cm" ) != std::wstring::npos )
	{
		scale = 72.0f / 2.54f;
	}
	else if ( strValue.find( L"mm" ) != std::wstring::npos )
	{
		scale = 7.2f / 2.54f;
	}
	else if ( strValue.find( L"in" ) != std::wstring::npos )
	{
		scale = 72.0f;
	}
	else if ( strValue.find( L"em" ) != std::wstring::npos )
	{
		scale = 12.0f;
	}
	else if ( strValue.find( L"ex" ) != std::wstring::npos )
	{
		scale = 8.0f;
	}
	else if ( strValue.find( L"pc" ) != std::wstring::npos )
	{
		scale = 8.0f;
	}

	return toFloat( strValue ) * scale;
}

float domGetDistance( ComPtr<IXMLDOMElement> element, const std::wstring& key, float defaultValue = 0.0f )
{
	_variant_t value;
	if ( SUCCEEDED( element->getAttribute( _bstr_t( key.c_str() ), &value ) ) )
	{
		value.ChangeType( VT_BSTR );
		if ( value.bstrVal )
		{
			return parseDistance( value.bstrVal );
		}
		else
		{
			return defaultValue;
		}
	}
	else
	{
		return defaultValue;
	}
}

Gdiplus::Color parseHexColor( const std::wstring& strColor, const std::wstring& strOpacity )
{
	BYTE r = 0, g = 0, b = 0, a = 255;
	if ( strColor.size() == 6 )
	{
		// 6-digits color code
		r = static_cast<BYTE>( wcstol( strColor.substr( 0, 2 ).c_str(), nullptr, 16 ) );
		g = static_cast<BYTE>( wcstol( strColor.substr( 2, 2 ).c_str(), nullptr, 16 ) );
		b = static_cast<BYTE>( wcstol( strColor.substr( 4, 2 ).c_str(), nullptr, 16 ) );
	}
	else
	{
		// 3-digits color code
		r = static_cast<BYTE>( wcstol( strColor.substr( 0, 1 ).c_str(), nullptr, 16 ) * 17 );  // 17=16+1, 0xFF=0xF*17
		g = static_cast<BYTE>( wcstol( strColor.substr( 1, 1 ).c_str(), nullptr, 16 ) * 17 );
		b = static_cast<BYTE>( wcstol( strColor.substr( 2, 1 ).c_str(), nullptr, 16 ) * 17 );
	}
	if ( !strOpacity.empty() )
	{
		a = static_cast<BYTE>(
			std::min( 255, static_cast<int>( fmax( 0.0f, toFloat( strOpacity, 1.0f ) ) * 255.0f ) ) );
	}

	return Gdiplus::Color( a, r, g, b );
}

HRESULT recursiveMakePropDict( ComPtr<IXMLDOMElement> element, std::map<std::wstring, std::wstring>& propDict )
{
	if ( !element )
		return E_POINTER;
	// inherit parent styles
	ComPtr<IXMLDOMNode> p;
	if ( SUCCEEDED( element->get_parentNode( &p ) ) && p )
	{
		ComPtr<IXMLDOMElement> elem;
		p.As( &elem );
		recursiveMakePropDict( elem, propDict );
	}

	makeDictFromString( domGetString( element, L"style" ), propDict );
	domMakeAttributeDict( element, propDict );

	return S_OK;
}

HRESULT parseTransform( Gdiplus::Matrix& mat, ComPtr<IXMLDOMElement> element, const std::wstring& lpKey = L"transform" )
{
	if ( !element )
		return E_POINTER;
	// inherit parent styles
	ComPtr<IXMLDOMNode> p;
	if ( SUCCEEDED( element->get_parentNode( &p ) ) && p )
	{
		ComPtr<IXMLDOMElement> elem;
		p.As( &elem );
		parseTransform( mat, elem, lpKey );
	}

	std::wstring src = static_cast<LPCWSTR>( domGetString( element, lpKey ) );
	searchReplace( src, L"\r", L"" );
	searchReplace( src, L"\t", L" " );
	searchReplace( src, L"\n", L" " );
	searchReplace( src, L") ", L")\n" );
	searchReplace( src, L"),", L")\n" );
	searchReplace( src, L" ", L"" );

	std::vector<std::wstring> transforms;
	stringtok( transforms, src, true, L"\n" );
	float a, b, c, d, e, f, x, y, angle;

	for ( size_t i = 0; i < transforms.size(); i++ )
	{
		const auto& s = transforms[i];

		if ( 6 == swscanf_s( s.c_str(), L"matrix(%f,%f,%f,%f,%f,%f)", &a, &b, &c, &d, &e, &f ) )
		{
			Gdiplus::Matrix matTmp( a, b, c, d, e, f );
			mat.Multiply( &matTmp );
		}
		else if ( 2 == swscanf_s( s.c_str(), L"translate(%f,%f)", &x, &y ) )
		{
			mat.Translate( x, y );
		}
		else if ( 1 == swscanf_s( s.c_str(), L"translate(%f)", &x ) )
		{
			mat.Translate( x, 0.0f );
		}
		else if ( 3 == swscanf_s( s.c_str(), L"rotate(%f,%f,%f)", &angle, &x, &y ) )
		{
			mat.RotateAt( angle, Gdiplus::PointF( x, y ) );
		}
		else if ( 1 == swscanf_s( s.c_str(), L"rotate(%f)", &angle ) )
		{
			mat.Rotate( angle );
		}
		else if ( 2 == swscanf_s( s.c_str(), L"scale(%f,%f)", &x, &y ) )
		{
			mat.Scale( x, y );
		}
		else if ( 1 == swscanf_s( s.c_str(), L"scale(%f)", &x ) )
		{
			mat.Scale( x, x );
		}
		else if ( 1 == swscanf_s( s.c_str(), L"skewX(%f)", &x ) )
		{
			Gdiplus::Matrix matTmp( 1.0f, 0.0f, tanf( x ), 1.0f, 0.0f, 0.0f );
			mat.Multiply( &matTmp );
		}
		else if ( 1 == swscanf_s( s.c_str(), L"skewY(%f)", &y ) )
		{
			Gdiplus::Matrix matTmp( 1.0f, tanf( y ), 0.0f, 1.0f, 0.0f, 0.0f );
			mat.Multiply( &matTmp );
		}
	}

	return S_OK;
}

HRESULT parseTransform( SVGElement& ve, ComPtr<IXMLDOMElement> element )
{
	Gdiplus::Matrix mat;
	parseTransform( mat, element );
	ve.setTransform( mat );
	return S_OK;
}

std::unique_ptr<Gdiplus::Brush> parseStyleFill( ComPtr<IXMLDOMElement> /*element*/,
												std::map<std::wstring, std::wstring>& propDict )
{
	auto strColor = propDict[L"fill"];

	if ( strColor == L"none" || strColor.empty() )
	{
		return nullptr;
	}
	else if ( strColor[0] == '#' )
	{
		// color code
		auto		   lpOpacity = propDict[L"fill-opacity"].c_str();
		Gdiplus::Color color	 = parseHexColor( strColor.c_str() + 1, lpOpacity );
		return std::make_unique<Gdiplus::SolidBrush>( color );
	}
	else if ( strColor.find( L"url(" ) != std::wstring::npos )
	{
		if ( strColor.find( '#' ) != std::wstring::npos )
		{
			// external reference is not supported
			return std::make_unique<Gdiplus::SolidBrush>( Gdiplus::Color( 0, 0, 0 ) );
		}

		// Fallback to default color
		return std::make_unique<Gdiplus::SolidBrush>( Gdiplus::Color( 0, 0, 0 ) );
	}
	else
	{
		// try to lookup color table
		COLORREF refColor;
		int		 ret = lookupColorTable( strColor.c_str(), refColor );
		if ( ret == -1 )
		{
			// Fallback to default color
			return std::make_unique<Gdiplus::SolidBrush>( Gdiplus::Color( 0, 0, 0 ) );
		}
		else
		{
			auto lpOpacity = propDict[L"fill-opacity"].c_str();
			BYTE a		   = 255;
			if ( lpOpacity && *lpOpacity )
			{
				a = static_cast<BYTE>(
					std::min( 255, static_cast<int>( fmax( 0.0f, toFloat( lpOpacity, 1.0f ) ) * 255.0f ) ) );
			}

			Gdiplus::Color color =
				Gdiplus::Color( a, GetRValue( refColor ), GetGValue( refColor ), GetBValue( refColor ) );
			return std::make_unique<Gdiplus::SolidBrush>( color );
		}
	}
}

std::unique_ptr<Gdiplus::Pen> parseStyleStroke( std::map<std::wstring, std::wstring>& propDict )
{
	auto& strColor = propDict[L"stroke"];

	// color
	Gdiplus::Color color( 0, 0, 0 );
	if ( strColor == L"none" || strColor.empty() )
	{
		return nullptr;
	}
	else if ( strColor[0] == '#' )
	{
		// color code
		LPCTSTR lpOpacity = propDict[L"stroke-opacity"].c_str();
		color			  = parseHexColor( strColor.c_str() + 1, lpOpacity );
	}
	else if ( strColor.find( L"url(" ) != std::wstring::npos )
	{
		// Unsupported format(including gradation)
		return nullptr;
	}
	else
	{
		// try to lookup color table
		COLORREF refColor;
		int		 ret = lookupColorTable( strColor.c_str(), refColor );
		if ( ret == -1 )
		{
			return nullptr;
		}
		else
		{
			LPCTSTR lpOpacity = propDict[L"stroke-opacity"].c_str();
			BYTE	a		  = 255;
			if ( lpOpacity && *lpOpacity )
			{
				a = static_cast<BYTE>(
					std::min( 255, static_cast<int>( fmax( 0.0f, toFloat( lpOpacity, 1.0f ) ) * 255.0f ) ) );
			}

			color = Gdiplus::Color( a, GetRValue( refColor ), GetGValue( refColor ), GetBValue( refColor ) );
		}
	}

	// width
	auto& strWidth = propDict[L"stroke-width"];
	float width;
	if ( strWidth.empty() )
	{
		width = 1.0f;
	}
	else
	{
		width = parseDistance( strWidth.c_str() );
	}
	auto pStroke = std::make_unique<Gdiplus::Pen>( color, width );

	// dash
	{
		float dashOffset = toFloat( propDict[L"stroke-dashoffset"].c_str() );
		pStroke->SetDashOffset( dashOffset );

		std::vector<std::wstring> dashStrArray;
		auto					  str = propDict[L"stroke-dasharray"];
		searchReplace( str, L"\r", L"" );
		searchReplace( str, L"\n", L"" );
		stringtok( dashStrArray, str, true, L"," );
		if ( !dashStrArray.empty() )
		{
			std::vector<float> dashArray;
			for ( size_t i = 0; i < dashStrArray.size(); i++ )
			{
				dashArray.push_back( toFloat( dashStrArray[i] ) );
			}
			pStroke->SetDashPattern( &dashArray[0], static_cast<INT>( dashArray.size() ) );
		}
	}

	// miter limit
	float miterLimit = toFloat( propDict[L"stroke-miterlimit"].c_str(), 4.0f );
	pStroke->SetMiterLimit( miterLimit );
	return pStroke;
}

std::tuple<std::unique_ptr<Gdiplus::Brush>, std::unique_ptr<Gdiplus::Pen>> parseStyle(
	ComPtr<IXMLDOMElement> element, std::wstring& pFamily, Gdiplus::FontStyle* pStyle = nullptr,
	float* pFontSize = nullptr )
{
	std::map<std::wstring, std::wstring> propDict;

	recursiveMakePropDict( element, propDict );

	// fill
	auto pFill = parseStyleFill( element, propDict );
	// stroke
	auto pStroke = parseStyleStroke( propDict );

	// font settings[optional]
	pFamily = propDict[L"font-family"];
	if ( pStyle )
	{
		int style = Gdiplus::FontStyleRegular;
		if ( propDict[L"font-weight"] == L"bold" )
		{
			style |= Gdiplus::FontStyleBold;
		}
		if ( propDict[L"font-style"] == L"italic" )
		{
			style |= Gdiplus::FontStyleItalic;
		}

		*pStyle = static_cast<Gdiplus::FontStyle>( style );
	}
	if ( pFontSize )
	{
		*pFontSize = parseDistance( propDict[L"font-size"].c_str() );
	}

	return std::make_tuple( std::move( pFill ), std::move( pStroke ) );
}

HRESULT parseRect( SVG& image, ComPtr<IXMLDOMNode>& node )
{
	ComPtr<IXMLDOMElement> element;
	node.As( &element );
	float x = 0.0;
	float y = 0.0;
	try
	{
		x = domGetDistance( element, L"x" );
	}
	catch ( ... )
	{
		x = 0.0;
	}
	try
	{
		y = domGetDistance( element, L"y" );
	}
	catch ( ... )
	{
		y = 0.0;
	}
	float width	 = domGetDistance( element, L"width" );
	float height = domGetDistance( element, L"height" );
	float rx	 = 0.0;
	float ry	 = 0.0;
	try
	{
		rx = domGetDistance( element, L"rx" );
		ry = domGetDistance( element, L"ry" );
	}
	catch ( ... )
	{
		rx = 0.0;
		ry = 0.0;
	}
	std::wstring family;
	auto [pFill, pStroke] = parseStyle( element, family );

	SVGElementRect* pRect = image.addRect( x, y, width, height, rx, ry );
	pRect->setFill( std::move( pFill ) );
	pRect->setStroke( std::move( pStroke ) );
	parseTransform( *pRect, element );

	return S_OK;
}

HRESULT parseCircle( SVG& image, ComPtr<IXMLDOMNode>& node )
{
	ComPtr<IXMLDOMElement> element;
	node.As( &element );
	float		 x = domGetDistance( element, L"cx" );
	float		 y = domGetDistance( element, L"cy" );
	float		 r = domGetDistance( element, L"r" );

	std::wstring family;
	auto [pFill, pStroke]	  = parseStyle( element, family );

	SVGElementCircle* pCircle = image.addCircle( x, y, r );
	pCircle->setFill( std::move( pFill ) );
	pCircle->setStroke( std::move( pStroke ) );
	parseTransform( *pCircle, element );

	return S_OK;
}

HRESULT parseLine( SVG& image, ComPtr<IXMLDOMNode>& node )
{
	ComPtr<IXMLDOMElement> element;
	node.As( &element );
	float		 x1 = domGetDistance( element, L"x1" );
	float		 y1 = domGetDistance( element, L"y1" );
	float		 x2 = domGetDistance( element, L"x2" );
	float		 y2 = domGetDistance( element, L"y2" );

	std::wstring family;
	auto [pFill, pStroke] = parseStyle( element, family );

	SVGElementPath* pPath = image.addPath();
	pPath->moveTo( x1, y1, true );
	pPath->lineTo( x2, y2, true );

	pPath->setFill( std::move( pFill ) );
	pPath->setStroke( std::move( pStroke ) );
	parseTransform( *pPath, element );

	return S_OK;
}

HRESULT parsePolyLine( SVG& image, ComPtr<IXMLDOMNode>& node )
{
	ComPtr<IXMLDOMElement> element;
	node.As( &element );
	std::wstring strPath = static_cast<LPCWSTR>( domGetString( element, L"points" ) );
	searchReplace( strPath, L"\r", L"" );
	searchReplace( strPath, L"\n", L"" );
	std::vector<std::wstring> pathData;
	stringtok( pathData, strPath, true, L" ," );

	if ( !pathData.empty() )
	{
		SVGElementPath* pPath = image.addPath();

		for ( size_t i = 0; i < pathData.size(); i += 2 )
		{
			if ( i == 0 )
			{
				pPath->moveTo( parseDistance( pathData[i] ), parseDistance( pathData[i + 1] ), true );
			}
			else
			{
				pPath->lineTo( parseDistance( pathData[i] ), parseDistance( pathData[i + 1] ), true );
			}
		}

		std::wstring family;
		auto [pFill, pStroke] = parseStyle( element, family );

		pPath->setFill( std::move( pFill ) );
		pPath->setStroke( std::move( pStroke ) );
		parseTransform( *pPath, element );
	}

	return S_OK;
}

HRESULT parsePolygon( SVG& image, ComPtr<IXMLDOMNode>& node )
{
	ComPtr<IXMLDOMElement> element;
	node.As( &element );
	std::wstring			  strPath = static_cast<LPCWSTR>( domGetString( element, L"points" ) );
	std::vector<std::wstring> pathData;
	searchReplace( strPath, L"\r", L"" );
	searchReplace( strPath, L"\n", L"" );
	stringtok( pathData, strPath, true, L" ," );

	if ( !pathData.empty() )
	{
		SVGElementPath* pPath = image.addPath();

		for ( size_t i = 0; i < pathData.size(); i += 2 )
		{
			if ( i == 0 )
			{
				pPath->moveTo( parseDistance( pathData[i] ), parseDistance( pathData[i + 1] ), true );
			}
			else
			{
				pPath->lineTo( parseDistance( pathData[i] ), parseDistance( pathData[i + 1] ), true );
			}
		}
		pPath->closePath();

		std::wstring family;
		auto [pFill, pStroke] = parseStyle( element, family );

		pPath->setFill( std::move( pFill ) );
		pPath->setStroke( std::move( pStroke ) );
		parseTransform( *pPath, element );
	}

	return S_OK;
}

HRESULT parseText( SVG& image, ComPtr<IXMLDOMNode>& node, float* pLastLeft = nullptr, float* pLastTop = nullptr )
{
	// texts are stored directly, or contained in some <tspan>s.
	float tmpX = 0.0f, tmpY = 0.0f;
	if ( !pLastLeft )
	{
		pLastLeft = &tmpX;
		pLastTop  = &tmpY;
	}

	ComPtr<IXMLDOMElement> parent;
	node.As( &parent );

	ComPtr<IXMLDOMNodeList> resultList;
	if ( SUCCEEDED( parent->get_childNodes( &resultList ) ) && resultList )
	{
		// updating text coordinate
		{
			float x = 0.0f;
			float y = 0.0f;
			try
			{
				x = domGetDistance( parent, L"x", *pLastLeft ) + domGetDistance( parent, L"dx" );
			}
			catch ( ... )
			{
				x = 0.0f;
			}
			try
			{
				y = domGetDistance( parent, L"y", *pLastTop ) + domGetDistance( parent, L"dy" );
			}
			catch ( ... )
			{
				y = 0.0f;
			}
			*pLastLeft = x;
			*pLastTop  = y;
		}

		while ( true )
		{
			ComPtr<IXMLDOMNode> child;
			if ( FAILED( resultList->nextNode( &child ) ) || !child )
				break;

			DOMNodeType type;
			if ( SUCCEEDED( child->get_nodeType( &type ) ) )
			{
				if ( type == NODE_TEXT )
				{
					// text found
					_variant_t var;
					if ( SUCCEEDED( child->get_nodeValue( &var ) ) )
					{
						var.ChangeType( VT_BSTR );
						std::wstring strText;
						strText = var.bstrVal;

						Gdiplus::FontStyle style;
						float			   fontSize;
						// Gdiplus::FontFamily::GenericSansSerif();
						std::wstring strFontFamily;
						auto [pFill, pStroke]			  = parseStyle( parent, strFontFamily, &style, &fontSize );

						Gdiplus::FontFamily* lpFontFamily = nullptr;
						if ( !strFontFamily.empty() )
						{
							lpFontFamily = new Gdiplus::FontFamily( strFontFamily.c_str() );
							if ( !lpFontFamily->IsAvailable() )
							{
								// font not found.
								delete lpFontFamily;
								lpFontFamily = nullptr;
							}
						}
						// if lpFontFamily==NULL, use default font
						const Gdiplus::FontFamily* lpFontToUse =
							( lpFontFamily ? lpFontFamily : Gdiplus::FontFamily::GenericSansSerif() );

						// add text as a path
						SVGElementPath* pPath = image.addPath();
						float			dy =
							( lpFontToUse->GetCellAscent( style ) ) * ( fontSize / lpFontToUse->GetEmHeight( style ) );

						pPath->addString( strText.c_str(), lpFontToUse, style, fontSize, *pLastLeft, *pLastTop - dy );

						// get text width to update the coordinate of the next text.
						Gdiplus::RectF rect;
						pPath->getBounds( rect );
						*pLastLeft += rect.Width;

						delete lpFontFamily;
						lpFontFamily = nullptr;

						pPath->setFill( std::move( pFill ) );
						pPath->setStroke( std::move( pStroke ) );
						parseTransform( *pPath, parent );
					}
				}
				else
				{
					// recursive processing
					parseText( image, child, pLastLeft, pLastTop );
				}
			}
		}
	}

	return S_OK;
}

HRESULT parsePath( SVG& image, ComPtr<IXMLDOMNode> node )
{
	ComPtr<IXMLDOMElement> element;
	node.As( &element );
	std::wstring strPath = static_cast<LPCWSTR>( domGetString( element, L"d" ) );
	searchReplace( strPath, L"\r", L"" );
	searchReplace( strPath, L"\n", L"" );

	// some coordinates are written without any space like "100-200" which means "100,-200"
	searchReplace( strPath, L"-", L" -" );

	// separate command and coordinates
	LPCWSTR lpCommands = L"MmLlHhVvCcSsZzQqTtAa";
	for ( LPCWSTR p = lpCommands; *p; p++ )
	{
		std::wstring oldValue = std::wstring( 1, *p );
		std::wstring newValue = std::wstring( 1, *p );
		newValue			  = L' ' + newValue + L' ';
		searchReplace( strPath, oldValue, newValue );
	}

	std::vector<std::wstring> pathData;
	stringtok( pathData, strPath, true, L" ," );
	if ( !pathData.empty() )
	{
		SVGElementPath* pPath = image.addPath();

		TCHAR			cmd	  = '\0';
		for ( size_t i = 0; i < pathData.size(); )
		{
			TCHAR head = pathData[i][0];
			for ( LPCTSTR p = lpCommands; *p; p++ )
			{
				if ( head == *p )
				{
					cmd = head;
					i++;
					break;
				}
			}

			switch ( cmd )
			{
				case L'M':	// move to
				case L'm':
					if ( !pPath->isClosed() )
					{
						pPath->startPath();
					}
					pPath->moveTo( parseDistance( pathData[i] ), parseDistance( pathData[i + 1] ), cmd == L'M' );
					i += 2;
					cmd = ( cmd == 'M' ) ? 'L' : 'l';
					break;
				case L'L':	// line to
				case L'l':
					pPath->lineTo( parseDistance( pathData[i] ), parseDistance( pathData[i + 1] ), cmd == L'L' );
					i += 2;
					break;
				case L'H':	// horizontal line to
				case L'h':
					pPath->horizontalLineTo( parseDistance( pathData[i] ), cmd == L'H' );
					i += 1;
					break;
				case L'V':	// vertical line to
				case L'v':
					pPath->verticalLineTo( parseDistance( pathData[i] ), cmd == L'V' );
					i += 1;
					break;
				case L'C':	// cubic bezier curve to
				case L'c':
					pPath->bezier( parseDistance( pathData[i] ), parseDistance( pathData[i + 1] ),
								   parseDistance( pathData[i + 2] ), parseDistance( pathData[i + 3] ),
								   parseDistance( pathData[i + 4] ), parseDistance( pathData[i + 5] ), cmd == L'C' );
					i += 6;
					break;
				case L'S':	// smooth cubic bezier curve to
				case L's':
					pPath->smoothCubicBezier( parseDistance( pathData[i] ), parseDistance( pathData[i + 1] ),
											  parseDistance( pathData[i + 2] ), parseDistance( pathData[i + 3] ),
											  cmd == L'S' );
					i += 4;
					break;
				case L'Z':	// close path
				case L'z':
				{
					pPath->closePath();
					// std::wstring family;
					// auto [pFill, pStroke] = parseStyle(element, family);

					// pPath->setFill(std::move(pFill));
					// pPath->setStroke(std::move(pStroke));
					// parseTransform(*pPath, element);
					// pPath = image.addPath();
					// pPath->startPath();
				}
				break;
				case L'Q':	// quadratic bezier
				case L'q':
					pPath->quadraticBezier( parseDistance( pathData[i] ), parseDistance( pathData[i + 1] ),
											parseDistance( pathData[i + 2] ), parseDistance( pathData[i + 3] ),
											cmd == L'Q' );
					i += 4;
					break;
				case L'T':	// smooth quadratic bezier
				case L't':
					pPath->smoothQuadraticBezier( parseDistance( pathData[i] ), parseDistance( pathData[i + 1] ),
												  cmd == L'T' );
					i += 2;
					break;
				case L'A':	// arcto
				case L'a':
					pPath->arcTo( parseDistance( pathData[i] ), parseDistance( pathData[i + 1] ),
								  toFloat( pathData[i + 2] ), toInt( pathData[i + 3] ), toInt( pathData[i + 4] ),
								  parseDistance( pathData[i + 5] ), parseDistance( pathData[i + 6] ), cmd == L'A' );
					i += 7;
					break;
			}
		}

		std::wstring family;
		auto [pFill, pStroke] = parseStyle( element, family );

		pPath->setFill( std::move( pFill ) );
		pPath->setStroke( std::move( pStroke ) );
		parseTransform( *pPath, element );
	}

	return S_OK;
}

HRESULT recursiveBuildVectorImage( SVG& image, ComPtr<IXMLDOMNode> node );

HRESULT parseGroup( SVG& image, ComPtr<IXMLDOMNode> element )
{
	std::map<std::wstring, std::wstring> propDict;
	ComPtr<IXMLDOMElement>				 elem;
	element.As( &elem );
	recursiveMakePropDict( elem, propDict );

	SVGElementGroup* pGroup = image.addGroup();

	// opacity : this opacity works on rendered result of group children. This opacity is not inherited to children.
	LPCTSTR lpOpacity = propDict[L"opacity"].c_str();
	float	a		  = fminf( 1.0f, ( fmaxf( 0.0f, toFloat( lpOpacity, 1.0f ) ) ) );
	pGroup->setOpacity( a );

	recursiveBuildVectorImage( pGroup->getInternalSVG(), element );

	return S_OK;
}

HRESULT recursiveBuildVectorImage( SVG& image, ComPtr<IXMLDOMNode> node )
{
	ComPtr<IXMLDOMNodeList> children;
	node->get_childNodes( &children );

	while ( children )
	{
		ComPtr<IXMLDOMNode> child;
		children->nextNode( &child );
		if ( !child )
			break;
		// check if this element is visible
		DOMNodeType type;
		if ( SUCCEEDED( child->get_nodeType( &type ) ) )
		{
			if ( type == NODE_ELEMENT )
			{
				ComPtr<IXMLDOMElement> elem;
				child.As( &elem );
				std::wstring strDisplay = static_cast<LPCWSTR>( domGetString( elem, L"display" ) );
				if ( strDisplay == L"none" )
					continue;
			}
		}

		_bstr_t nodeName;
		child->get_nodeName( nodeName.GetAddress() );
		if ( wcscmp( nodeName, L"defs" ) == 0 )
		{
			// do not recursive process
			continue;
		}
		else if ( wcscmp( nodeName, L"g" ) == 0 )
		{
			parseGroup( image, child );
			// recursive parsing on different image
			continue;
		}
		else if ( wcscmp( nodeName, L"rect" ) == 0 )
		{
			parseRect( image, child );
		}
		else if ( wcscmp( nodeName, L"circle" ) == 0 )
		{
			parseCircle( image, child );
		}
		else if ( wcscmp( nodeName, L"line" ) == 0 )
		{
			parseLine( image, child );
		}
		else if ( wcscmp( nodeName, L"polyline" ) == 0 )
		{
			parsePolyLine( image, child );
		}
		else if ( wcscmp( nodeName, L"polygon" ) == 0 )
		{
			parsePolygon( image, child );
		}
		else if ( wcscmp( nodeName, L"text" ) == 0 )
		{
			parseText( image, child );
		}
		else if ( wcscmp( nodeName, L"path" ) == 0 )
		{
			parsePath( image, child );
		}
		recursiveBuildVectorImage( image, child );
	}

	return S_OK;
}
