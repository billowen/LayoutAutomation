/*
 * This file is part of GDSII.
 *
 * library.h -- The header file which declare the library in GDSII format.
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

#ifndef LIBRARY_H
#define LIBRARY_H

#include <string>
#include <vector>
#include <map>
#include "sqlite/sqlite3.h"

namespace GDS 
{
//class Techfile;
class Structure;


class Library
{
    

        
public:
    Library();
    ~Library();

    void Init();

    size_t Size() const;
    Structure *Get(int index);
    Structure *Get(std::string name);
    Structure *Add(std::string name);
    void Del(std::string name);
    //void BuildCellLinks(bool del_dirty_links = false);
    //void CollectLayers(Techfile &tech_file);
    bool OpenDB(const std::string &file_name, std::string &err);
    void CloseDB();
    void Clear();

    /*int read(std::ifstream &in, std::string &msg);
    int write(std::ofstream &out, std::string &msg);*/
private:
    short           mVersion;
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
    std::string     mLibName;
    double          mDBUnitInMeter;
    double          mDBUnitInUserUnit;

    std::vector<Structure*> mCells;
    std::vector<Structure*> mDeletedCells;
    std::map<std::string, int> _CellMaps;   //< 

    sqlite3 *mDBConnection;

};
}

extern const char *INFO_TABLE;
extern const char *CELL_TABLE;
extern const char *GDS_VERSION_ID;
extern const char *MOD_TIME_ID;
extern const char *ACC_TIME_ID;
extern const char *LIB_NAME_ID;
extern const char *UNITS_ID;
extern const char *DATA_COL_NAME;

#endif // LIBRARY_H

