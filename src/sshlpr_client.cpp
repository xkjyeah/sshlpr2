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
#include <pwd.h>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <map>
#include "data.h"
#include <sstream>
#include <iostream>
#include <fcntl.h>
#include <csignal>
#include <errno.h>

using namespace std;

int conn_sock = 0;

char default_helper[] = "sshlpr_helper";
char *helper = default_helper;

void on_int(int c) {
	if (conn_sock)
		close(conn_sock);
	unlink(SSHLPRD_SOCKPATH);
	exit(1);
}

// returns the input fd of the spawned process
void start_helper(string &server, string &num_copies, string &lpr_options, string &fifo_path, string &user, int lp_fifo_fd ) {

	// drop euid back to real user's id
	// start the helper
	
	int usr_id = getuid();	
	
	int lp_uid = geteuid();
	int usr_uid = getuid();
	
	setuid(usr_uid); // call the process and set up fifos as the real user

	// make a fifo -- as the real user, so that the FIFO can be read by his process
	char *fifoname;
	for (int i=0; i<5; i++) {
		fifoname = tempnam(NULL, "lpf");
	
		if (fifoname == NULL) {
			cerr << "ERROR: Could not generate FIFO" << endl;
			exit(1);
		}
	
		if (! mkfifo( fifoname, 0644) )
			break;
	
		free(fifoname);
	}
	
	// restore to privileged uid
	setuid(lp_uid);
	cerr << "LP UID: " << getuid() << endl;
	
	// fork the process
	int pid = fork();
	
	if (pid == -1) {
		cerr << "Error spawning helper process" << endl;
		exit(1);
	}
	
	cerr << "pid " << pid << endl;
	if (pid == 0) { // child process
		setuid(usr_uid); // call the process and set up fifos as the real user
		
		
		execlp(helper, "sshlpr_helper", server.c_str(), num_copies.c_str(), lpr_options.c_str(), fifoname, user.c_str(), NULL);
		
		cerr << "exec failed: " << errno << endl;
		
		exit(1);
	}

	// cannot guarantee that user's process will open FIFO.
	// in which case the following call will block
	// so we start another process which waits until we're happy.
	
	pid = fork();
	
	if (pid == -1) {
		cerr << "Error spawning helper process" << endl;
		exit(1);
	}
	
	if (pid == 0) {
		// open FIFO as real user
		setuid(usr_uid);
		int fifo_fd = open(fifoname, O_WRONLY);
	
		// parent process
		cerr << "Opened fifo " << fifoname << endl;	
		
		int result = 0;
		setuid(lp_uid);
		
		char buf[4096];
		while ( result = read( lp_fifo_fd, buf, 4096 ) ) {
			int len;
			
			if (result < 0) {
				cerr << "Error while reading to pipe: " << errno << endl;
				result = 1;
				break;
			}
			
			result = write( fifo_fd, buf, result );
			
			if (result < 0) {
				cerr << "Error while writing to pipe: " << errno << endl;
				result = 1;
				break;
			}
		}
		
		cerr << "Finished print job " << (result?"with errors": "") << endl;

		close(0);
		close(1);
		close(2);
		close(lp_fifo_fd);
		close(fifo_fd);
		unlink(fifo_path.c_str());
		unlink(fifoname);
			
		exit(result);
	}
	
	// parent process
	close(lp_fifo_fd);
	return; 
}

int main(int argc, char* argv[]) {

	// create the unix sockets
	int result;
	int sock = socket(PF_UNIX, SOCK_STREAM, 0);
	string message;
	
	if (argc >= 2) {
		helper = argv[argc-1];
	}
	else {
		char *env_helper = getenv("SSHLPR_HELPER");
		if ( env_helper ) helper = env_helper;
	}

	if (sock == -1)
	{
		cerr << "Exception thrown creating socket" << endl;
		return 1;
	}

	struct sockaddr_un sock_addr;
	
	sock_addr.sun_family = AF_UNIX;
	strcpy( sock_addr.sun_path, SSHLPRD_SOCKPATH );

	result = connect(sock, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr_un));
	if (result == -1) {
		cerr << "Unable to connect to pathname "
			<< SSHLPRD_SOCKPATH
			<< endl;
		return 1;
	}

	conn_sock = sock;

	int fd;
	socklen_t addrlen;

	stringstream ss;
	struct passwd *user_details = getpwuid(getuid());
	char blank = 0;

	if (user_details == NULL) {
		cerr << "Unable to discover user details" << endl;
		return 1;
	}
	
	size_t dtlen;
	int action;
	
	writeint(sock, 7);
	writestring(sock, user_details->pw_name); // identify ourselves
	
	while ( action = readint(sock) ) {
	
		cerr << "action" << action << endl;
		string server, num_copies, lpr_options, fifo_path, user;
		int fifo_fd;
		
		if (action == 2) {
			server = readstring(sock);
			num_copies = readstring(sock);
			lpr_options = readstring(sock);
			fifo_path = readstring(sock);
			user = readstring(sock);
			
			// print to shell to be managed by script
			cout << server << endl
				<< num_copies << endl
				<< lpr_options << endl
				<< fifo_path << endl
				<< user << endl;
				
			writeint(sock, 6);
			writeint(sock, 0);
			writestring(sock, "");

			
			fifo_fd = open(fifo_path.c_str(), O_RDONLY);
			
			start_helper( server, num_copies, lpr_options, fifo_path, user, fifo_fd );
		}
		else {
			cerr << "Unknown action received: " << action << endl;
			return -1;
		}
	}

	close(sock);
}
