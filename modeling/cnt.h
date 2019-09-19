#pragma once
#include <vector>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

typedef boost::geometry::model::d2::point_xy<double> point_type;
typedef boost::geometry::model::polygon<point_type> polygon_type;

using std::vector;

class cnt
{
public:
	double a, k;
	int idParent, idClus;
	point_type stratPoint;
	polygon_type poly;
	vector <int> nearCnt;
	vector <int> colour;
	vector <int> idChild;
	cnt();
	cnt(double k, double a, point_type stratPoint, polygon_type poly);
	cnt(double k, double a, point_type stratPoint, polygon_type poly, int idParent);
	~cnt();
};

