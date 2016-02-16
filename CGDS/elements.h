/*
 * This file is part of GDSII.
 *
 * elements.h -- The header file which declare the elements in GDSII format.
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

#ifndef ELEMENTS_H
#define ELEMENTS_H
#include "tags.h"
#include <vector>
#include <string>
#include <fstream>

namespace GDS 
{
class Structure;
class Element 
{
public:
    Element(Structure* parent = nullptr);
    Element(Record_type tag, Structure* parent = nullptr);
    virtual ~Element();

    Record_type Tag() const;
    std::string Type() const;
    /*!
    Get the boundary rect of current element.
    @param x[out], y[out] The left bottom point of boundary rect.
    @param w[out] The width of boundary rect.
    @param h[out] The height of boundary rect.
    @return The usefulness of the boundary rect data. If this is a 
            empty element, the output data is useless, and the function
            will return false.
    */
    virtual bool BBox(int &x, int &y, int &w, int &h) const = 0;

    Structure *Parent() const;

    friend class Structure;

protected:
    void SetTag(Record_type tag);
    
    void SetParent(Structure *parent);

private:
    Record_type mTag;
    Structure* mParent;
};

}
#endif // ELEMENTS_H

