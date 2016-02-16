/*
 * This file is part of GDSII.
 *
 * path.h -- The header file which declare the PATH in GDSII format.
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

#ifndef GDS_PATH_H
#define GDS_PATH_H
#include "elements.h"

namespace GDS {

/*!
    * \brief Class for 'PATH' GDSII element
    *
    * GDS syntax:
    *  PATH
    *  [EFLAGS]
    *  [PLEX]
    *  LAYER
    *  DATATYPE
    *  [PATHTYPE]
    *  [WIDTH]
    *  [BGNEXTN]
    *  [ENDEXTN]
    *  XY
    *  ENDEL
    */
class Path : public Element 
{
public:
    Path(Structure* parent = nullptr);
    virtual ~Path();

    short Layer() const;
    short DataType() const;
    int Width() const;
    short PathType() const;
    std::vector<Point> XY() const;
    virtual bool BBox(int &x, int &y, int &w, int &h) const;

    void SetLayer(short layer);
    void SetDataType(short data_type);
    void SetWidth(int width);
    void SetPathType(short type);
    void SetXY(const std::vector<Point> &pts);

    /*virtual int read(std::ifstream &in, std::string &msg);
    virtual int write(std::ofstream &out, std::string &msg);*/

private:
    short               mEflags;         //< 2 bytes of bit flags. Not support yet.
    short               mLayer;
    short               mDataType;
    int                 mWidth;
    short               mPathType;
    std::vector<Point>  mPts;
};

}
#endif // PATH_H

