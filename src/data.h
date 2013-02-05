
#pragma once
#include <string>

#ifndef SSHLPRD_SOCKPATH
#define SSHLPRD_SOCKPATH "/tmp/sshlprd.sock"
#endif

std::string readstring(int);
int readint(int);


void writestring(int, const std::string &);
void writeint(int, int);
