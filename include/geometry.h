#ifndef LEMSOLVER_GEOMETRY_H
#define LEMSOLVER_GEOMETRY_H

#include "utils.h"
#include "settings.h"

Point2D getMidPoint(Point2D &entryPoint, Point2D &exitPoint);

double calculateLength(std::array<double, 2> &exitPoint, std::array<double, 2> &entryPoint);

std::pair<double, double> calculateDistToPolyline(Point2D &ptP, Point2D &ptA, Point2D &ptB);

double getRadiusByChordCircum(double chordLength, double circumToChord);

std::array<double, 2> getCenter(std::array<double, 2> chordMid, double angle, double radius, double chordtoCircum);

MultiPoints2D cutFromPolyLine(const MultiPoints2D &polyline, const Point2D &startPoint, const Point2D &endPoint);

std::vector<double> interpolate(double start, double end, int numPoints);

MultiPoints2D interpolatedPolyLine(const MultiPoints2D &polyline, int numPoints);

double getYbyX(const MultiPoints2D &polyline, double x);

int intersectionNum(const MultiPoints2D &polyline, const std::array<double, 2> &center, double radius);

std::optional<Point2D> segmentIntersection(const Point2D &A, const Point2D &B, const Point2D &C, const Point2D &D);

MultiPoints2D polyLineArcIntersection(const MultiPoints2D &polyline, const Point2D &center, double radius);

MultiPoints2D twoPolyLineIntersection(const MultiPoints2D &pLine1, const MultiPoints2D &pLine2);

// 从两个点和圆心、圆弧去生成CAD多线段表示法
PolyLineCAD toPolyLineCAD(const Point2D &A, const Point2D &B, const Point2D &center, double radius, bool ccw = true);

// 从CAD多线段转化出两个点、圆心和半径，是否顺时针或者逆时针
std::tuple<Point2D, Point2D, Point2D, double, bool> fromPolyLineCAD(const PolyLineCAD &P);

#endif