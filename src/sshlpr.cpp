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

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstdio>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <map>
#include "data.h"
#include <sstream>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {

	// translation of sshlpr.sh into C
	if (argc <= 1) {
		cout << "network sshlpr \"Unknown\" \"LPR thorugh SSH\"" << endl;
		return 0;
	}
	
	if (argc < 6 || argc > 7) {
		cerr << "Please refer to CUPS manual on how to use backends." << endl;
		return -1;
	}
	
	char *user = strdup(argv[2]);
	char *localuser = user;
	
	if (getenv("DEVICE_URI") == NULL)
		return 1;
	
	char *device_uri = strdup(getenv("DEVICE_URI"));
	
	strtok(device_uri, "/");
	strtok(NULL, "/");
	char *server = strtok(NULL, "/");
	char *printer = strtok(NULL, "/");
	
	if (strstr(server, "@") != NULL) {
		user = strtok(server, "@");
		server = strtok(NULL, "@");
	}

	if (strstr(user, ":") != NULL) {
		localuser = strtok(user, ":");
		user = strtok(NULL, ":");
	}
	
	// create the unix sockets
	int result;
	int sock = socket(PF_UNIX, SOCK_STREAM, 0);
	string message;

	if (sock == -1)
	{
		cerr << "ERROR: Exception thrown creating socket" << endl;
		return 1;
	}

	struct sockaddr_un sock_addr;
	
	sock_addr.sun_family = AF_UNIX;
	strcpy( sock_addr.sun_path, SSHLPRD_SOCKPATH );

	result = connect(sock, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr_un));
	if (result == -1) {
		cerr << "ERROR: Unable to connect to pathname "
			<< SSHLPRD_SOCKPATH
			<< endl;
		return 1;
	}

	int fd;
	socklen_t addrlen;

	stringstream ss;
	struct passwd *user_details = getpwuid(getuid());
	char blank = 0;

	if (user_details == NULL) {
		cerr << "ERROR: Unable to discover user details" << endl;
		return 1;
	}
	
	size_t dtlen;
	int action;
	
	writeint(sock, 1);
	writestring(sock, getenv("DEVICE_URI"));
	writestring(sock, argv[4]); // num copies
	writestring(sock, argv[5]); // options
	
	char *fifoname;
	
	for (int i=0; i<5; i++) {
		fifoname = tempnam(NULL, "sshlp");
		
		if (fifoname == NULL) {
			cerr << "ERROR: Could not generate FIFO" << endl;
			return 1;
		}
		
		if (! mkfifo( fifoname, 0644) )
			break;
		
		free(fifoname);
	}
	
	writestring(sock, fifoname);
	writestring(sock, user);
	
	cerr << "INFO: Job details sent" << endl;
	
	// read acknowledgment
	if ( readint(sock) != 5) {
		cerr << "ERROR: Protocol error" << endl;
		return 1;
	}
	{
		int err = readint(sock);
		string msg = readstring(sock);
		
		if (err) {
			cerr << "ERROR: " << msg << endl;
			return 1;
		}
	}

	cerr << "INFO: Acknowledgment received" << endl;
	
	// dump contents into fifo
	int contents_fd = 0; // standard input;
	int fifo_fd = open(fifoname, O_WRONLY);
	int read_bytes;
	char buf[4096];
	
	if (argc == 7) {
		contents_fd = open(argv[6], O_RDONLY);
	}
	
	if (fifo_fd < 0 || contents_fd < 0) {
		cerr << "ERROR: Could not open FIFO (" << fifo_fd << ") "
			<< ", or could not read from file (" << contents_fd << "). " << endl;
		writeint(sock, -1); // inform socket that FIFO failed
		return 1;
	}
	
	while ( (read_bytes = read(contents_fd, buf, 4096)) > 0 ) {
//		writeint( fifo_fd, read_bytes );
		write( fifo_fd, buf, read_bytes );
	}

	if (argc == 7) {
		free(fifoname);
		close(contents_fd);
	}
	cerr << "INFO: Data dumped into FIFO" << endl;
	
	close(fifo_fd);
	
	close(sock);
	
	free(device_uri);
}
