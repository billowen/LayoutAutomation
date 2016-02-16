/*
 * This file is part of GDSII.
 *
 * elements.cpp -- The source file which defines the elements of GDSII format.
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
#include "tags.h"
#include "elements.h"


namespace GDS
{

Element::Element(Structure* parent)
{
    mTag = RECORD_UNKNOWN;
    mParent = parent;
}

Element::Element(Record_type tag, Structure* parent)
{
    mTag = tag;
    mParent = parent;
}

Element::~Element()
{
    
}

std::string Element::Type() const
{
    std::map<int, std::string>::const_iterator iter = Record_name.find(mTag);
    if (iter == Record_name.end())
        return "RECORD_UNKNOWN";
    else
        return iter->second;
}

Structure* Element::Parent() const
{
    return mParent;
}

void Element::SetParent(Structure *parent)
{
    mParent = parent;
}

Record_type Element::Tag() const
{
    return mTag;
}

void Element::SetTag(Record_type tag)
{
    mTag = tag;
}

}

