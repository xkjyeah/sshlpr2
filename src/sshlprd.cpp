#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <string>
#include <map>
#include <sstream>
#include <iostream>


#ifndef SSHLPRD_SOCKPATH
#define SSHLPRD_SOCKPATH "/tmp/sshlprd.sock"
#endif

using namespace std;
/*

main loop:
	receive request to map user to X server
	map user to X server

	*/

map<string, string> user_xserver;

int main(int argc, char *argv[]) {
	for (int i=1; i<argc; i++) {
		if (!strcmp(argv[i], "-f")) {
			unlink(SSHLPRD_SOCKPATH);
		}
	}

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

	int fd;
	socklen_t addrlen = sizeof(sockaddr_un);

	while ( -1 != (fd = accept(sock, (struct sockaddr*)&sock_addr, &addrlen)) ) {
		stringstream ss;
		size_t dtlen;

		// read in the action type
		int action;
		result = read(fd, &action, sizeof(int));

		char *buf;

		if (action == 1) {
			result = read(fd, &dtlen, sizeof(size_t));
			buf = new char[dtlen + 1];
			result = read(fd, buf, dtlen);
			buf[dtlen] = 0;

			string param = buf;
			delete buf;

			result = read(fd, &dtlen, sizeof(size_t));
			buf = new char[dtlen + 1];
			result = read(fd, buf, dtlen);
			buf[dtlen] = 0;

			string value = buf;
			delete buf;

			user_xserver[param] = value;

			cout << "Received input. User: "
				<< param
				<< " Display: "
				<< value
				<< endl;
		}
		else if (action == 2) {
			result = read(fd, &dtlen, sizeof(size_t));
			buf = new char[dtlen];
			result = read(fd, buf, dtlen);
			buf[dtlen] = 0;

			string param = buf;
			delete buf;

			cout << "Query requested for: "
				<< param
				<< endl;
			
			if (user_xserver.find(param) == user_xserver.end()) {
				write(fd, (size_t)0, sizeof(size_t));
				write(fd, "", 1);
			}
			else {
				dtlen = user_xserver[param].length();
				write(fd, &dtlen, sizeof(size_t));
				write(fd, user_xserver[param].c_str(),
					user_xserver[param].length());
			}
		}
		else {
			cerr << "Unknown action number " << action << endl;
		}
		close(fd);
	}
	cout << errno << endl;
	close(sock);
}
