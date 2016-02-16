/*
 * This file is part of GDSII.
 *
 * sref.h -- The header file which declare the SREF in GDSII format.
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

#ifndef GDS_SREF_H
#define GDS_SREF_H
#include "elements.h"

namespace GDS {
class Structure;
/*!
    * \brief Class for 'SREF' GDSII element
    *
    * GDS syntax:
    *  SREF
    *  [EFLAGS]
    *  [PLEX]
    *  SNAME
    *  [STRANS]
    *  XY
    *  ENDEL
    */
class SRef : public Element 
{
public:
    SRef(Structure *parent = nullptr);
    virtual ~SRef();

    std::string SName() const;
    Point XY() const;
    double Angle() const;
    double Mag() const;
    short Strans() const;
    bool StransFlag(STRANS_FLAG flag) const;
    virtual bool BBox(int &x, int &y, int &w, int &h) const;

    void SetSName(std::string name);
    void SetXY(Point pt);
    void SetAnagle(double angle);
    void SetMag(double mag);
    void SetStrans(short strans);
    void SetStrans(STRANS_FLAG flag, bool enable = true);

    /*virtual int read(std::ifstream &in, std::string &msg);
    virtual int write(std::ofstream &out, std::string &msg);*/

private:
    short               mEflags;
    std::string         mSName;
    short               mStrans;
    Point               mPt;
    double              mAngle;
    double              mMag;
};

}

#endif // SREF_H

