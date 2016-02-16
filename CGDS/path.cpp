/*
 * This file is part of GDSII.
 *
 * path.cpp -- The source file which defines the PATH of GDSII format.
 *
 * Copyright (c) 2015 Kangpeng Shao <billowen035@gmail.com>
 *
 * GDSII is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at you option) any later version.
 *
 * GDSII is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABLILTY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GDSII. If not, see <http://www.gnu.org/licenses/>.
 **/

#include <assert.h>
#include "path.h"
#include <sstream>
#include <algorithm>
#include "gdsio.h"

namespace GDS
{

Path::Path(Structure *parent) :Element(PATH, parent)
{
    mEflags = 0;
    mLayer = -1;
    mDataType = -1;
    mWidth = 0;
    mPathType = 0;
}

Path::~Path()
{

}

short Path::Layer() const
{
    return mLayer;
}

short Path::DataType() const
{
    return mDataType;
}

int Path::Width() const
{
    return mWidth;
}

short Path::PathType() const
{
    return mPathType;
}

std::vector<Point> Path::XY() const
{
    return mPts;
}

void Path::SetLayer(short layer)
{
    mLayer = layer;
}

void Path::SetDataType(short data_type)
{
    mDataType = data_type;
}

void Path::SetWidth(int width)
{
    mWidth = width;
}

void Path::SetPathType(short type)
{
    mPathType = type;
}

void Path::SetXY(const std::vector<Point> &pts)
{
    mPts.clear();
    mPts = pts;
}

bool Path::BBox(int &x, int &y, int &w, int &h) const
{
    assert(mWidth > 0 && mPts.size() >= 2);
    if (mWidth == 0 || mPts.size() < 2)
        return false;

    int llx = GDS_MAX_INT;
    int lly = GDS_MAX_INT;
    int urx = GDS_MIN_INT;
    int ury = GDS_MIN_INT;
    for (auto pt : mPts)
    {
        llx = pt.X < llx ? pt.X : llx;
        lly = pt.Y < lly ? pt.Y : lly;
        urx = pt.X > urx ? pt.X : urx;
        ury = pt.Y > ury ? pt.Y : ury;
    }
    // extend the path
    for (size_t i = 1; i < mPts.size(); i++)
    {
        if (mPts[i - 1].X == mPts[i].X)
        {
            llx = (mPts[i].X - mWidth / 2) < llx ? (mPts[i].X - mWidth / 2) : llx;
            urx = (mPts[i].X + mWidth / 2) > urx ? (mPts[i].X + mWidth / 2) : urx;
        }
        else
        {
            lly = (mPts[i].Y - mWidth / 2) < lly ? (mPts[i].Y - mWidth / 2) : lly;
            ury = (mPts[i].Y + mWidth / 2) > ury ? (mPts[i].Y + mWidth / 2) : ury;
        }
    }
    // extend the end point
    if (mPathType > 0)
    {
        if (mPts[0].X == mPts[1].X)
        {
            if (mPts[0].Y < mPts[1].Y)
                lly = (mPts[0].Y - mWidth / 2) < lly ? (mPts[0].Y - mWidth / 2) : lly;
            else
                ury = (mPts[0].Y + mWidth / 2) > ury ? (mPts[0].Y + mWidth / 2) : ury;
        }
        else
        {
            if (mPts[0].X < mPts[1].X)
                llx = (mPts[0].X - mWidth / 2) < llx ? (mPts[0].X - mWidth / 2) : llx;
            else
                urx = (mPts[0].X + mWidth / 2) > urx ? (mPts[0].X + mWidth / 2) : urx;
        }
        size_t n = mPts.size();
        if (mPts[n - 2].X == mPts[n - 1].X)
        {
            if (mPts[n - 1].Y < mPts[n - 2].Y)
                lly = (mPts[n - 1].Y - mWidth / 2) < lly ? (mPts[n - 1].Y - mWidth / 2) : lly;
            else
                ury = (mPts[n - 1].Y + mWidth / 2) > ury ? (mPts[n - 1].Y + mWidth / 2) : ury;
        }
        else 
        {
            if (mPts[n - 1].X < mPts[n - 2].X)
                llx = (mPts[n - 1].X - mWidth / 2) < llx ? (mPts[n - 1].X - mWidth / 2) : llx;
            else
                urx = (mPts[n - 1].X + mWidth / 2) > urx ? (mPts[n - 1].X + mWidth / 2) : urx;
        }
    }
    x = llx;
    y = lly;
    w = urx - llx;
    h = ury - lly;

    return true;
}

//int Path::read(std::ifstream &in, std::string &msg)
//{
//    msg = "";
//    bool finished = false;
//    while (!finished)
//    {
//        short record_size;
//        Byte record_type, record_dt;
//        if (!readShort(in, record_size)
//            || !readByte(in, record_type)
//            || !readByte(in, record_dt))
//            return FILE_ERROR;
//        switch (record_type)
//        {
//        case ENDEL:
//            finished = true;
//            break;
//        case EFLAGS:
//            if (record_size != 6)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of EFLAGS (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            if (!readShort(in, mEflags))
//                return FILE_ERROR;
//            break;
//        case LAYER:
//            if (record_size != 6)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of LAYER (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            if (!readShort(in, mLayer))
//                return FILE_ERROR;
//            break;
//        case DATATYPE:
//            if (record_size != 6)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of DATATYPE (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            if (!readShort(in, mDataType))
//                return FILE_ERROR;
//            break;
//        case XY:
//        {
//            record_size -= 4;
//            int num = record_size / 8;
//            if (record_size % 8 != 0 || num < 2)
//            {
//                std::stringstream ss;
//                ss << "wrong record size of XY for PATH (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << "). ";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            for (int i = 0; i < num; i++)
//            {
//                int x, y;
//                if (!readInteger(in, x)
//                    || !readInteger(in, y))
//                    return FILE_ERROR;
//                mPts.push_back(Point(x, y));
//            }
//            break;
//        }
//        case WIDTH:
//            if (record_size != 8)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of WIDTH (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << "). ";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            if (!readInteger(in, mWidth))
//                return FILE_ERROR;
//            break;
//        case PATHTYPE:
//            if (record_size != 6)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of PATHTYPE (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << "). ";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            if (!readShort(in, mPathType))
//                return FILE_ERROR;
//            break;
//        default:
//            break;
//        }
//    }
//
//    return 0;
//}
//
//int Path::write(std::ofstream &out, std::string &msg)
//{
//    short record_size;
//
//    record_size = 4;
//    writeShort(out, record_size);
//    writeByte(out, PATH);
//    writeByte(out, NoData);
//
//    record_size = 6;
//    writeShort(out, record_size);
//    writeByte(out, EFLAGS);
//    writeByte(out, Integer_2);
//    writeShort(out, mEflags);
//
//    record_size = 6;
//    writeShort(out, record_size);
//    writeByte(out, LAYER);
//    writeByte(out, Integer_2);
//    writeShort(out, mLayer);
//
//    record_size = 6;
//    writeShort(out, record_size);
//    writeByte(out, DATATYPE);
//    writeByte(out, Integer_2);
//    writeShort(out, mDataType);
//
//    record_size = 8;
//    writeShort(out, record_size);
//    writeByte(out, WIDTH);
//    writeByte(out, Integer_4);
//    writeInteger(out, mWidth);
//
//    record_size = 6;
//    writeShort(out, record_size);
//    writeByte(out, PATHTYPE);
//    writeByte(out, Integer_2);
//    writeShort(out, mPathType);
//
//    record_size = 4 + short(8 * mPts.size());
//    writeShort(out, record_size);
//    writeByte(out, XY);
//    writeByte(out, Integer_4);
//    for (auto p : mPts)
//    {
//        writeInteger(out, p.x);
//        writeInteger(out, p.y);
//    }
//
//    record_size = 4;
//    writeShort(out, record_size);
//    writeByte(out, ENDEL);
//    writeByte(out, NoData);
//
//    return out.fail() ? FILE_ERROR : 0;
//}
}

