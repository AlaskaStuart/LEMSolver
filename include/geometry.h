#ifndef LEMSOLVER_GEOMETRY_H
#define LEMSOLVER_GEOMETRY_H

#include "utils.h"

MultiPoints2D cutFromPolyLine(const MultiPoints2D &polyline,
                              std::array<double, 2> startPoint,
                              std::array<double, 2> endPoint);

MultiPoints2D interpolatedPolyLine(const MultiPoints2D &polyline, int numPoints);

// double calculateCircumToChordDist(double chordLength, double chordAngle);

double getRadiusByChordCircum(double chordLength, double circumToChord);

bool isLineIntersectedCircle(const MultiPoints2D &polyline,
                             const std::array<double, 2> &center,
                             double radius);

// double calculateAreaOfQuad(const MultiPoints2D &pts);

int intersectionNum(const MultiPoints2D &polyline,
                    const std::array<double, 2> &center,
                    double radius);

double getYbyX(const MultiPoints2D &polyline, double x);

#endif