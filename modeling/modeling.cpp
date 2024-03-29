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
vector<vector<int>> temp;
vector <cnt> infoCnt(0);
vector <cnt> tempCnt(0);
ofstream fileLog;
bool *visited;

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
		if (infoCnt[j].k + k + 2 * radius >= bg::distance(startPoint, infoCnt[j].stratPoint))
		{
			distanse = bg::distance(newPoly, infoCnt[j].poly);
			if (distanse < Eps*radius) return false;
			if (distanse < 2.0*mF)
			{
				temp.push_back(vector<int>(0));
				temp[temp.size() - 1].push_back(id);
				temp[temp.size() - 1].push_back(j);
			}
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
	double Left = bg::distance(newPoly, lineLeft);
	double Right = bg::distance(newPoly, lineRight);
	double Top = bg::distance(newPoly, lineTop);
	double Bottom = bg::distance(newPoly, lineBottom);

	if (Left < Eps && Bottom < Eps)
		if (!createTrans(startPoint.x() + L, startPoint.y() + L, k, a, idParent)) return false;

	if (Left < Eps && Top < Eps)
		if (!createTrans(startPoint.x() + L, startPoint.y() - L, k, a, idParent)) return false;

	if (Right < Eps && Top < Eps)
		if (!createTrans(startPoint.x() - L, startPoint.y() - L, k, a, idParent)) return false;

	if (Right < Eps && Bottom < Eps)
		if (!createTrans(startPoint.x() - L, startPoint.y() + L, k, a, idParent)) return false;

	if (Left < Eps)
		if (!createTrans(startPoint.x() + L, startPoint.y(), k, a, idParent)) return false;

	if (Top < Eps)
		if (!createTrans(startPoint.x(), startPoint.y() - L, k, a, idParent)) return false;

	if (Right < Eps)
		if (!createTrans(startPoint.x() - L, startPoint.y(), k, a, idParent)) return false;

	if (Bottom < Eps)
		if (!createTrans(startPoint.x(), startPoint.y() + L, k, a, idParent)) return false;

	return true;
}

void pack()
{
	double k, a;
	boost::progress_display progress(n);
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
			if (tempCnt.size() != 0)
			{
				int idCh = infoCnt.size();
				for (int it = 0; it < tempCnt.size(); it++, idCh++)
					infoCnt[infoCnt.size() - 1].idChild.push_back(idCh);

				infoCnt.insert(infoCnt.end(), tempCnt.begin(), tempCnt.end());
			}

			for (int j=0; j<temp.size(); j++)
			{
				infoCnt[temp[j][0]].nearCnt.push_back(temp[j][1]);
				infoCnt[temp[j][1]].nearCnt.push_back(temp[j][0]);
			}
			++progress;
		}
		else i--;
	}
}

bool p0(polygon_type poly, int axis)
{
	bool f = false;
	switch (axis)
	{
	case 0:
	{
		if (bg::distance(poly, lineBottom) < mF) f = true;
		break;
	}
	case 1:
	{
		if (bg::distance(poly, lineLeft) < mF) f = true;
		break;
	}
	default:
		break;
	}
	return f;
}

bool pL(polygon_type poly, int axis)
{
	bool f = false;
	switch (axis)
	{
	case 0:
		{
		if (bg::distance(poly, lineTop) < mF) f = true;
		break;
		}
	case 1:
		{
		if (bg::distance(poly, lineRight) < mF) f = true;
		break;
		}
	default:
		break;
	}
	return f;
}

bool parent(int i, int j)
{
	//надо проверить, что i не родитель j и j не родитель i
	if (i == infoCnt[j].idParent || j == infoCnt[i].idParent) return true;
	return false;
}

void DFS(int st, int clusters, vector <int> colour) // кластеризация
{
	if (visited[st]) return;
	visited[st] = true;
	infoCnt[st].idClus = clusters;
	infoCnt[st].colour.insert(infoCnt[st].colour.end(), colour.begin(), colour.end());

	if (infoCnt[st].idParent != -1)
		DFS(infoCnt[st].idParent, clusters, colour);

	if (infoCnt[st].idChild.size() != 0)
		for(int it=0; it<infoCnt[st].idChild.size(); it++)
			DFS(infoCnt[st].idChild[it], clusters, colour);

	for (int r = 0; r < infoCnt[st].nearCnt.size(); r++)
			DFS(infoCnt[st].nearCnt[r], clusters, colour);
}

