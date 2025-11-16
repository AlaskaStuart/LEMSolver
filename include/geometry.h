#ifndef LEMSOLVER_GEOMETRY_H
#define LEMSOLVER_GEOMETRY_H

#include "utils.h"
const std::array<double, 2> &getMidPoint(std::array<double, 2> &entryPoint, std::array<double, 2> &exitPoint);

double calculateLength(std::array<double, 2> &exitPoint, std::array<double, 2> &entryPoint);

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