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

int main(int argc, char*argv[]) {
	if (argc < 2) {
		cerr << "Usage: sshlpr_query <username>" << endl;
		return 1;
	}

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

	size_t len_read, total_len_read = 0;
	size_t dtlen;
	int action=2;
	
	// include the null byte because it is our delimiter
	write(sock, &(action = 2), sizeof(int));
	write(sock, &(dtlen=strlen(argv[1])), sizeof(size_t));
	write(sock, argv[1], dtlen);

	// read from socket
	stringstream rr;
	char *buf;
	
	read(sock, &dtlen, sizeof(size_t));
	buf = new char[dtlen + 1];
	read(sock, buf, dtlen);
	buf[dtlen] = 0;

	// my DISPLAY is now stored in ss
	cout << buf << endl;

	close(sock);
}
