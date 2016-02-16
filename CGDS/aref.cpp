/*
 * This file is part of GDSII.
 *
 * aref.cpp -- The source file which defines the AREF of GDSII format.
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

//#include <QtGui/QTransform>
#include <assert.h>
#include "aref.h"
#include <sstream>
#include "gdsio.h"
#include "structures.h"
#include "library.h"
#include "transform.h"

namespace GDS
{

ARef::ARef(Structure *parent) :Element(AREF, parent)
{
    mEflags = 0;
    mSName = "";
    mStrans = 0;
    mRow = 0;
    mCol = 0;
    mAngle = 0;
    mMag = 1;
}

ARef::~ARef()
{
}

std::string ARef::SName() const
{
    return mSName;
}

short ARef::Row() const
{
    return mRow;
}

short ARef::Col() const
{
    return mCol;
}

std::vector<Point> ARef::XY() const
{
    return mPts;
}

double ARef::Angle() const
{
    return mAngle;
}

double ARef::Mag() const
{
    return mMag;
}

short ARef::Strans() const
{
    return mStrans;
}

bool ARef::StransFlag(STRANS_FLAG flag) const
{
    return (mStrans & flag) != 0;
}

void ARef::SetSName(std::string name)
{
    mSName = name;
}

void ARef::SetRowCol(int row, int col)
{
    mRow = row;
    mCol = col;
}

void ARef::SetXY(std::vector<Point> pts)
{
    mPts = pts;
}

void ARef::SetAngle(double angle)
{
    mAngle = angle;
}

void ARef::SetMag(double mag)
{
    mMag = mag;
}

void ARef::SetStrans(short strans)
{
    mStrans = strans;
}

void ARef::SetStrans(STRANS_FLAG flag, bool enable)
{
    mStrans = enable ? (mStrans | flag) : (mStrans & (~flag));
}


bool ARef::BBox(int &x, int &y, int &w, int &h) const
{
    Library *gds = nullptr;
    if (Parent() == nullptr)
        return false;
    gds = Parent()->Parent();
    if (gds == nullptr)
        return false;
    Structure *reference = gds->Get(SName());
    if (reference == nullptr)
        return false;

    int ref_x, ref_y, ref_w, ref_h;
    if (!reference->BBox(ref_x, ref_y, ref_w, ref_h))
        return false;

    assert(mPts.size() == 3);
    int row_pitch_x = (mPts[2].X - mPts[0].X) / Row();
    int row_pitch_y = (mPts[2].Y - mPts[0].Y) / Row();
    int col_pitch_x = (mPts[1].X - mPts[0].X) / Col();
    int col_pitch_y = (mPts[1].Y - mPts[0].Y) / Col();
    Point translate_pts[] = {
        Point(mPts[0].X, mPts[0].Y),
        Point(mPts[0].X + col_pitch_x * (Col() - 1), mPts[0].Y + col_pitch_y * (Col() - 1)),
        Point(mPts[0].X + row_pitch_x * (Row() - 1), mPts[0].Y + row_pitch_y * (Row() - 1)),
        Point(mPts[0].X + row_pitch_x * (Row() - 1) + col_pitch_x * (Col() - 1),
              mPts[0].Y + row_pitch_y * (Row() - 1) + col_pitch_y * (Col() - 1))
    };

    int llx = GDS_MAX_INT;
    int lly = GDS_MAX_INT;
    int urx = GDS_MIN_INT;
    int ury = GDS_MIN_INT;

    for (int i = 0; i < 4; i++)
    {
        Transform transform;
        if (StransFlag(REFLECTION))
            transform.Scale(1, -1);
        transform.Scale(Mag(), Mag());
        transform.Rotate(Angle());
        transform.Translate(translate_pts[i].X, translate_pts[i].Y);

        std::vector<Point> rect_pts;
        rect_pts.push_back(Point(ref_x, ref_y));
        rect_pts.push_back(Point(ref_x + ref_w, ref_y));
        rect_pts.push_back(Point(ref_x + ref_w, ref_y + ref_h));
        rect_pts.push_back(Point(ref_x, ref_y + ref_h));

        for (auto p : rect_pts)
        {
            Point tmp = transform.Map(p);
            llx = tmp.X < llx ? tmp.X : llx;
            urx = tmp.X > urx ? tmp.X : urx;
            lly = tmp.Y < lly ? tmp.Y : lly;
            ury = tmp.Y > ury ? tmp.Y : ury;
        }
    }

    x = llx;
    y = lly;
    w = urx - llx;
    h = ury - lly;

    return true;
}

//int ARef::read(std::ifstream &in, std::string &msg)
//{
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
//        case SNAME:
//            if (record_size < 4 || record_size % 2 != 0)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of SNAME (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            if (!readString(in, record_size - 4, mSName))
//                return FILE_ERROR;
//            break;
//        case XY:
//            if (record_size != 28)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of XY for AREF (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << "). ";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            for (int i = 0; i < 3; i++)
//            {
//                int x, y;
//                if (!readInteger(in, x)
//                    || !readInteger(in, y))
//                    return FILE_ERROR;
//                mPts.push_back(Point(x, y));
//            }
//            break;
//        case STRANS:
//            if (record_size != 6)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of STRANS (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            if (!readShort(in, mStrans))
//                return FILE_ERROR;
//            break;
//        case COLROW:
//            if (record_size != 8)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of COLROW (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            if (!readShort(in, mCol)
//                || !readShort(in, mRow))
//                return FILE_ERROR;
//            break;
//        case MAG:
//            if (record_size != 12)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of MAG (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            if (!readDouble(in, mMag))
//                return FILE_ERROR;
//            break;
//        case ANGLE:
//            if (record_size != 12)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of ANGLE (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            if (!readDouble(in, mAngle))
//                return FILE_ERROR;
//            break;
//        default:
//            break;
//        }
//    }
//    return 0;
//}
//
//int ARef::write(std::ofstream &out, std::string &msg)
//{
//    short record_size;
//
//    // AREF
//    record_size = 4;
//    writeShort(out, record_size);
//    writeByte(out, AREF);
//    writeByte(out, NoData);
//
//    record_size = 6;
//    writeShort(out, record_size);
//    writeByte(out, EFLAGS);
//    writeByte(out, Integer_2);
//    writeShort(out, mEflags);
//
//    record_size = 4 + short(mSName.size());
//    if (record_size % 2 != 0)
//        record_size += 1;
//    writeShort(out, record_size);
//    writeByte(out, SNAME);
//    writeByte(out, String);
//    writeString(out, mSName);
//
//    record_size = 6;
//    writeShort(out, record_size);
//    writeByte(out, STRANS);
//    writeByte(out, Integer_2);
//    writeShort(out, mStrans);
//
//    record_size = 8;
//    writeShort(out, record_size);
//    writeByte(out, COLROW);
//    writeByte(out, Integer_2);
//    writeShort(out, mCol);
//    writeShort(out, mRow);
//
//    record_size = 28;
//    writeShort(out, record_size);
//    writeByte(out, XY);
//    writeByte(out, Integer_4);
//    for (int i = 0; i < 3; i++)
//    {
//        writeInteger(out, mPts[i].x);
//        writeInteger(out, mPts[i].y);
//    }
//
//    record_size = 12;
//    writeShort(out, record_size);
//    writeByte(out, ANGLE);
//    writeByte(out, Real_8);
//    writeDouble(out, mAngle);
//
//    record_size = 12;
//    writeShort(out, record_size);
//    writeByte(out, MAG);
//    writeByte(out, Real_8);
//    writeDouble(out, mMag);
//
//    record_size = 4;
//    writeShort(out, record_size);
//    writeByte(out, ENDEL);
//    writeByte(out, NoData);
//
//    return out.fail() ? FILE_ERROR : 0;
//}

}


