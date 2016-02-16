/*
 * This file is part of GDSII.
 *
 * gdsio.h -- The header file which declare the functions used to convert
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

#ifndef GDSIO_H
#define GDSIO_H
#include <fstream>
#include <string>
//#include "tags.h"

namespace GDS {

const int DB_ERROR = 1;
const int FILE_ERROR = 2;
const int FORMAT_ERROR = 3;

/*
 * 2-Byte Signed Integer    ---- short
 * 4-Byte Signed Integer    ---- int
 * 4-Byte Real              ---- float
 * 8-Byte Real              ---- double
 * ASCII String             ---- std::string
 * Bit Array                ---- short
 **/

bool readByte(std::ifstream &in, char &data);
bool readShort(std::ifstream &in, short &data);
bool readInteger(std::ifstream &in, int &data);
bool readDouble(std::ifstream &in, double &data);
bool readString(std::ifstream &in, int size, std::string &data);
bool readBitarray(std::ifstream &in, short &data);

bool writeByte(std::ofstream &out, char data);
bool writeShort(std::ofstream &out, short data);
bool writeInteger(std::ofstream &out, int data);
bool writeFloat(std::ofstream &out, float data);
bool writeDouble(std::ofstream &out, double data);
bool writeString(std::ofstream &out, std::string data);
bool writeBitarray(std::ofstream &out, short data);

std::string byteToString(char data);

/*!
 * Encode a number with Short type into 2-byte binary.
 * @param in The number.
 * @param out The binary code after encoding.
 */
void Encode(short in, char *output);
/*!
 * Encode a number with Int type into 4-byte binary.
 * @param in The number.
 * @param out The binary code after encoding.
 */
void Encode(int in, char *output);
/*!
 * Encode a number with Double type into 8-byte binary.
 * @param in The number.
 * @param out The binary code after encoding.
 */
void Encode(double in, char *output);

/*!
 * Decode a 2-byte binary into a short number.
 * @param in 2-byte binary code.
 * @param out The number.
 */
void Decode(char *in, short &out);
/*!
 * Decode a 4-byte binary into a int number.
 * @param in 4-byte binary code.
 * @param out The number.
 */
void Decode(char *in, int &out);
/*!
 * Decode a 8-byte binary into a double number.
 * @param in 8-byte binary code.
 * @param out The number.
 */
void Decode(char *in, double &out);

int ConvertGDSII2DB(std::string gdsName, std::string dbName, std::string &err);




}

#endif // GDSIO_H

