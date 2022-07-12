#pragma once
#include <memory>
#include <string>
#include <vector>
#pragma warning( push )
#pragma warning( disable : 4458 )  // declaration of 'xxx' hides class member
#include <gdiplus.h>
#pragma warning( pop )

class SVGElementGroup;
class SVGElementPath;
class SVGElementCircle;
class SVGElementRect;
class SVGElement;

struct SVGPoint
{
	float x = 0.0f;
	float y = 0.0f;
};

struct SVGPolygon
{
	std::vector<SVGPoint> points;
};

class SVG
{
public:
	void clear();
	bool parseFile( const std::wstring& path );
	void render( Gdiplus::Graphics& graphics ) const;
	void render( Gdiplus::Graphics& graphics, float left, float top, float scaleX = 1.0f, float scaleY = 1.0f ) const;

	std::vector<SVGPolygon> getPolygons() const;

	void					setCanvasSize( float width, float height );
	float					getCanvasWidth() const;
	float					getCanvasHeight() const;

	SVGElementRect*			addRect( float x, float y, float width, float height, float rx, float ry );
	SVGElementCircle*		addCircle( float x, float y, float r );
	SVGElementPath*			addPath();
	SVGElementGroup*		addGroup();

private:
	std::vector<std::unique_ptr<SVGElement>> m_elements;
	float									 m_canvasWidth	= 0;
	float									 m_canvasHeight = 0;
};

class SVGElement
{
public:
	SVGElement()		  = default;
	virtual ~SVGElement() = default;
	void							setFill( std::unique_ptr<Gdiplus::Brush>&& brush );
	void							setStroke( std::unique_ptr<Gdiplus::Pen>&& stroke );
	void							pushRotation( float angle );
	void							pushRotation( float angle, float cx, float cy );
	void							pushScale( float sx, float sy );
	void							pushTranslation( float tx, float ty );
	void							pushSkewX( float angle );
	void							pushSkewY( float angle );
	void							pushMatrix( float a, float b, float c, float d, float e, float f );
	void							setTransform( const Gdiplus::Matrix& mat );
	virtual void					render( Gdiplus::Graphics&, float canvasX, float canvasY ) = 0;
	virtual std::vector<SVGPolygon> getPolygons() const										   = 0;

protected:
	std::unique_ptr<Gdiplus::Brush> m_fill;
	std::unique_ptr<Gdiplus::Pen>	m_stroke;

	Gdiplus::Matrix					m_matrix;
};

class SVGElementRect : public SVGElement
{
public:
	SVGElementRect()		  = default;
	virtual ~SVGElementRect() = default;
	void					setParams( float x, float y, float width, float height, float rx, float ry );
	void					render( Gdiplus::Graphics& graphics, float canvasX, float canvasY ) override;
	std::vector<SVGPolygon> getPolygons() const override;

protected:
	float m_x	   = 0.0;
	float m_y	   = 0.0;
	float m_width  = 0.0;
	float m_height = 0.0;
	float m_rx	   = 0.0;
	float m_ry	   = 0.0;
};

class SVGElementCircle : public SVGElement
{
public:
	SVGElementCircle()			= default;
	virtual ~SVGElementCircle() = default;
	void					setParams( float x, float y, float r );
	void					render( Gdiplus::Graphics& graphics, float canvasX, float canvasY ) override;
	std::vector<SVGPolygon> getPolygons() const override;

protected:
	float m_x = 0.0;
	float m_y = 0.0;
	float m_r = 0.0;
};

class SVGElementPath : public SVGElement
{
public:
	SVGElementPath();
	virtual ~SVGElementPath() = default;
	void addString( LPCWSTR lpText, const Gdiplus::FontFamily* lpFontFamily, Gdiplus::FontStyle style, float fontSize,
					float x, float y );
	void lineTo( float x, float y, bool bAbsolute = false );
	void horizontalLineTo( float x, bool bAbsolute = false );
	void verticalLineTo( float y, bool bAbsolute = false );
	void moveTo( float x, float y, bool bAbsolute = false );
	void bezier( float x1, float y1, float x2, float y2, float endX, float endY, bool bAbsolute = false );
	void smoothCubicBezier( float x2, float y2, float endX, float endY, bool bAbsolute = false );
	void quadraticBezier( float x1, float y1, float endX, float endY, bool bAbsolute = false );
	void smoothQuadraticBezier( float endX, float endY, bool bAbsolute = false );
	void arcTo( float rx, float ry, float xAxisRotation, int largeARCFlag, int sweepFlag, float x2, float y2,
				bool bAbsolute = false );
	void startPath();
	void closePath();
	bool isClosed() const;
	const Gdiplus::PointF&	getLastPos() const;
	void					getBounds( Gdiplus::RectF& bounds ) const;
	void					render( Gdiplus::Graphics& graphics, float canvasX, float canvasY ) override;
	std::vector<SVGPolygon> getPolygons() const override;

protected:
	Gdiplus::GraphicsPath m_path{};
	Gdiplus::PointF		  m_lastPos{};
	Gdiplus::PointF		  m_lastC2{};  // last position of the bezier control point
	Gdiplus::PointF		  m_firstPos{};
	bool				  m_bFirst = true;

protected:
	static float getAngle( const Gdiplus::PointF& a, const Gdiplus::PointF& b );
};

class SVGElementGroup : public SVGElement
{
public:
	SVGElementGroup()		   = default;
	virtual ~SVGElementGroup() = default;
	void					setOpacity( float opacity );
	void					render( Gdiplus::Graphics& parentGraphics, float cX, float cY ) override;
	std::vector<SVGPolygon> getPolygons() const override;
	SVG&					getInternalSVG();

protected:
	float m_opacity;
	SVG	  m_svg;
};
