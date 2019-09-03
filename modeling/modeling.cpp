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
#include <map>
#include <Windows.h>
#include <boost/assign/std/vector.hpp>
#include <boost/progress.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/io/svg/svg_mapper.hpp>

using namespace std;

#define pi 3.14159265358979323846
#define Eps 0.000001

int L, n, id;
double mean, devi, second = 0.0, radius = 1, mF;
bool ready = false;
MtRng64 mt;
map <int, int> temp;
map <int, int>::iterator it;
vector <cnt> infoCnt(0);
vector <cnt> tempCnt(0);
ofstream fileLog;

namespace bg = boost::geometry;
typedef bg::model::d2::point_xy<double> point_type;
typedef bg::model::polygon<point_type> polygon_type;
typedef boost::geometry::model::linestring<point_type> linestring_type;

linestring_type lineLeft;
linestring_type lineRight;
linestring_type lineTop;
linestring_type lineBottom;

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

polygon_type createPolygon(point_type startPoint, double k, double a)
{
	polygon_type poly;
	
	point_type point1(x(startPoint.x(), radius, a + 90), y(startPoint.y(), radius, a + 90));
	point_type point2(x(point1.x(), k, a), y(point1.y(), k, a));

	point_type point4(x(startPoint.x(), radius, a - 90), y(startPoint.y(), radius, a - 90));
	point_type point3(x(point4.x(), k, a), y(point4.y(), k, a));

	poly.outer().push_back(point1);
	poly.outer().push_back(point2);
	poly.outer().push_back(point3);
	poly.outer().push_back(point4);

	return poly;
}

bool check(polygon_type newPoly, point_type startPoint, double k)
{
	double distanse = 0;
	for (int j = 0; j < infoCnt.size(); j++)
	{
		if (infoCnt[j].k + k + 2 * radius > bg::distance(startPoint, infoCnt[j].stratPoint))
		{
			distanse = bg::distance(newPoly, infoCnt[j].poly);
			if (distanse < Eps*radius) return false;
			else if (distanse < mF) temp.insert(pair<int, int>(id, j));
		}
	}
	return true;
}

bool createTrans(double x, double y, double k, double a, int idParent)
{
	point_type newPoint(x, y);
	polygon_type poly = createPolygon(newPoint, k, a);
	id++;
	if (check(poly, newPoint, k)) tempCnt.push_back(cnt(k, a, newPoint, poly, idParent));
	else return false;
	return true;
}

bool transCheck(polygon_type newPoly, point_type startPoint, double k, double a)
{
	int idParent = id;
	if (bg::distance(newPoly, lineLeft) < Eps && bg::distance(newPoly, lineBottom) < Eps)
		if (!createTrans(startPoint.x() + L, startPoint.y() + L, k, a, idParent)) return false;

	if (bg::distance(newPoly, lineLeft) < Eps && bg::distance(newPoly, lineTop) < Eps)
		if (!createTrans(startPoint.x() + L, startPoint.y() - L, k, a, idParent)) return false;

	if (bg::distance(newPoly, lineRight) < Eps && bg::distance(newPoly, lineTop) < Eps)
		if (!createTrans(startPoint.x() - L, startPoint.y() - L, k, a, idParent)) return false;

	if (bg::distance(newPoly, lineRight) < Eps && bg::distance(newPoly, lineBottom) < Eps)
		if (!createTrans(startPoint.x() - L, startPoint.y() + L, k, a, idParent)) return false;

	if (bg::distance(newPoly, lineLeft) < Eps)
		if (!createTrans(startPoint.x() + L, startPoint.y(), k, a, idParent)) return false;

	if (bg::distance(newPoly, lineTop) < Eps)
		if (!createTrans(startPoint.x(), startPoint.y() - L, k, a, idParent)) return false;

	if (bg::distance(newPoly, lineRight) < Eps)
		if (!createTrans(startPoint.x() - L, startPoint.y(), k, a, idParent)) return false;

	if (bg::distance(newPoly, lineBottom) < Eps)
		if (!createTrans(startPoint.x(), startPoint.y() + L, k, a, idParent)) return false;

	return true;
}

void pack()
{
	double k, a;
	boost::progress_display progress(n*1.25);
	for (int i = 0; i < n; i++)
	{
		temp.clear();
		tempCnt.clear();
		k = bm();
		a = mt.getReal1() * 360.0;
		point_type point(mt.getReal1()*L, mt.getReal1()*L);
		polygon_type poly = createPolygon(point, k, a);

		id = infoCnt.size();

		if (check(poly, point, k) && transCheck(poly, point, k, a))
		{
			infoCnt.push_back(cnt(k, a, point, poly));
			infoCnt.insert(infoCnt.end(), tempCnt.begin(), tempCnt.end());

			for (it = temp.begin(); it != temp.end(); ++it)
			{
				infoCnt[it->first].nearCnt.push_back(it->second);
				infoCnt[it->second].nearCnt.push_back(it->first);
			}
		}
		else i--;
		++progress;
	}
}

string strMainPolygon()
{
	lineLeft.push_back(point_type(0, -L));
	lineLeft.push_back(point_type(0, 2*L));

	lineRight.push_back(point_type(L, -L));
	lineRight.push_back(point_type(L, 2*L));

	lineTop.push_back(point_type(-L, L));
	lineTop.push_back(point_type(2*L, L));

	lineBottom.push_back(point_type(-L, 0));
	lineBottom.push_back(point_type(2*L, 0));

	return "POLYGON((0 0,0 " + toStr(L) + "," + toStr(L) + " " + toStr(L) + "," + toStr(L) + " 0))";
}

int main()
{
	setlocale(LC_ALL, "rus");
	CreateDirectoryW(L"files", NULL);
	fileLog.open("./files/log.txt");

	double p;
	cout << "L ";
	cin >> L;
	cout << "mean ";
	cin >> mean;
	cout << "p ";
	cin >> p;
	cout << "mF ";
	cin >> mF;
	devi = mean * 0.1;
	n = (int) (p * L * L / (mean * 2 * radius)); //количество к-меров 
	cout << "Количество: " << n << " p = " << p << endl;

	polygon_type poly;
	bg::read_wkt(strMainPolygon(), poly);


	ofstream svg("my_map.svg");
	bg::svg_mapper<point_type> mapper(svg, 400, 400, "viewBox = \"200 -30 500 500\"");

	mapper.add(poly);
	mapper.map(poly, "fill-opacity:0.1;fill:rgb(51,51,153);stroke:rgb(51,51,153);stroke-width:1");

	pack();

	for (int i = 0; i < infoCnt.size(); i++)
	{
		mapper.add(infoCnt[i].poly);
		if(infoCnt[i].idParent == 0) mapper.map(infoCnt[i].poly, "fill-opacity:0.1;fill:rgb(51,51,153);stroke:rgb(51,51,153);stroke-width:1");
		else mapper.map(infoCnt[i].poly, "fill-opacity:0.1;fill:rgb(255,0,0);stroke:rgb(255,0,0);stroke-width:1");
	}
	for (int i = 0; i < infoCnt.size(); i++)
	{
		fileLog << i << " ";
		for (int j = 0; j < infoCnt[i].nearCnt.size(); j++)
			fileLog << infoCnt[i].nearCnt[j] << " ";
		fileLog << "  idParent: " << infoCnt[i].idParent << " ";
		fileLog << endl;
	}
	temp.clear();
	tempCnt.clear();
	infoCnt.clear();
    return 0;
}

