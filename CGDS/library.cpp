/*
 * This file is part of GDSII.
 *
 * library.cpp -- The source file which defines the library of GDSII format.
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

#include <assert.h>
#include <fstream>
#include "library.h"
#include "tags.h"
#include "gdsio.h"
#include <sstream>
#include <ctime>
//#include "techfile.h"
#include "elements.h"
#include "aref.h"
#include "sref.h"
#include "boundary.h"
#include "path.h"
#include "structures.h"
#include "sqlite/sqlite3.h"
//#include "text.h"


namespace GDS 
{                                            
    

    Library::Library()
    {
        mDBConnection = nullptr;
        Init();
    }

    Library::~Library()
    {
        Clear();
    }

    void Library::Init()
    {
        mLibName = "";

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

        mVersion = 0;
        mDBUnitInMeter = 1e-9;
        mDBUnitInUserUnit = 1e-3;

        Clear();
    }

    size_t Library::Size() const
    {
        return mCells.size();
    }

    void Library::Clear()
    {
        if (mDBConnection != nullptr)
        {
            sqlite3_close(mDBConnection);
            mDBConnection = nullptr;
        }

        for (auto &e : mCells)
        {
            if (e != nullptr)
            {
                delete e;
                e = nullptr;
            }
        }
        mCells.clear();
        for (auto &e : mDeletedCells)
        {
            if (e != nullptr)
            {
                delete e;
                e = nullptr;
            }
        }
        mDeletedCells.clear();
    }

    Structure *Library::Get(int index)
    {
        if (index < 0 || (size_t)index >= mCells.size())
            return nullptr;
        return mCells[index];
    }

    Structure *Library::Add(std::string name)
    {
        Structure *ret(nullptr);

        Structure *new_item = new Structure(name, this);
        mCells.push_back(new_item);
        ret = new_item;

        return ret;
    }

    Structure *Library::Get(std::string name)
    {
        for (auto& e : mCells)
        {
            if (!e)
                continue;
            if (e->Name() == name)
            {
                return e;
            }
        }
        return nullptr;
    }

    void Library::Del(std::string name)
    {
        for (size_t i = 0; i < Size(); i++)
        {
            auto node = mCells[i];
            if (node == nullptr)
                continue;
            if (node->Name() == name)
            {
                mCells.erase(mCells.begin() + i);
                mDeletedCells.push_back(node);
                break;
            }
        }
    }

    const char *GET_ROWID_TEMPLATE = "SELECT rowid FROM %s WHERE ID='%s';";
    sqlite3_int64 GetRowID(sqlite3 *db, const std::string &table, const std::string &ID, std::string &err)
    {
        char cmd[100];
        sprintf(cmd, GET_ROWID_TEMPLATE, table.c_str(), ID.c_str());
        sqlite3_stmt *ppStmt;
        int rc = sqlite3_prepare_v2(db, cmd, -1, &ppStmt, NULL);
        if (rc != SQLITE_OK)
        {
            err = "SQL error: failed to get row id of " + ID + " in " + table + ".\n";
            sqlite3_finalize(ppStmt);
            return -1;
        }
        rc = sqlite3_step(ppStmt);
        if (rc != SQLITE_DONE)
        {
            err = "SQL error: failed to get row id of " + ID + " in " + table + ".\n";
            sqlite3_finalize(ppStmt);
            return -1;
        }
        sqlite3_int64 ret = sqlite3_column_int64(ppStmt, 0);
        sqlite3_finalize(ppStmt);
        return ret;
    }

    bool Library::OpenDB(const std::string &file_name, std::string &err)
    {
        Init();

        int rc;

        rc = sqlite3_open(file_name.c_str(), &mDBConnection);
        if (rc != SQLITE_OK)
        {
            err = "Failed to open the database in " + file_name + ".\n";
            sqlite3_close(mDBConnection);
            mDBConnection = nullptr;
            return false;
        }

        sqlite3_int64 rowId;
        sqlite3_blob *ppBlob;
        char buffer_2b[2];
        char buffer_8b[8];

        // VERSION
        rowId = GetRowID(mDBConnection, INFO_TABLE, GDS_VERSION_ID, err);
        if (rowId == -1)
        {
            err = "Failed to get version information from database.\n";
            sqlite3_close(mDBConnection);
            mDBConnection = nullptr;
            return false;
        }

        rc = sqlite3_blob_open(mDBConnection,
                               "main",
                               INFO_TABLE,
                               DATA_COL_NAME,
                               rowId,
                               0,
                               &ppBlob);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get version information from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }

        rc = sqlite3_blob_read(ppBlob, buffer_2b, 2, 0);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get version information from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_2b, mVersion);
        sqlite3_blob_close(ppBlob);

        // MOD TIME INFO
        rowId = GetRowID(mDBConnection, INFO_TABLE, MOD_TIME_ID, err);
        if (rowId == -1)
        {
            err = "Failed to get the last modified time from database.\n";
            sqlite3_close(mDBConnection);
            mDBConnection = nullptr;
            return false;
        }
        rc = sqlite3_blob_open(mDBConnection,
                               "main",
                               INFO_TABLE,
                               DATA_COL_NAME,
                               rowId,
                               0,
                               &ppBlob);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last modified time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        rc = sqlite3_blob_read(ppBlob, buffer_2b, 2, 0);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last modified time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_2b, mModYear);
        rc = sqlite3_blob_read(ppBlob, buffer_2b, 2, 2);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last modified time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_2b, mModMonth);
        rc = sqlite3_blob_read(ppBlob, buffer_2b, 2, 4);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last modified time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_2b, mModDay);
        rc = sqlite3_blob_read(ppBlob, buffer_2b, 2, 6);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last modified time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_2b, mModHour);
        rc = sqlite3_blob_read(ppBlob, buffer_2b, 2, 8);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last modified time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_2b, mModMinute);
        rc = sqlite3_blob_read(ppBlob, buffer_2b, 2, 10);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last modified time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_2b, mModSecond);
        sqlite3_blob_close(ppBlob);

        // ACC TIME INFO
        rowId = GetRowID(mDBConnection, INFO_TABLE, ACC_TIME_ID, err);
        if (rowId == -1)
        {
            err = "Failed to get the last access time from database.\n";
            sqlite3_close(mDBConnection);
            mDBConnection = nullptr;
            return false;
        }
        rc = sqlite3_blob_open(mDBConnection,
                               "main",
                               INFO_TABLE,
                               DATA_COL_NAME,
                               rowId,
                               0,
                               &ppBlob);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last access time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        rc = sqlite3_blob_read(ppBlob, buffer_2b, 2, 0);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last access time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_2b, mAccYear);
        rc = sqlite3_blob_read(ppBlob, buffer_2b, 2, 2);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last access time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_2b, mAccMonth);
        rc = sqlite3_blob_read(ppBlob, buffer_2b, 2, 4);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last access time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_2b, mAccDay);
        rc = sqlite3_blob_read(ppBlob, buffer_2b, 2, 6);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last access time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_2b, mAccHour);
        rc = sqlite3_blob_read(ppBlob, buffer_2b, 2, 8);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last access time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_2b, mAccMinute);
        rc = sqlite3_blob_read(ppBlob, buffer_2b, 2, 10);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get the last access time from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_2b, mAccSecond);
        sqlite3_blob_close(ppBlob);

        // UNITS
        rowId = GetRowID(mDBConnection, INFO_TABLE, UNITS_ID, err);
        if (rowId == -1)
        {
            err = "Failed to get units information from database.\n";
            sqlite3_close(mDBConnection);
            mDBConnection = nullptr;
            return false;
        }
        rc = sqlite3_blob_open(mDBConnection,
                               "main",
                               INFO_TABLE,
                               DATA_COL_NAME,
                               rowId,
                               0,
                               &ppBlob);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get units information from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        rc = sqlite3_blob_read(ppBlob, buffer_8b, 8, 0);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get units information from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_8b, mDBUnitInUserUnit);
        rc = sqlite3_blob_read(ppBlob, buffer_8b, 8, 8);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get units information from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        Decode(buffer_8b, mDBUnitInMeter);
        sqlite3_blob_close(ppBlob);

        // LIB NAME
        rowId = GetRowID(mDBConnection, INFO_TABLE, LIB_NAME_ID, err);
        if (rowId == -1)
        {
            err = "Failed to get lib name information from database.\n";
            sqlite3_close(mDBConnection);
            mDBConnection = nullptr;
            return false;
        }
        rc = sqlite3_blob_open(mDBConnection,
                               "main",
                               INFO_TABLE,
                               DATA_COL_NAME,
                               rowId,
                               0,
                               &ppBlob);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get lib name information from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        char libname_buffer[100];
        int nSize = sqlite3_blob_bytes(ppBlob);
        rc = sqlite3_blob_read(ppBlob, libname_buffer, nSize, 0);
        if (rc != SQLITE_OK)
        {
            err = "Failed to get lib name information from database.\n";
            sqlite3_close(mDBConnection);
            sqlite3_blob_close(ppBlob);
            mDBConnection = nullptr;
            return false;
        }
        mLibName.clear();
        for (int i = 0; i < nSize; i++)
        {
            if (libname_buffer[i] == '\0')
                break;
            mLibName.push_back(libname_buffer[i]);
        }
        sqlite3_blob_close(ppBlob);

        char cmd[100];
        sprintf(cmd, "SELECT ID FROM %s;", CELL_TABLE);
        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(mDBConnection, cmd, (int)strlen(cmd), &stmt, 0);
        if (rc != SQLITE_OK)
        {
            err = "Failed during fetching cell info from database.\n";
            sqlite3_finalize(stmt);
            sqlite3_close(mDBConnection);
            mDBConnection = nullptr;
            return false;
        }
        while (true)
        {
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW)
            {
                char cell_name[100];
                sprintf(cell_name, "%s", sqlite3_column_text(stmt, 0));
                Add(std::string(cell_name));
            }
            else if (rc == SQLITE_DONE)
            {
                break;
            }
            else
            {
                err = "Failed during fetching cell info from database.\n";
                sqlite3_finalize(stmt);
                sqlite3_close(mDBConnection);
                mDBConnection = nullptr;
                return false;
            }
        }
        sqlite3_finalize(stmt);
        return true;
    }

    void Library::CloseDB()
    {
        if (mDBConnection != nullptr)
            sqlite3_close(mDBConnection);
        Clear();
        Init();
    }

    //void Library::BuildCellLinks(bool del_dirty_links)
    //{
    //    for (auto cell : mCells)
    //    {
    //        for (size_t i = 0; i < cell->size(); i++)
    //        {
    //            auto element = cell->get(i);
    //            if (element->tag() == SREF)
    //            {
    //                auto temp = std::dynamic_pointer_cast<SRef>(element);
    //                std::string sname = temp->structName();
    //                auto source_cell = Get(sname);
    //                temp->set_reference(source_cell);
    //                source_cell->AddReferBy(cell);
    //            }
    //            else if (element->tag() == AREF)
    //            {
    //                auto temp = std::dynamic_pointer_cast<ARef>(element);
    //                std::string sname = temp->structName();
    //                auto source_cell = Get(sname);
    //                temp->set_reference(source_cell);
    //                source_cell->AddReferBy(cell);
    //            }
    //        }
    //    }
    //}
    //
    //void Library::CollectLayers(Techfile &tech_file)
    //{
    //    for (auto cell : mCells)
    //    {
    //        for (size_t i = 0; i < cell->size(); i++)
    //        {
    //            auto element = cell->get(i);
    //            switch (element->tag())
    //            {
    //            case BOUNDARY:
    //            {
    //                auto boundary = std::dynamic_pointer_cast<Boundary>(element);
    //                tech_file.AddLayer(boundary->layer(), boundary->data_type());
    //                break;
    //            }
    //            case PATH:
    //            {
    //                auto path = std::dynamic_pointer_cast<Path>(element);
    //                tech_file.AddLayer(path->layer(), path->data_type());
    //                break;
    //            }
    //            case TEXT:
    //            {
    //                auto text = std::dynamic_pointer_cast<Text>(element);
    //                tech_file.AddLayer(text->layer(), text->text_type());
    //                break;
    //            }
    //            default:
    //                break;
    //            }
    //        }
    //    }
    //}
    //
    //int Library::read(std::ifstream &in, std::string &msg)
    //{
    //    msg = "";
    //    Init();
    //    // read HEADER
    //    short record_size;
    //    Byte record_type, record_dt;
    //    if (!readShort(in, record_size)
    //        || !readByte(in, record_type)
    //        || !readByte(in, record_dt))
    //        return FILE_ERROR;
    //    if (record_type != HEADER)
    //    {
    //        std::stringstream ss;
    //        ss << "Unexpected tag where HEADER are expected (";
    //        ss << std::hex << record_size << record_type << record_dt;
    //        ss << ").";
    //        msg = ss.str();
    //        return FORMAT_ERROR;
    //    }
    //    if (record_size != 6)
    //    {
    //        std::stringstream ss;
    //        ss << "wrong record size of HEADER (";
    //        ss << std::hex << record_size << record_type << record_dt;
    //        ss << ").";
    //        msg = ss.str();
    //        return FORMAT_ERROR;
    //    }
    //    if (!readShort(in, mVersion))
    //        return FILE_ERROR;
    //
    //    // read BGNLIB
    //    if (!readShort(in, record_size)
    //        || !readByte(in, record_type)
    //        || !readByte(in, record_dt))
    //        return FILE_ERROR;
    //    if (record_type != BGNLIB)
    //    {
    //        std::stringstream ss;
    //        ss << "Unexpected tag where BGNLIB are expected (";
    //        ss << std::hex << record_size << record_type << record_dt;
    //        ss << ").";
    //        msg = ss.str();
    //        return FORMAT_ERROR;
    //    }
    //    if (record_size != 28)
    //    {
    //        std::stringstream ss;
    //        ss << "Wrong record size of BGNLIB (";
    //        ss << std::hex << record_size << record_type << record_dt;
    //        ss << ").";
    //        msg = ss.str();
    //        return FORMAT_ERROR;
    //    }
    //
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
    //    while (1)
    //    {
    //        if (!readShort(in, record_size)
    //            || !readByte(in, record_type)
    //            || !readByte(in, record_dt))
    //            return FILE_ERROR;
    //
    //        bool finished = false;
    //        switch (record_type)
    //        {
    //        case ENDLIB:
    //            finished = true;
    //            break;
    //        case LIBNAME:
    //            if (record_size < 4 || record_size % 2 != 0)
    //            {
    //                std::stringstream ss;
    //                ss << "Wrong record size of " + Record_name[record_type] + " (";
    //                ss << std::hex << record_size << record_type << record_dt;
    //                ss << ").";
    //                msg = ss.str();
    //                return FORMAT_ERROR;
    //            }
    //            if (!readString(in, record_size - 4, mLibName))
    //                return FILE_ERROR;
    //            break;
    //        case UNITS:
    //            if (record_size != 20)
    //            {
    //                std::stringstream ss;
    //                ss << "Wrong record size of UNITS (";
    //                ss << std::hex << record_size << record_type << record_dt;
    //                ss << ").";
    //                msg = ss.str();
    //                return FORMAT_ERROR;
    //            }
    //            if (!readDouble(in, mDBUnitInUserUnit)
    //                || !readDouble(in, mDBUnitInMeter))
    //                return FILE_ERROR;
    //            break;
    //        case BGNSTR:
    //        {
    //            if (record_size != 28)
    //            {
    //                std::stringstream ss;
    //                ss << "Wrong record size of BGNSTR (";
    //                ss << std::hex << record_size << record_type << record_dt;
    //                ss << ").";
    //                msg = ss.str();
    //                return FORMAT_ERROR;
    //            }
    //            std::shared_ptr<Structure> node = std::make_shared<Structure>();
    //            int error_code = node->read(in, msg);
    //            if (error_code > 0)
    //                return error_code;
    //            mCells.push_back(node);
    //            break;
    //        }
    //        default:
    //            break;
    //        }
    //        if (finished)
    //            break;
    //    }
    //
    //    return 0;
    //}
    //
    //int Library::write(std::ofstream &out, std::string &msg)
    //{
    //    short record_size;
    //
    //    record_size = 6;
    //    writeShort(out, record_size);
    //    writeByte(out, HEADER);
    //    writeByte(out, Integer_2);
    //    writeShort(out, mVersion);
    //
    //    record_size = 28;
    //    writeShort(out, record_size);
    //    writeByte(out, BGNLIB);
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
    //    record_size = 4 + short(mLibName.size());
    //    if (record_size % 2 != 0)
    //        record_size += 1;
    //    writeShort(out, record_size);
    //    writeByte(out, LIBNAME);
    //    writeByte(out, String);
    //    writeString(out, mLibName);
    //
    //    record_size = 20;
    //    writeShort(out, record_size);
    //    writeByte(out, UNITS);
    //    writeByte(out, Real_8);
    //    writeDouble(out, mDBUnitInUserUnit);
    //    writeDouble(out, mDBUnitInMeter);
    //
    //    for (auto e : mCells)
    //    {
    //        int err_code = e->write(out, msg);
    //        if (err_code > 0)
    //            return err_code;
    //    }
    //
    //    record_size = 4;
    //    writeShort(out, record_size);
    //    writeByte(out, ENDLIB);
    //    writeByte(out, NoData);
    //
    //    return out.fail() ? FILE_ERROR : 0;
    //}

}


