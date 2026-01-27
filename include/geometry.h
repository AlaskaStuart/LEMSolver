#ifndef LEMSOLVER_GEOMETRY_H
#define LEMSOLVER_GEOMETRY_H

#include "utils.h"
#include "settings.h"

Point2D getMidPoint(Point2D &entryPoint, Point2D &exitPoint);

double calculateLength(std::array<double, 2> &exitPoint, std::array<double, 2> &entryPoint);

std::pair<double, double> calculateDistToPolyline(Point2D &ptP, Point2D &ptA, Point2D &ptB);

MultiPoints2D cutFromPolyLine(const MultiPoints2D &polyline,
                              std::array<double, 2> startPoint,
                              std::array<double, 2> endPoint);

std::vector<double> interpolate(double start, double end, int numPoints);

MultiPoints2D interpolatedPolyLine(const MultiPoints2D &polyline, int numPoints);

// double calculateCircumToChordDist(double chordLength, double chordAngle);

double getRadiusByChordCircum(double chordLength, double circumToChord);

std::array<double, 2> getCenter(std::array<double, 2> chordMid, double angle, double radius, double chordtoCircum);

bool isLineIntersectedCircle(const MultiPoints2D &polyline,
                             const std::array<double, 2> &center,
                             double radius);

// double calculateAreaOfQuad(const MultiPoints2D &pts);

int intersectionNum(const MultiPoints2D &polyline, const std::array<double, 2> &center, double radius);

// std::optional<std::pair<int, PolyLineCAD>> polyLineCADIntersection(const PolyLineCAD &P, PolyLineCAD &Q);

double getYbyX(const MultiPoints2D &polyline, double x);

std::optional<Point2D> segmentIntersection(const Point2D &A, const Point2D &B, const Point2D &C, const Point2D &D);

// 折线与圆弧段相交
PolyLineCAD generateCompositeSlip(const Point2D &entryPt, const Point2D &exitPt, const Point2D &arcCenter, const double &radius, const MultiPoints2D &mPts);

// 从两个点和圆心、圆弧去生成CAD多线段表示法
PolyLineCAD generatePloyLineCAD(const Point2D &A, const Point2D &B, const Point2D &center, double radius, bool ccw = true);

// 从CAD多线段转化出两个点、圆心和半径，是否顺时针或者逆时针
std::tuple<Point2D, Point2D, Point2D, double, bool> polyLineCADToPointAndCircle(const PolyLineCAD &P);

MultiPoints2D intersectionArcAndPolyLine(const MultiPoints2D &polyline, const Point2D &center, double radius);

#endif