/*
This file is part of sshlpr2.

    sshlpr2 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with sshlpr2.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#include "config.h"
#include <string>

#ifndef SSHLPRD_SOCKPATH
#define SSHLPRD_SOCKPATH "/tmp/sshlprd.sock"
#endif

std::string readstring(int);
int readint(int);


void writestring(int, const std::string &);
void writeint(int, int);
