/*
 * This file is part of GDSII.
 *
 * aref.h -- The header file which declare the AREF in GDSII format.
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

#ifndef AREF_H
#define AREF_H
#include "elements.h"

namespace GDS {
class Structure;
/*!
    * \brief Class for 'AREF' GDSII element
    *
    * GDS syntax:
    * GDS syntax:
    *  AREF
    *  [EFLAGS]
    *  [PLEX]
    *  SNAME
    *  [STRANS]
    *  COLROW
    *  XY
    *  ENDEL
    */
class ARef : public Element {
    short               mEflags;
    std::string         mSName;
    short               mStrans;
    short               mRow, mCol;
    std::vector<Point>  mPts;
    double              mAngle;
    double              mMag;

public:
    ARef(Structure *parent = nullptr);
    virtual ~ARef();

    std::string SName() const;
    short Row() const;
    short Col() const;
    std::vector<Point> XY() const;
    double Angle() const;
    double Mag() const;
    short Strans() const;
    bool StransFlag(STRANS_FLAG flag) const;
    virtual bool BBox(int &x, int &y, int &w, int &h) const;

    void SetSName(std::string name);
    void SetRowCol(int row,  int col);
    void SetXY(std::vector<Point> pts);
    void SetAngle(double angle);
    void SetMag(double mag);
    void SetStrans(short strans);
    void SetStrans(STRANS_FLAG flag, bool enable = true);

    /*virtual int read(std::ifstream &in, std::string &msg);
    virtual int write(std::ofstream &out, std::string &msg);*/
};

}

#endif // AREF_H

