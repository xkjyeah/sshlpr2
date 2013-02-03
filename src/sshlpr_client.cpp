#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <cstdlib>
#include <pwd.h>
#include <unistd.h>
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

int main() {

	// create the unix sockets
	int result;
	int sock = socket(PF_UNIX, SOCK_STREAM, 0);
	string message;

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

	int fd;
	socklen_t addrlen;

	stringstream ss;
	struct passwd *user_details = getpwuid(getuid());
	char blank = 0;
	char *xserver = getenv("DISPLAY");

	if (user_details == NULL) {
		cerr << "Unable to discover user details" << endl;
		return 1;
	}
	if (xserver == NULL) {
		cerr << "Warning: DISPLAY environment variable is not set. Mapping will be unset." << endl;
		xserver = &blank;
	}

	size_t dtlen;
	int action = 1;
	
	// include the null byte because it is our delimiter
	write(sock, &(action = 1), sizeof(int));
	write(sock, &(dtlen = strlen(user_details->pw_name)), sizeof(size_t));
	write(sock, user_details->pw_name, dtlen);
	write(sock, &(dtlen = strlen(xserver)), sizeof(size_t));
	write(sock, xserver, dtlen);

	// now write to the socket
	if (result == -1) {
		cerr << "Write to socket failed" << endl;
		return 1;
	}

	close(sock);
}
