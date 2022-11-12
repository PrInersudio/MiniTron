#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <linux/if_link.h>
#include <string.h>
#define ip_size 16
#include "udp_p2p.h"
int tie = 0;

static int Socket (int domain, int type, int protocol)
{
	int res = socket(domain, type, protocol);
	if (res == -1)
	{
		perror("Socket error");
		tie = 1;
		exit(EXIT_FAILURE);
	}
	return res;
}

static int Bind (int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int res = bind(sockfd, addr, addrlen);
	if (res == -1)
	{
		perror("Bind error");
		tie = 1;
		exit(EXIT_FAILURE);
	}
	return res;
}

static int Inet_aton(const char *cp, struct in_addr *inp, char* host_ip)
{
	int res = inet_aton(cp, inp);
	if (res == 0)
	{
		free(host_ip);
		perror("Inet_aton error");
		tie = 1;
		exit(EXIT_FAILURE);
	}
	return res;
}

static void Connect (int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int res = connect(sockfd, addr, addrlen);
	if (res == -1)
	{
		perror("Connect error");
		tie = 1;
		exit(EXIT_FAILURE);
	}
}

static int Getifaddrs(struct ifaddrs **ifap)
{
	int res = getifaddrs(ifap);
	if (res == -1)
	{
		perror("Getifaddrs error");
		tie = 1;
		exit(EXIT_FAILURE);
	}
	return res;
}

static int Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, size_t hostlen, char *serv, size_t servlen, int flags)
{
	int res = getnameinfo(sa, salen, host, hostlen, serv, servlen, flags);
	if (res != 0)
	{
		perror("Getifaddrs error");
		tie = 1;
		exit(EXIT_FAILURE);
	}
	return res;
}

static int SetAddress(char* ip_port, struct sockaddr_in *addr, char* host_ip)
{
	int i;
	char IPaddr[ip_size];
	addr->sin_family = AF_INET;
	for (i = 0; ip_port[i] != ':' && ip_port[i] != '\0' && i < ip_size; i++)
		IPaddr[i] = ip_port[i];
	IPaddr[i] = '\0';
	if (ip_port[i] == ':')
		addr->sin_port = htons(atoi(ip_port+i+1));
	else
	{
		free(host_ip);
		fprintf(stderr, "Неправильный порт\n");
		tie = 1;
		exit(-2);
	}
	if (i != 0)
		 Inet_aton(IPaddr, &addr->sin_addr, host_ip);
	else
	{
		free(host_ip);
		fprintf(stderr, "Неправильный IP\n");
		tie = 1;
		exit(-3);
	}
	return (strlen(ip_port)-i);
}

static char* GetHostIP ()
{
	struct ifaddrs *ifaddr;
	int family, s;
	char host[NI_MAXHOST];
	char* host_ip = (char*)malloc(ip_size*sizeof(char));
	memset(host_ip, 0, ip_size*sizeof(char));
	Getifaddrs(&ifaddr);
	for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;
		if (family == AF_INET)
		{
			Getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (host != "127.0.0.1")
				strcpy(host_ip, host);

		}
	}
	freeifaddrs(ifaddr);
	return host_ip;
}

struct connection_data InternetConnection (char* rival_ip)
{
	char* host_ip = GetHostIP();
	struct connection_data con_data = {0};
	con_data.sd = Socket(PF_INET, SOCK_DGRAM, 0);
	int port_size = SetAddress(rival_ip, &con_data.addr_rival, host_ip);
	int old_ip_size = strlen(host_ip);
	host_ip = realloc(host_ip, old_ip_size + port_size);
	for (int i = 0; i<=port_size; i++)
		host_ip[i+old_ip_size] = rival_ip[strlen(rival_ip)-port_size+i];
	SetAddress(host_ip, &con_data.addr_host, host_ip);
	free(host_ip);
	Bind(con_data.sd, (struct sockaddr *)&con_data.addr_host, sizeof(con_data.addr_host));
	Connect(con_data.sd, (struct sockaddr *)&con_data.addr_rival, sizeof(con_data.addr_rival));
	return con_data;
}
