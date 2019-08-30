#pragma once
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

typedef boost::geometry::model::d2::point_xy<double> point_type;
typedef boost::geometry::model::polygon<point_type> polygon_type;

class cnt
{
public:
	double a, k;
	point_type stratPoint;
	polygon_type poly;
	cnt();
	cnt(double k, double a, point_type stratPoint, polygon_type poly);
	~cnt();
};

