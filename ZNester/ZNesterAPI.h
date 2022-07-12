#pragma once

#ifdef ZNESTER_EXPORT
#define ZNESTER_API __declspec( dllexport )
#else
#define ZNESTER_API __declspec( dllimport )
#endif

namespace ZNester
{

	struct ZNesterPoint
	{
		float x = 0.0f;
		float y = 0.0f;
	};
	struct ZNesterPolygon
	{
		ZNesterPoint* points	 = nullptr;
		int			  pointCount = 0;
		__int64		  id		 = 0;
	};

	void ZNESTER_API run( ZNesterPolygon target, ZNesterPolygon* objects, int objectCount );

}  // namespace ZNester