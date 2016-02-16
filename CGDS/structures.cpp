/*
 * This file is part of GDSII.
 *
 * structure.cpp -- The source file which defines the structure of GDSII format.
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

#include <iostream>
#include <limits>
#include <algorithm>
#include <sstream>
#include "tags.h"
#include "structures.h"
#include "boundary.h"
#include "path.h"
#include "sref.h"
#include "aref.h"
#include "gdsio.h"
//#include "text.h"
#include <ctime>

namespace GDS
{

Structure::Structure(Library *parent)
{
    mStructName = "";
    mParent = parent;

    time_t now = time(0);
    tm *ltm = localtime(&now);
    mModYear = ltm->tm_year + 1900;
    mModMonth = ltm->tm_mon + 1;
    mModDay = ltm->tm_mday;
    mModHour = ltm->tm_hour + 1;
    mModMinute = ltm->tm_min + 1;
    mModSecond = ltm->tm_sec + 1;
    mAccYear = ltm->tm_year + 1900;
    mAccMonth = ltm->tm_mon + 1;
    mAccDay = ltm->tm_mday;
    mAccHour = ltm->tm_hour + 1;
    mAccMinute = ltm->tm_min + 1;
    mAccSecond = ltm->tm_sec + 1;

    mIsCached = true;
    mIsChanged = false;
}

Structure::Structure(std::string name, Library *parent)
{
    mStructName = name;
    mParent = parent;

    time_t now = time(0);
    tm *ltm = localtime(&now);
    mModYear = ltm->tm_year + 1900;
    mModMonth = ltm->tm_mon + 1;
    mModDay = ltm->tm_mday;
    mModHour = ltm->tm_hour + 1;
    mModMinute = ltm->tm_min + 1;
    mModSecond = ltm->tm_sec + 1;
    mAccYear = ltm->tm_year + 1900;
    mAccMonth = ltm->tm_mon + 1;
    mAccDay = ltm->tm_mday;
    mAccHour = ltm->tm_hour + 1;
    mAccMinute = ltm->tm_min + 1;
    mAccSecond = ltm->tm_sec + 1;
    
    mIsCached = true;
    mIsChanged = false;
}

Structure::~Structure()
{
    mElements.clear();
}

Library* Structure::Parent() const
{
    return mParent;
}

std::string Structure::Name() const
{
    return mStructName;
}

size_t Structure::Size() const
{
    return mElements.size();
}

Element *Structure::Get(int index) const
{
    if (index < 0 || index >= (int)mElements.size())
        return nullptr;
    else
        return mElements[index];
}

void Structure::Add(Element *new_element)
{
    if (new_element == nullptr)
        return;
    mElements.push_back(new_element);
    new_element->SetParent(this);
}

void Structure::SetParent(Library* parent)
{
    mParent = parent;
}

bool Structure::BBox(int &x, int &y, int &w, int &h) const
{
    int llx = GDS_MAX_INT;
    int lly = GDS_MAX_INT;
    int urx = GDS_MIN_INT;
    int ury = GDS_MIN_INT;
    bool ret = false;
    for (auto node : mElements)
    {
        int _x, _y, _w, _h;
        if (node->BBox(_x, _y, _w, _h))
        {
            ret = true;
            llx = _x < llx ? _x : llx;
            lly = _y < lly ? _y : lly;
            urx = (_x + _w) > urx ? (_x + _w) : urx;
            ury = (_y + _h) > ury ? (_y + _h) : ury;
        }
    }
    x = llx;
    y = lly;
    w = urx - llx;
    h = ury - lly;

    return ret;
}

//void Structure::AddReferBy(std::shared_ptr<Structure> cell)
//{
//    bool existed = false;
//    for (auto tmp : mReferBy)
//    {
//        auto node = tmp.lock();
//        if (!node)
//            continue;
//        if (node->name() == cell->Name())
//        {
//            existed = true;
//            break;
//        }
//    }
//    if (!existed)
//        mReferBy.push_back(cell);
//}


//
//int Structure::read(std::ifstream &in, std::string &msg)
//{
//    if (!readShort(in, mModYear)
//        || !readShort(in, mModMonth)
//        || !readShort(in, mModDay)
//        || !readShort(in, mModHour)
//        || !readShort(in, mModMinute)
//        || !readShort(in, mModSecond)
//        || !readShort(in, mAccYear)
//        || !readShort(in, mAccMonth)
//        || !readShort(in, mAccDay)
//        || !readShort(in, mAccHour)
//        || !readShort(in, mAccMinute)
//        || !readShort(in, mAccSecond))
//        return FILE_ERROR;
//
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
//        case ENDSTR:
//            if (record_size != 4)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of ENDSTR (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            finished = true;
//            break;
//        case STRNAME:
//            if (record_size < 4 || record_size % 2 != 0)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of STRNAME (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            if (!readString(in, record_size - 4, mStructName))
//                return FILE_ERROR;
//            break;
//        case TEXT:
//        {
//            if (record_size != 4)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of TEXT (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            std::shared_ptr<Text> e = std::make_shared<Text>();
//            int error_code = e->read(in, msg);
//            if (error_code > 0)
//                return error_code;
//            mElements.push_back(e);
//            break;
//        }
////		case NODE:
////		{
////			if (record_size != 4)
////			{
////				std::stringstream ss;
////				ss << "Wrong record size of NODE (";
////				ss << std::hex << record_size << record_type << record_dt;
////				ss << ").";
////				msg = ss.str();
////				return FORMAT_ERROR;
////			}
////			std::shared_ptr<Node> e = std::make_shared<Node>();
////			int error_code = e->read(in, msg);
////			if (error_code > 0)
////				return error_code;
////			Elements.push_back(e);
////			break;
////		}
//        case BOUNDARY:
//        {
//            if (record_size != 4)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of BOUNDARY (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            std::shared_ptr<Boundary> e = std::make_shared<Boundary>();
//            int error_code = e->read(in, msg);
//            if (error_code > 0)
//                return error_code;
//            mElements.push_back(e);
//            break;
//        }
//        case PATH:
//        {
//            if (record_size != 4)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of PATH (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            std::shared_ptr<Path> e = std::make_shared<Path>();
//            int error_code = e->read(in, msg);
//            if (error_code > 0)
//                return error_code;
//            mElements.push_back(e);
//            break;
//        }
//        case SREF:
//        {
//            if (record_size != 4)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of SREF (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            std::shared_ptr<SRef> e = std::make_shared<SRef>(); 
//            int error_code = e->read(in, msg);
//            if (error_code > 0)
//                return error_code;
//            mElements.push_back(e);
//            break;
//        }
//        case AREF:
//        {
//            if (record_size != 4)
//            {
//                std::stringstream ss;
//                ss << "Wrong record size of AREF (";
//                ss << std::hex << record_size << record_type << record_dt;
//                ss << ").";
//                msg = ss.str();
//                return FORMAT_ERROR;
//            }
//            std::shared_ptr<ARef> e = std::make_shared<ARef>();
//            int error_code = e->read(in, msg);
//            if (error_code > 0)
//                return error_code;
//            mElements.push_back(e);
//            break;
//        }
//        default:
//            break;
//        }
//    }
//    return 0;
//}
//
//int Structure::write(std::ofstream &out, std::string &msg)
//{
//    short record_size;
//
//    record_size = 28;
//    writeShort(out, record_size);
//    writeByte(out, BGNSTR);
//    writeByte(out, Integer_2);
//    writeShort(out, mModYear);
//    writeShort(out, mModMonth);
//    writeShort(out, mModDay);
//    writeShort(out, mModHour);
//    writeShort(out, mModMinute);
//    writeShort(out, mModSecond);
//    writeShort(out, mAccYear);
//    writeShort(out, mAccMonth);
//    writeShort(out, mAccDay);
//    writeShort(out, mAccHour);
//    writeShort(out, mAccMinute);
//    writeShort(out, mAccSecond);
//
//    record_size = 4 + mStructName.size();
//    if (record_size % 2 != 0)
//        record_size += 1;
//    writeShort(out, record_size);
//    writeByte(out, STRNAME);
//    writeByte(out, String);
//    writeString(out, mStructName);
//
//    if (out.fail())
//        return FILE_ERROR;
//
//    for (auto e : mElements)
//    {
//        int err_code = e->write(out, msg);
//        if (err_code > 0)
//            return err_code;
//    }
//
//    record_size = 4;
//    writeShort(out, record_size);
//    writeByte(out, ENDSTR);
//    writeByte(out, NoData);
//
//    return out.fail() ? FILE_ERROR : 0;
//}


}




