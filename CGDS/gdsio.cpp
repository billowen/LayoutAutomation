/*
 * This file is part of GDSII.
 *
 * gdsio.cpp -- The source file which define the functions used to convert
 *            binary data from GDSII files.
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

#include <sstream>
#include <cmath>
#include <vector>
#include <cassert>
#include "gdsio.h"
#include "tags.h"
#include "sqlite/sqlite3.h"

const char *INFO_TABLE = "db_info_table";
const char *CELL_TABLE = "cell_table";
//const std::string DATA_COL_NAME = "DATA";
const char *GDS_VERSION_ID = "GDS_VERSION";
const char *MOD_TIME_ID = "MOD_TIME";
const char *ACC_TIME_ID = "ACC_TIME";
const char *LIB_NAME_ID = "LIB_NAME";
const char *UNITS_ID = "UNITS";
const char *DATA_COL_NAME = "DATA";

const std::string CREATE_DB_BASIC_INFO =
" \
DROP TABLE IF EXISTS db_info_table; \
DROP TABLE IF EXISTS cell_table; \
CREATE TABLE db_info_table ( ID TEXT NOT NULL, DATA BLOB NOT NULL); \
CREATE TABLE cell_table (ID TEXT NOT NULL, DATA BLOB);\
";
const char *GET_ROWID_TEMPLATE = "SELECT rowid FROM %s WHERE ID='%s';";
const char *UPDATE_LIB_NAME_SIZE = "UPDATE db_info_table SET DATA=zeroblob(%d) WHERE ID='LIB_NAME';";



void GDS::Encode(short in, char* out)
{
    char high, low;
    high = (in & 0xff00) >> 8;
    low = low & 0x00ff;
    out[0] = high;
    out[1] = low;
}

void GDS::Decode(char *in, short &out)
{
    out = in[0] << 8 | in[1];
}

void GDS::Encode(int in, char* out)
{
    out[3] = in & 0xff;
    out[2] = (in >> 8) & 0xff;
    out[1] = (in >> 16) & 0xff;
    out[0] = (in >> 24) & 0xff;
}

void GDS::Decode(char *in, int &out)
{
    out = in[3] + (in[2] << 8) + (in[1] << 16) + (in[0] << 24);
}

void GDS::Encode(double in, char *out)
{
    if (in == 0)
    {
        for (int i = 0; i < 8; i++)
        {
            out[i] = 0;
        }
    }
    else
    {
        double mantissa = in >= 0 ? in : -in;
        short exponent = 0;
        while (mantissa >= 1 || mantissa < (1.0 / 16))
        {
            if (mantissa >= 1)
            {
                mantissa /= 16;
                exponent++;
            }
            else
            {
                mantissa *= 16;
                exponent--;
            }
        }
        exponent += 64;
        out[0] = exponent;
        if (in < 0)
            out[0] = out[0] | 0x80;
        for (int j = 1; j < 8; j++)
        {
            mantissa *= 256;
            out[j] = mantissa;
            mantissa = mantissa - int(mantissa);

        }
    }
}

void GDS::Decode(char *in, double &out)
{
    short sign_flag = (in[0] & 0x80) ? -1 : 1;
    short exponent = (in[0] & 0x7f) - 64;
    long long mantissa = 0;
    for (int i = 1; i < 8; i++)
    {
        mantissa = mantissa << 8;
        mantissa = mantissa | in[i];
    }

    out = mantissa / std::pow(2.0, 56);
    if (exponent >= 0)
    {
        for (int j = 0; j < exponent; j++)
        {
            out = out * 16;
        }
    }
    else
    {
        for (int j = 0; j < -exponent; j++)
        {
            out = out / 16;
        }
    }
    out *= sign_flag;
}

bool GDS::readShort(std::ifstream &in, short &data)
{

    char buffer[2];
    in.read((char*)buffer, 2);
    if (in.fail())
        return false;
    
    data = buffer[0] << 8 | buffer[1];

    return true;
}

bool GDS::writeShort(std::ofstream &out, short data)
{
    char high, low;
    high = (data & 0xff00) >> 8;
    low = data & 0x00ff;
    out.write(&high, 1);
    out.write(&low, 1);

    return !out.fail();
}

bool GDS::readInteger(std::ifstream &in, int &data)
{
    char buffer[4];
    in.read((char*)buffer, 4);
    if (in.fail())
        return false;

    data = buffer[3]
            + (buffer[2] << 8)
            + (buffer[1] << 16)
            + (buffer[0] << 24);

    return true;
 }

bool GDS::writeInteger(std::ofstream &out, int data)
{
    char buffer[4];
    buffer[3] = data & 0xff;
    buffer[2] = (data >> 8) & 0xff;
    buffer[1] = (data >> 16) & 0xff;
    buffer[0] = (data >> 24) & 0xff;
    out.write((char *)buffer, 4);

    return !out.fail();
}

bool GDS::readString(std::ifstream &in, int size, std::string &data)
{
    data = "";
    for (int i = 0; i < size; i++)
    {
        char c;
        in.read(&c, 1);
        if (in.fail())
            return false;
        if (c != '\0')
            data.push_back(c);
    }
    return true;
}

bool GDS::writeString(std::ofstream &out, std::string data)
{
    out.write(data.c_str(), data.size());
    
    if (data.size() % 2 != 0)
    {
        char c = '\0';
        out.write(&c, 1);
    }

    return !out.fail();
}

bool GDS::readBitarray(std::ifstream &in, short &data)
{
    return readShort(in, data);
}

bool GDS::writeBitarray(std::ofstream &out, short data)
{
    return writeShort(out, data);
}

bool GDS::readDouble(std::ifstream &in, double &data)
{
    unsigned char buffer[8];
    in.read((char*)buffer, 8);
    if (in.fail())
        return false;

    short sign_flag = (buffer[0] & 0x80) ? -1 : 1;
    short exponent = (buffer[0] & 0x7f) - 64;
    long long mantissa = 0;
    for (int i = 1; i < 8; i++)
    {
        mantissa = mantissa << 8;
        //mantissa = mantissa | (0x00000000000000ff & buffer[i]);
        mantissa = mantissa | buffer[i];
    }

    data = mantissa / std::pow(2.0, 56);
    if (exponent >= 0)
    {
        for (int j = 0; j < exponent; j++)
        {
            data = data * 16;
        }
    }
    else
    {
        for (int j = 0; j < -exponent; j++)
        {
            data = data / 16;
        }
    }
    data = data * sign_flag;

    return true;
}

bool GDS::writeDouble(std::ofstream &out, double data)
{
    unsigned char buffer[8];
    if (data == 0)
    {
        for (int i = 0; i < 8; i++)
        {
            buffer[i] = 0;
        }
        out.write((char*)buffer, 8);
    }
    else
    {
        double mantissa = data >= 0 ? data : -data;
        short exponent = 0;
        while (mantissa >= 1 || mantissa < (1.0 / 16))
        {
            if (mantissa >= 1)
            {
                mantissa /= 16;
                exponent ++;
            }
            else
            {
                mantissa *= 16;
                exponent --;
            }
        }
        exponent += 64;
        buffer[0] = exponent;
        if (data < 0)
            buffer[0] = buffer[0] | 0x80;
        for (int j = 1; j < 8; j++)
        {
            mantissa *= 256;
            buffer[j] = mantissa;
            mantissa = mantissa - int(mantissa);

        }
        out.write((char*)buffer, 8);
    }

    return !out.fail();
}

bool GDS::readByte(std::ifstream &in, char &data)
{
    in.read((char*)&data, 1);
    return !in.fail();
}

bool GDS::writeByte(std::ofstream &out, char data)
{
    out.write((char*)&data, 1);
    return !out.fail();
}

std::string GDS::byteToString(char data)
{
    std::stringstream ss;
    ss << std::hex << data;
    return ss.str();
}

int InsertGDSData2DB(sqlite3 *db,
                     const std::string &table,
                     const std::string &ID,
                     char *data, int nSize,
                     std::string &err)
{
    int rc;
    sqlite3_stmt *stmt;
    char sql[100];
    sprintf(sql, "INSERT INTO %s VALUES(?,?)", table.c_str());
    rc = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
    if (rc != SQLITE_OK)
    {
        goto ERROR;
    }

    sqlite3_reset(stmt);
    rc = sqlite3_bind_text(stmt, 1, ID.c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        goto ERROR;
    }
    rc = sqlite3_bind_blob(stmt, 2, data, nSize, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK)
    {
        goto ERROR;
    }
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        goto ERROR;
    }

    sqlite3_finalize(stmt);
    return 0;

ERROR:
    char buffer[100];
    sprintf(buffer, "SQL error: failed to insert data of %s into %s.\n", ID.c_str(), table.c_str());
    err = buffer;
    sqlite3_finalize(stmt);
    return 1;
}

int GDS::ConvertGDSII2DB(std::string gdsName, std::string dbName, std::string &err)
{
    sqlite3 *db;
    int rc;
    char *zErrMsg = 0;

    rc = sqlite3_open(dbName.c_str(), &db);
    if (rc)
    {
        err = "Can't open database: " + std::string(sqlite3_errmsg(db));
        sqlite3_close(db);
        return DB_ERROR;
    }

    rc = sqlite3_exec(db, CREATE_DB_BASIC_INFO.c_str(), 0, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        err = "SQL error: " + std::string(zErrMsg) + "\n";
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return DB_ERROR;
    }

    std::ifstream infile;
    infile.open(gdsName, std::ios::binary);
    if (!infile.is_open())
    {
        err = "GDSII file error: failed to open the GDSII file.\n";
        sqlite3_close(db);
        return FILE_ERROR;
    }

    short record_size;
    char record_type, record_dt;

    if (!readShort(infile, record_size) ||
        !readByte(infile, record_type) ||
        !readByte(infile, record_dt))
    {
        err = "GDSII file error: failed to read excepted data from the GDSII file.\n";
        sqlite3_close(db);
        return FILE_ERROR;
    }
    if (record_type != HEADER)
    {
        err = "GDSII format error: unexpected tag where 'HEADER' is expected.\n";
        sqlite3_close(db);
        return FORMAT_ERROR;
    }
    if (record_size != 6)
    {
        err = "GDSII format error: incorrect record size of 'HEADER'.\n";
        sqlite3_close(db);
        return FORMAT_ERROR;
    }
   
    char buffer_2b[2];
    // VERSION
    infile.read(buffer_2b, 2);
    if (InsertGDSData2DB(db, INFO_TABLE, GDS_VERSION_ID, buffer_2b, 2, err))
    {
        sqlite3_close(db);
        return DB_ERROR;
    }
    // TIME INFO OF LIB
    if (!readShort(infile, record_size) ||
        !readByte(infile, record_type) ||
        !readByte(infile, record_dt))
    {
        err = "GDSII file error: failed to read excepted data from the GDSII file.\n";
        sqlite3_close(db);
        return FILE_ERROR;
    }
    if (record_type != BGNLIB)
    {
        err = "GDSII format error: unexpected tag where 'BGNLIB' is expected.\n";
        sqlite3_close(db);
        return FORMAT_ERROR;
    }
    if (record_size != 28)
    {
        err = "GDSII format error: incorrect record size of 'BGNLIB'.\n";
        sqlite3_close(db);
        return FORMAT_ERROR;
    }
    char buffer_12b[12];
    infile.read(buffer_12b, 12);
    if (InsertGDSData2DB(db, INFO_TABLE, MOD_TIME_ID, buffer_12b, 12, err))
    {
        sqlite3_close(db);
        return DB_ERROR;
    }
    infile.read(buffer_12b, 12);
    if (InsertGDSData2DB(db, INFO_TABLE, ACC_TIME_ID, buffer_12b, 12, err))
    {
        sqlite3_close(db);
        return DB_ERROR;
    }

    std::map<std::string, std::pair<unsigned long long, unsigned long long> > cache_map;
    while (true)
    {
        if (!readShort(infile, record_size) ||
            !readByte(infile, record_type) ||
            !readByte(infile, record_dt))
        {
            err = "GDSII file error: failed to read excepted data from the GDSII file.\n";
            sqlite3_close(db);
            return FILE_ERROR;
        }

        bool finished = false;
        switch (record_type)
        {
        case ENDLIB:
            finished = true;
            break;
        case LIBNAME:
        {
            if (record_size < 4 || record_size % 2 != 0)
            {
                err = "GDSII format error: incorrect record size of 'LIBNAME'.\n";
                sqlite3_close(db);
                return FORMAT_ERROR;
            }
            char *buffer_str = new char[record_size - 4];
            infile.read(buffer_str, record_size - 4);
            if (InsertGDSData2DB(db, INFO_TABLE, LIB_NAME_ID, buffer_str, record_size - 4, err))
            {
                sqlite3_close(db);
                delete[] buffer_str;
                return DB_ERROR;
            }
            delete[] buffer_str;
            break;
        }
        case UNITS:
        {
            if (record_size != 20)
            {
                err = "GDSII format error: incorrect record size of 'UNITS'.\n";
                sqlite3_close(db);
                return FORMAT_ERROR;
            }
            char buffer[16];
            infile.read(buffer, 16);
            if (InsertGDSData2DB(db, INFO_TABLE, UNITS_ID, buffer, 16, err))
            {
                sqlite3_close(db);
                return DB_ERROR;
            }
            break;
        }
        case BGNSTR:
        {
            infile.seekg(-4, std::ios_base::cur);
            unsigned long long start_pos = infile.tellg();
            unsigned long long end_pos = start_pos;
            std::string name;
            while (1)
            {
                if (!readShort(infile, record_size) ||
                    !readByte(infile, record_type) ||
                    !readByte(infile, record_dt))
                {
                    err = "GDSII file error: failed to read excepted data from the GDSII file.\n";
                    sqlite3_close(db);
                    return FILE_ERROR;
                }
                if (record_type == STRNAME)
                {

                    if (!readString(infile, record_size - 4, name))
                    {
                        err = "GDSII file error: failed to read excepted data from the GDSII file.\n";
                        sqlite3_close(db);
                        return FILE_ERROR;
                    }
                }
                else if (record_type == ENDSTR)
                {
                    end_pos = infile.tellg();
                    break;
                }
                else
                {
                    infile.seekg(record_size - 4, std::ios_base::cur);
                }
            }
            assert(start_pos != end_pos);
            cache_map[name] = std::make_pair(start_pos, end_pos);

            break;
        }
        default:
            break;
        }

        if (finished)
            break;
    }

    // Write cell data into database;
    sqlite3_exec(db, "begin;", 0, 0, 0);
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO cell_table VALUES(?,?)";
    rc = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        err = "SQL error: failed to add cell data into cell_table.\n";
        return DB_ERROR;
    }
    for (auto &e : cache_map)
    {
        sqlite3_reset(stmt);
        rc = sqlite3_bind_text(stmt, 1, e.first.c_str(), -1, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            err = "SQL error: failed to add cell data into cell_table.\n";
            return DB_ERROR;
        }
        infile.seekg(e.second.first, std::ios_base::beg);
        char *buffer = new char[e.second.second - e.second.first];
        infile.read(buffer, e.second.second - e.second.first);
        rc = sqlite3_bind_blob64(stmt, 2, buffer, e.second.second - e.second.first, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK)
        {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            err = "SQL error: failed to add cell data into cell_table.\n";
            return DB_ERROR;
        }
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            delete[] buffer;
            sqlite3_close(db);
            err = "SQL error: failed to add cell data into cell_table.\n";
            return DB_ERROR;
        }
        delete[]buffer;
    }
    sqlite3_finalize(stmt);
    sqlite3_exec(db, "commit;", 0, 0, 0);

    sqlite3_close(db);

    return 0;
}

