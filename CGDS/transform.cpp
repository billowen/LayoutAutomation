#include <cmath>
#include "transform.h"

GDS::Transform::Transform()
{
    mMatrix[0][0] = 1;
    mMatrix[0][1] = 0;
    mMatrix[0][2] = 0;
    mMatrix[1][0] = 0;
    mMatrix[1][1] = 1;
    mMatrix[1][2] = 0;
    mMatrix[2][0] = 0;
    mMatrix[2][1] = 0;
    mMatrix[2][2] = 1;
}

GDS::Transform::~Transform()
{
}

GDS::Transform& GDS::Transform::Scale(double xScale, double yScale)
{
    mMatrix[0][0] *= xScale;
    mMatrix[1][0] *= xScale;
    mMatrix[2][0] *= xScale;
    mMatrix[0][1] *= yScale;
    mMatrix[1][1] *= yScale;
    mMatrix[2][1] *= yScale;

    return *this;
}

GDS::Transform & GDS::Transform::Translate(double x, double y)
{
    double tmp[][3] = { {0,0,0},{0,0,0},{0,0,0} };
    tmp[0][0] = mMatrix[0][0] + mMatrix[0][2] * x;
    tmp[1][0] = mMatrix[1][0] + mMatrix[1][2] * x;
    tmp[2][0] = mMatrix[2][0] + mMatrix[2][2] * x;
    tmp[0][1] = mMatrix[0][1] + mMatrix[0][2] * y;
    tmp[1][1] = mMatrix[1][1] + mMatrix[1][2] * y;
    tmp[2][1] = mMatrix[2][1] + mMatrix[2][2] * y;
    tmp[0][2] = mMatrix[0][2];
    tmp[1][2] = mMatrix[1][2];
    tmp[2][2] = mMatrix[2][2];

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            mMatrix[i][j] = tmp[i][j];
        }
    }

    return *this;
}

GDS::Transform & GDS::Transform::Rotate(double degrees)
{
    const double PI = std::atan(1.0) * 4;
    double angle = PI * degrees / 180.0;
    double tmp[][3] = { { 0,0,0 },{ 0,0,0 },{ 0,0,0 } };

    tmp[0][0] = mMatrix[0][0] * std::cos(angle) - mMatrix[0][1] * std::sin(angle);
    tmp[0][1] = mMatrix[0][0] * std::cos(angle) + mMatrix[0][1] * std::sin(angle);
    tmp[0][2] = mMatrix[0][2];
    tmp[1][0] = mMatrix[1][0] * std::cos(angle) - mMatrix[1][1] * std::sin(angle);
    tmp[1][1] = mMatrix[1][0] * std::cos(angle) + mMatrix[1][1] * std::sin(angle);
    tmp[1][2] = mMatrix[1][2];
    tmp[2][0] = mMatrix[2][0] * std::cos(angle) - mMatrix[2][1] * std::sin(angle);
    tmp[2][1] = mMatrix[2][0] * std::cos(angle) + mMatrix[2][1] * std::sin(angle);
    tmp[2][2] = mMatrix[2][2];

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            mMatrix[i][j] = tmp[i][j];
        }
    }

    return *this;
}

GDS::Point GDS::Transform::Map(GDS::Point p)
{
    Point ret;
    ret.X = p.X * mMatrix[0][0] + p.Y * mMatrix[1][0];
    ret.Y = p.X * mMatrix[1][0] + p.Y * mMatrix[1][1];
    return ret;
}



