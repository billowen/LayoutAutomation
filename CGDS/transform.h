
#pragma once

#include "tags.h"

namespace GDS
{
    class Transform
    {
    public:
        Transform();
        ~Transform();

        Transform& Scale(double xScale, double yScale);
        Transform& Translate(double x, double y);
        Transform& Rotate(double degrees);

        Point Map(Point p);
    private:
        double mMatrix[3][3];
    };
}
