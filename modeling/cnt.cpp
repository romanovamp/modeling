#include "stdafx.h"
#include "cnt.h"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace bg = boost::geometry;
typedef bg::model::d2::point_xy<double> point_type;
typedef bg::model::polygon<point_type> polygon_type;

cnt::cnt()
{
}

cnt::cnt(double _k, double _a, point_type _stratPoint, polygon_type _poly)
{
	k = _k;
	a = _a;
	stratPoint = _stratPoint;
	poly = _poly;
	idParent = 0;
}
cnt::cnt(double _k, double _a, point_type _stratPoint, polygon_type _poly, int _idParent)
{
	k = _k;
	a = _a;
	stratPoint = _stratPoint;
	poly = _poly;
	idParent = _idParent;
}

cnt::~cnt()
{
}
