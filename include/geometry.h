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

int intersectionNum(const MultiPoints2D &polyline,
                    const std::array<double, 2> &center,
                    double radius);
std::optional<std::pair<int, PolyLineCAD>> polyLineCADIntersection(const PolyLineCAD &P, PolyLineCAD &Q);

double getYbyX(const MultiPoints2D &polyline, double x);

std::optional<Point2D> segmentIntersection(const Point2D &A, const Point2D &B, const Point2D &C, const Point2D &D);

// 不带曲率的折线相交
// @deprecated
// 可能引发bug
// 放弃了
// std::optional<PolyLineCAD> getUpperPolyLineIntersection(const MultiPoints2D &P, const MultiPoints2D &Q);

// 折线与圆弧段相交
// std::optional<PolyLineCAD> getUpperPolyLineIntersection(const PolyLineCAD &P, PolyLineCAD &Q);

#endif