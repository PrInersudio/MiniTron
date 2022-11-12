#ifndef UDP_P2P
#define UDP_P2P
struct connection_data
{
	struct sockaddr_in addr_host;
	struct sockaddr_in addr_rival;
	int sd;
};

struct connection_data InternetConnection (char* rival_ip);
#endif
