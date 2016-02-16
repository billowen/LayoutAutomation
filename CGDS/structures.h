/*
 * This file is part of GDSII.
 *
 * struct.h -- The header file which declare the structure of GDSII format.
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

#ifndef GDS_STRUCTURES_H
#define GDS_STRUCTURES_H
#include <vector>
#include <string>
#include <fstream>

namespace GDS {
class Library;
class Element;

class Structure
{ 
public:
    Structure(Library *parent = nullptr);
    Structure(std::string name, Library *parent = nullptr);
    ~Structure();

    std::string Name() const;
    size_t Size() const;
    Element* Get(int index) const;
    /*!
    Get the boundary rect of current structure.
    @param x[out], y[out] The left bottom point of boundary rect.
    @param w[out] The width of boundary rect.
    @param h[out] The height of boundary rect.
    @return The usefulness of the boundary rect data. If this is a
    empty structure, the output data is useless, and the function
    will return false.
    */
    bool BBox(int &x, int &y, int &w, int &h) const;
    void Add(Element *new_element);
    bool IsCached() const;
    void SetCached(bool flag);
    bool IsChanged() const;
    void SetChanged(bool flag);
    Library *Parent() const;

private:
    
    void SetParent(Library *parent);

    std::string     mStructName;
    short           mModYear;
    short           mModMonth;
    short           mModDay;
    short           mModHour;
    short           mModMinute;
    short           mModSecond;
    short           mAccYear;
    short           mAccMonth;
    short           mAccDay;
    short           mAccHour;
    short           mAccMinute;
    short           mAccSecond;

    std::vector<Element*> mElements;
    Library *mParent;

    bool mIsCached;     //< Indicate the content of current cell has been cached or not.
    bool mIsChanged;
    
};

}



#endif // STRUCTURES_H