void DFS(int st, bool &pc, int axis) //x = true - по оси x,  = false - по оси y
{
	visited[st] = true;
	if (pL(infoCnt[st].poly, axis)) pc = true;
	for (int r = 0; r < infoCnt[st].nearCnt.size(); r++)
		if (!visited[infoCnt[st].nearCnt[r]] && !parent(st, infoCnt[st].nearCnt[r]))
			DFS(infoCnt[st].nearCnt[r], pc, axis);
}

void percolationClusters(vector <int> &pClus, vector <int> locVector, int axis)
{
	for (int i = 0; i < locVector.size(); i++)
	{
		bool f = false;
		for (int j = 0; j < pClus.size(); j++)
			if (infoCnt[locVector[i]].idClus == pClus[j]) f = true; //этот перколяционный кластер уже найден
		if (f) continue;

		for (int j = 0; j < infoCnt.size(); j++)
			visited[j] = false;

		bool pc = false;
		DFS(locVector[i], pc, axis);
		if (pc) // если кластер перколяционный, то добавляем в список id
			pClus.push_back(infoCnt[locVector[i]].idClus);
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
void clustering(int &clusters, vector <int> &pClusters)
{
	//поиск всех кластеров 
	visited = new bool[infoCnt.size()];
	for (int w = 0; w < infoCnt.size(); w++)
		visited[w] = false;
	vector <int> colour(0);
	for (int q = 0; q < infoCnt.size(); q++)
	{
		if (infoCnt[q].idClus == 0)
		{
			clusters++;
			colour.clear();
			for (int j = 0; j<3; j++)
				colour.push_back((int)(mt.getReal1() * 225));
			DFS(q, clusters, colour);
		}
	}
	int x = 0, y = 1;
	//поиск перколяционных кластеров
	vector <int> x0(0), y0(0);
	for (int w = 0; w < infoCnt.size(); w++)
	{
		if (p0(infoCnt[w].poly, x)) x0.push_back(w);
		if (p0(infoCnt[w].poly, y)) y0.push_back(w);
	}


	for (int i = 0; i < x0.size(); i++)
		fileLog << x0[i] << " ";
	fileLog << endl << x0.size() << endl;

	for (int i = 0; i < y0.size(); i++)
		fileLog << y0[i] << " ";
	fileLog << endl << y0.size() << endl;

	percolationClusters(pClusters, x0, x);
	percolationClusters(pClusters, y0, y);
	x0.clear();
	y0.clear();
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


	ofstream svg("img.svg");
	bg::svg_mapper<point_type> mapper(svg, 400, 400, "viewBox = \"200 -30 500 500\"");

	mapper.add(poly);
	mapper.map(poly, "fill-opacity:0.1;fill:rgb(51,51,153);stroke:rgb(51,51,153);stroke-width:1");

	//упаковка 
	pack();

	//поиск всех кластеров 
	int clusters = 0;
	vector <int> pClusters(0);
	clustering(clusters, pClusters);

	for (int i = 0; i < infoCnt.size(); i++)
	{
		mapper.add(infoCnt[i].poly);
		string str = "fill-opacity:0.1;fill:rgb(0,0,0);stroke:rgb(" + 
							toStr(infoCnt[i].colour[0]) + "," +
							toStr(infoCnt[i].colour[1]) + "," +
							toStr(infoCnt[i].colour[2]) + ");stroke-width:1";
		mapper.map(infoCnt[i].poly, str);

		
	}
	
	cout << "Кластеров " << clusters << ", из них перколяционных: "<< pClusters.size() << endl;
	for(int i=0; i<pClusters.size(); i++)
		cout << " " << pClusters[i] << endl;
	temp.clear();
	tempCnt.clear();
	infoCnt.clear();
	pClusters.clear();
    return 0;
}

