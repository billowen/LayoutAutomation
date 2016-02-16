/*
 * This file is part of GDSII.
 *
 * boundary.cpp -- The source file which defines the BOUNDARY of GDSII format.
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

#include <limits>
#include <assert.h>
#include "boundary.h"
#include <sstream>
#include "gdsio.h"

namespace GDS
{

Boundary::Boundary(Structure *parent) :Element(BOUNDARY, parent)
{
	mEflags = 0;
	mLayer = -1;
	mDataType = -1;
}

Boundary::~Boundary()
{

}

short Boundary::Layer() const
{
	return mLayer;
}

short Boundary::DataType() const
{
	return mDataType;
}

std::vector<Point> Boundary::XY() const
{
	return mPts;
}

void Boundary::SetLayer(short layer)
{
	mLayer = layer;
}

void Boundary::SetDataType(short data_type)
{
	mDataType = data_type;
}

void Boundary::SetXY(const std::vector<Point> &pts)
{
	mPts.clear();
	mPts = pts;
}

bool Boundary::BBox(int &x, int &y, int &w, int &h) const
{
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
	x = llx;
	y = lly;
	w = urx - llx;
	h = ury - lly;

	return true;
}

//int Boundary::read(std::ifstream &in, std::string &msg)
//{
//	msg = "";
//	bool finished = false;
//	while (!finished)
//	{
//		short record_size;
//		Byte record_type, record_dt;
//		if (!readShort(in, record_size)
//			|| !readByte(in, record_type)
//			|| !readByte(in, record_dt))
//			return FILE_ERROR;
//
//		switch (record_type)
//		{
//		case ENDEL:
//			finished = true;
//			break;
//		case EFLAGS:
//			if (record_size != 6)
//			{
//				std::stringstream ss;
//				ss << "Wrong record size of EFLAGS (";
//				ss << std::hex << record_size << record_type << record_dt;
//				ss << ").";
//				msg = ss.str();
//				return FORMAT_ERROR;
//			}
//			if (!readShort(in, Eflags))
//				return FILE_ERROR;
//			break;
//		case LAYER:
//			if (record_size != 6)
//			{
//				std::stringstream ss;
//				ss << "Wrong record size of LAYER (";
//				ss << std::hex << record_size << record_type << record_dt;
//				ss << ").";
//				msg = ss.str();
//				return FORMAT_ERROR;
//			}
//			if (!readShort(in, Layer))
//				return FILE_ERROR;
//			break;
//		case DATATYPE:
//			if (record_size != 6)
//			{
//				std::stringstream ss;
//				ss << "wrong record size of DATATYPE (";
//				ss << std::hex << record_size << record_type << record_dt;
//				ss << ").";
//				msg = ss.str();
//				return FORMAT_ERROR;
//			}
//			if (!readShort(in, Data_type))
//				return FILE_ERROR;
//			break;
//		case XY:
//		{
//			record_size -= 4;
//			int num = record_size / 8;
//			if (record_size % 8 != 0 || num < 4)
//			{
//				std::stringstream ss;
//				ss << "wrong record size of XY for BOUNDARY (";
//				ss << std::hex << record_size << record_type << record_dt;
//				ss << "). ";
//				std::string msg = ss.str();
//				return FORMAT_ERROR;
//			}
//			for (int i = 0; i < num; i++)
//			{
//				int x, y;
//				if (!readInteger(in, x)
//					|| !readInteger(in, y))
//					return FILE_ERROR;
//				Pts.push_back(Point(x, y));
//			}
//			break;
//		}
//		default:
//			break;
//		}
//	}
//
//	return 0;
//}
//
//int Boundary::write(std::ofstream &out, std::string &msg)
//{
//	short record_size;
//
//	record_size = 4;
//	writeShort(out, record_size);
//	writeByte(out, BOUNDARY);
//	writeByte(out, NoData);
//
//	record_size = 6;
//	writeShort(out, record_size);
//	writeByte(out, EFLAGS);
//	writeByte(out, Integer_2);
//	writeShort(out, Eflags);
//
//	record_size = 6;
//	writeShort(out, record_size);
//	writeByte(out, LAYER);
//	writeByte(out, Integer_2);
//	writeShort(out, Layer);
//
//	record_size = 6;
//	writeShort(out, record_size);
//	writeByte(out, DATATYPE);
//	writeByte(out, Integer_2);
//	writeShort(out, Data_type);
//
//    record_size = 4 + short(8 * Pts.size());
//	writeShort(out, record_size);
//	writeByte(out, XY);
//	writeByte(out, Integer_4);
//	for (auto p : Pts)
//	{
//		writeInteger(out, p.x);
//		writeInteger(out, p.y);
//	}
//
//	record_size = 4;
//	writeShort(out, record_size);
//	writeByte(out, ENDEL);
//	writeByte(out, NoData);
//
//	return out.fail() ? FILE_ERROR : 0;
//}

}


