#ifndef LEMSOLVER_GEOMETRY_H
#define LEMSOLVER_GEOMETRY_H

#include "utils.h"

// 二维单点
using Point2D = std::array<double, 2>;

Point2D operator+(const Point2D &a, const Point2D &b)
{
    return {a[0] + b[0], a[1] + b[1]};
}

Point2D operator-(const Point2D &a, const Point2D &b)
{
    return {a[0] - b[0], a[1] - b[1]};
}

Point2D operator*(const Point2D &v, double s)
{
    return {v[0] * s, v[1] * s};
}

double dot(const Point2D &a, const Point2D &b)
{
    return a[0] * b[0] + a[1] * b[1];
}

double length(const Point2D &v)
{
    return std::sqrt(dot(v, v));
}
// 二维多点
using MultiPoints2D = std::vector<Point2D>;

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

double getYbyX(const MultiPoints2D &polyline, double x);

#endif