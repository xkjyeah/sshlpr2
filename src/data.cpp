#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <string>
#include "data.h"
#include <map>
#include <sstream>
#include <iostream>

using namespace std;

string readstring(int fd) {
	int result;
	size_t dtlen;
	
	result = read(fd, &dtlen, sizeof(size_t));
	
	if (result == -1) {
		throw string("Unable to read string");
	}
	
	char *buf = new char[dtlen + 1];
	result = read(fd, buf, dtlen);
	
	if (result == -1) {
		delete buf;
		throw string("Unable to read string");
	}
	
	buf[dtlen] = 0;

	string param = buf;
	delete buf;

	return param;
}

void writestring(int fd, const std::string &data) {
	
	size_t len = data.length();
	int result;
	
	result = write(fd, &len, sizeof(size_t));
	
	if (result == -1) {
		throw string("Unable to write string");
	}
	result = write(fd, data.c_str(), len);
	if (result == -1) {
		throw string("Unable to write string");
	}
	
}

int readint(int fd) {
	int result, datum;
	
	result = read(fd, &datum, sizeof(int));
	
	if (result == -1) {
		throw string("Unable to read int");
	}
	
	return datum;
}

void writeint(int fd, int datum) {
	int result;
	
	result = write(fd, &datum, sizeof(int));
	
	if (result == -1) {
		throw string("Unable to write int");
	}
}
