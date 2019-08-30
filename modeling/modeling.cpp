// modeling.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "mersennetwister.h"
#include "cnt.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <vector>
#include <Windows.h>
#include <boost/assign/std/vector.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/io/svg/svg_mapper.hpp>

using namespace std;

#define pi 3.14159265358979323846
int L;
double mean, devi, second = 0.0, radius = 1;
bool ready = false;
MtRng64 mt;

namespace bg = boost::geometry;
typedef bg::model::d2::point_xy<double> point_type;
typedef bg::model::polygon<point_type> polygon_type;
vector <cnt> infoCnt(0);

double bm()
{
	if (ready)
	{
		ready = false;
		return (second * devi + mean);
	}
	double s = 0, u = 0, v = 0;
	do
	{
		u = 2.0 * mt.getReal1() - 1.0;
		v = 2.0 * mt.getReal1() - 1.0;
		s = u * u + v * v;
	} while (s > 1.0 || s == 0.0);

	double r = sqrt(-2.0 * log(s) / s);
	second = r * u;
	ready = true;
	return (r * v * devi + mean);
}

string toStr(int number)
{
	stringstream ss;
	ss << number;
	return ss.str();
}

double x(double x, double k, double a)
{
	return x + k * cos(a);
}
double y(double y, double k, double a)
{
	return y + k * sin(a);
}

void pack()
{
	double k, a;
	for (int i = 0; i < 1000; i++)
	{
		polygon_type poly;
		k = bm();
		a = mt.getReal1() * 360.0;
		point_type point(mt.getReal1()*L, mt.getReal1()*L);

		point_type point1(x(point.x(), radius, a + 90), y(point.y(), radius, a + 90));
		point_type point2(x(point1.x(), k, a), y(point1.y(), k, a));

		point_type point4(x(point.x(), radius, a - 90), y(point.y(), radius, a - 90));
		point_type point3(x(point4.x(), k, a), y(point4.y(), k, a));

		poly.outer().push_back(point1);
		poly.outer().push_back(point2);
		poly.outer().push_back(point3);
		poly.outer().push_back(point4);

		infoCnt.push_back(cnt(k, a, point, poly));
	}
}

string strMainPolygon()
{
	return "POLYGON((0 0,0 " + toStr(L) + "," + toStr(L) + " " + toStr(L) + "," + toStr(L) + " 0))";
}

int main()
{
	cin >> L;
	cin >> mean;
	devi = mean * 0.1;

	polygon_type poly;
	bg::read_wkt(strMainPolygon(), poly);

	ofstream svg("my_map.svg");
	bg::svg_mapper<point_type> mapper(svg, 500, 500);

	mapper.add(poly);
	mapper.map(poly, "fill-opacity:0.1;fill:rgb(51,51,153);stroke:rgb(51,51,153);stroke-width:1");

	pack();

	for (int i = 0; i < infoCnt.size(); i++)
	{
		mapper.add(infoCnt[i].poly);
		mapper.map(infoCnt[i].poly, "fill-opacity:0.1;fill:rgb(51,51,153);stroke:rgb(51,51,153);stroke-width:1");
	}
	infoCnt.clear();
    return 0;
}

