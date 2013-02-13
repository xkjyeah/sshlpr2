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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <string>
#include <cstdlib>
#include <csignal>
#include <map>
#include "data.h"
#include <sstream>
#include <iostream>

using namespace std;
/*

main loop:
	receive request to map user to X server
	map user to X server

	*/

map<string, int> clients;
int listen_socket = 0;

void on_int(int c) {
	if (listen_socket)
		close(listen_socket);
	unlink(SSHLPRD_SOCKPATH);
	exit(1);
}

void drop_privileges() {
	if (geteuid() == 0) {
		struct passwd *lpuser = getpwnam(LPUSER);
		cerr << "Running as user " << LPUSER << " (" << lpuser->pw_uid << ")" << endl;
		setuid(lpuser->pw_uid);
	}
}

void test_socket() {

	struct stat sock_stats;

	if (stat(SSHLPRD_SOCKPATH, &sock_stats) == 0) { // file exists
		// test file to see if we can connect 
		int sock = socket(PF_UNIX, SOCK_STREAM, 0);

		struct sockaddr_un sock_addr;
		
		sock_addr.sun_family = AF_UNIX;
		strcpy( sock_addr.sun_path, SSHLPRD_SOCKPATH );

		if ( connect(sock, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr_un)) != -1) {
			cerr << "Daemon is already listening." << endl;
			close(sock);
			exit(1);
		}
		else {
			if (unlink( sock_addr.sun_path )) {
				cerr << "Failed to delete file. Do you have sufficient permissions?" << endl;
				exit(1);
			}
		}
	}
}

int main(int argc, char *argv[]) {
	drop_privileges();
	test_socket();
	signal(2, on_int);

	// create the unix sockets
	int result;
	int sock = socket(PF_UNIX, SOCK_STREAM, 0);
	string message;

	if (sock == -1) {
		cerr << "Exception thrown creating socket" << endl;
		return 1;
	}

	struct sockaddr_un sock_addr;
	
	sock_addr.sun_family = AF_UNIX;
	strcpy( sock_addr.sun_path, SSHLPRD_SOCKPATH );

	result = bind(sock, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr_un));
	if (result == -1) {
		cerr << "Unable to bind to pathname "
			<< SSHLPRD_SOCKPATH
			<< endl;
		return 1;
	}
	
	result = listen(sock, 10);
	if (result == -1) {
		cerr << "Unable to listen on socket "
			<< endl;
		return 1;
	}

	listen_socket = sock;
	
	{
		struct passwd *user_details = getpwuid(getuid());
		
		cout << "sshlprd started as user " << user_details->pw_name << endl;
		cout << "Listening on UNIX socket on "
			<< SSHLPRD_SOCKPATH
			<< endl;
	}

	int fd;
	socklen_t addrlen = sizeof(sockaddr_un);

	while ( -1 != (fd = accept(sock, (struct sockaddr*)&sock_addr, &addrlen)) ) {
		stringstream ss;
		size_t dtlen;

		// read in the action type
		int action = readint(fd);

		char *buf;
		
		
		string server, num_copies, lpr_options, fifo_path, user;

		switch (action) { // sshlpr -> sshlprd options
		case 1:
			cerr << "Print job received." << endl;
			try {
				server = readstring(fd);
				num_copies = readstring(fd);
				lpr_options = readstring(fd);
				fifo_path = readstring(fd);
				user = readstring(fd);
			} catch (string &message) {
				cerr << "Failed to read print job: " << message << endl;
				
				// attempt to send a reply
				try {
					writeint(fd, 5);
					writeint(fd, -1);
					writestring(fd, message);
				}catch (string &message) {}
				close(fd);
				
				break;
			}
			
			// forward message to client
			if (clients.find(user) != clients.end()) {
				int client_fd = clients[user];
				try {
					// send to client
					writeint(client_fd, 2);
					writestring(client_fd, server);
					writestring(client_fd, num_copies);
					writestring(client_fd, lpr_options);
					writestring(client_fd, fifo_path);
					writestring(client_fd, user);
					
					// read client's reply:
					int reply = readint(client_fd);
					int status = readint(client_fd);
					string message = readstring(client_fd);
					
					// reply to origin
					writeint(fd, 5);
					writeint(fd, status);
					writestring(fd, message);
				}
				catch(string &message) {
					try {
						writeint(fd, 5);
						writeint(fd, -2);
						writestring(fd, "User/sshlpr disconnected, or " + message);
						
						clients.erase(user);
						close(client_fd);
					}catch (string &message){}
				}
			}
			else {
				try {
					writeint(fd, 5);
					writeint(fd, -2);
					writestring(fd, "User not connected: " + user);
				}catch(string &message){}
			}
			break;
		case 2: // sent to client. not received by daemon;
			break;
		case 3: // not captured here
		case 4: // forwarded to client. not received by daemon;
		case 5: // reply messages to sshlpr
		case 6: //
			close(fd);
			break;
		case 7: // user details
			user = readstring(fd);
			clients[user] = fd;
			
			cout << "User connected: " << user << endl;
			break;
			
		default:
			close(fd);
			break;
		}
	}
	close(sock);
	unlink(SSHLPRD_SOCKPATH);
}
