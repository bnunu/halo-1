/*
WIN32_NET.C

The socket half of port/linux/src/posix.h for Windows (the Linux version is
posix_net.c). The Xbox's Winsock and Windows' share option numbers and error
codes, so this passes nearly everything straight through.
*/

/* select() takes as many sockets as the Linux version (FD_SETSIZE) */
#define FD_SETSIZE 1024

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <bcrypt.h>
#include <stdlib.h>
#include <string.h>

#include "posix.h"

/* mstcpip.h has them only in some Windows SDKs */
#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#endif
#ifndef SIO_UDP_NETRESET
#define SIO_UDP_NETRESET _WSAIOW(IOC_VENDOR, 15)
#endif

static __thread int last_error;

static int fail(void)
{
	last_error = WSAGetLastError();
	return -1;
}

static int succeed(int result)
{
	if (result == SOCKET_ERROR)
		return fail();
	last_error = 0;
	return result;
}

/* sockets are small integers on Windows too; the Xbox's are 32-bit */
static int from_socket(SOCKET socket)
{
	return socket == INVALID_SOCKET ? -1 : (int)socket;
}

static void start_winsock(void)
{
	static LONG started;

	if (!InterlockedCompareExchange(&started, 1, 0))
	{
		WSADATA data;

		WSAStartup(MAKEWORD(2, 2), &data);
	}
}

int posix_socket_last_error(void)
{
	return last_error;
}

int posix_socket(int family, int type, int protocol)
{
	SOCKET result;

	start_winsock();
	result = WSASocketW(family, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT);
	if (result == INVALID_SOCKET)
		return fail();
	if (type == SOCK_DGRAM)
	{
		/* Windows reports an ICMP port unreachable (a datagram to an address
		nothing listens on, such as a network.broadcast machine not running)
		as a WSAECONNRESET from the socket's next recvfrom, and a time
		exceeded as a WSAENETRESET; neither the Xbox nor Linux does, and the
		game takes both as its connection lost */
		BOOL report = FALSE;
		DWORD returned = 0;

		WSAIoctl(result, SIO_UDP_CONNRESET, &report, sizeof(report), NULL, 0, &returned, NULL, NULL);
		WSAIoctl(result, SIO_UDP_NETRESET, &report, sizeof(report), NULL, 0, &returned, NULL, NULL);
	}
	last_error = 0;
	return from_socket(result);
}

int posix_socket_close(int socket)
{
	return succeed(closesocket((SOCKET)socket));
}

int posix_socket_bind(int socket, const void *address, int address_length)
{
	return succeed(bind((SOCKET)socket, address, address_length));
}

int posix_socket_connect(int socket, const void *address, int address_length)
{
	return succeed(connect((SOCKET)socket, address, address_length));
}

int posix_socket_listen(int socket, int backlog)
{
	return succeed(listen((SOCKET)socket, backlog));
}

int posix_socket_accept(int socket, void *address, int *address_length)
{
	SOCKET result = accept((SOCKET)socket, address, address_length);

	if (result == INVALID_SOCKET)
		return fail();
	last_error = 0;
	return from_socket(result);
}

int posix_socket_send(int socket, const void *buffer, int length, int flags)
{
	return succeed(send((SOCKET)socket, buffer, length, flags));
}

int posix_socket_sendto(int socket, const void *buffer, int length, int flags,
	const void *address, int address_length)
{
	return succeed(sendto((SOCKET)socket, buffer, length, flags, address, address_length));
}

int posix_socket_recv(int socket, void *buffer, int length, int flags)
{
	return succeed(recv((SOCKET)socket, buffer, length, flags));
}

int posix_socket_recvfrom(int socket, void *buffer, int length, int flags,
	void *address, int *address_length)
{
	return succeed(recvfrom((SOCKET)socket, buffer, length, flags, address, address_length));
}

int posix_socket_shutdown(int socket, int how)
{
	return succeed(shutdown((SOCKET)socket, how));
}

int posix_socket_set_nonblocking(int socket, int nonblocking)
{
	u_long value = nonblocking ? 1 : 0;

	return succeed(ioctlsocket((SOCKET)socket, FIONBIO, &value));
}

int posix_socket_set_nodelay(int socket)
{
	BOOL value = TRUE;

	return succeed(setsockopt((SOCKET)socket, IPPROTO_TCP, TCP_NODELAY, (const char *)&value, sizeof(value)));
}

int posix_socket_bytes_available(int socket, posix_ulong *count)
{
	u_long available = 0;
	int result = ioctlsocket((SOCKET)socket, FIONREAD, &available);

	if (result != SOCKET_ERROR)
		*count = (posix_ulong)available;
	return succeed(result);
}

/* SOL_SOCKET options Windows knows; the Xbox's own (SO_ENCRYPT and the
like) have nothing to do here */
static int known_option(int level, int name)
{
	if (level != SOL_SOCKET)
		return 1;
	switch (name)
	{
	case SO_REUSEADDR:
	case SO_KEEPALIVE:
	case SO_BROADCAST:
	case SO_LINGER:
	case SO_SNDBUF:
	case SO_RCVBUF:
	case SO_ERROR:
	case SO_TYPE:
		return 1;
	default:
		return 0;
	}
}

int posix_socket_setsockopt(int socket, int level, int name, const void *value, int length)
{
	if (!known_option(level, name))
	{
		last_error = 0;
		return 0;
	}
	return succeed(setsockopt((SOCKET)socket, level, name, value, length));
}

int posix_socket_getsockopt(int socket, int level, int name, void *value, int *length)
{
	if (!known_option(level, name))
	{
		last_error = WSAENOPROTOOPT;
		return -1;
	}
	return succeed(getsockopt((SOCKET)socket, level, name, value, length));
}

int posix_socket_getsockname(int socket, void *address, int *address_length)
{
	return succeed(getsockname((SOCKET)socket, address, address_length));
}

int posix_socket_getpeername(int socket, void *address, int *address_length)
{
	return succeed(getpeername((SOCKET)socket, address, address_length));
}

static int fill_set(fd_set *set, const int *descriptors, int count)
{
	int index, filled = 0;

	FD_ZERO(set);
	for (index = 0; index < count; index++)
	{
		if (descriptors[index] >= 0 && set->fd_count < FD_SETSIZE)
		{
			FD_SET((SOCKET)descriptors[index], set);
			filled++;
		}
	}
	return filled;
}

static void keep_ready(fd_set *set, int *descriptors, int *count)
{
	int index, kept = 0;

	for (index = 0; index < *count; index++)
	{
		if (descriptors[index] >= 0 && FD_ISSET((SOCKET)descriptors[index], set))
			descriptors[kept++] = descriptors[index];
	}
	*count = kept;
}

int posix_socket_select(int *read, int *read_count, int *write, int *write_count,
	int *error, int *error_count, posix_long timeout_seconds, posix_long timeout_microseconds, int infinite)
{
	fd_set read_set, write_set, error_set;
	struct timeval timeout;
	int sockets = 0;
	int result;

	sockets += fill_set(&read_set, read, read ? *read_count : 0);
	sockets += fill_set(&write_set, write, write ? *write_count : 0);
	sockets += fill_set(&error_set, error, error ? *error_count : 0);
	if (!sockets)
	{
		/* Windows rejects a select() without sockets; POSIX waits */
		if (!infinite)
			Sleep((DWORD)(timeout_seconds * 1000 + timeout_microseconds / 1000));
		if (read)
			*read_count = 0;
		if (write)
			*write_count = 0;
		if (error)
			*error_count = 0;
		last_error = 0;
		return 0;
	}
	timeout.tv_sec = timeout_seconds;
	timeout.tv_usec = timeout_microseconds;
	result = select(0, read ? &read_set : NULL, write ? &write_set : NULL,
		error ? &error_set : NULL, infinite ? NULL : &timeout);
	if (result == SOCKET_ERROR)
		return fail();
	if (read)
		keep_ready(&read_set, read, read_count);
	if (write)
		keep_ready(&write_set, write, write_count);
	if (error)
		keep_ready(&error_set, error, error_count);
	last_error = 0;
	return result;
}

/* the address the default route leaves from: a UDP socket "connected" to
an internet address (a documentation one; nothing is sent) has it */
static posix_ulong default_route_address(void)
{
	struct sockaddr_in route;
	int length = sizeof(route);
	posix_ulong result = 0;
	SOCKET probe;

	start_winsock();
	probe = WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_NO_HANDLE_INHERIT);
	if (probe == INVALID_SOCKET)
		return 0;
	memset(&route, 0, sizeof(route));
	route.sin_family = AF_INET;
	route.sin_port = htons(9);
	route.sin_addr.s_addr = htonl(0xC6336401);
	if (connect(probe, (struct sockaddr *)&route, sizeof(route)) == 0 &&
		getsockname(probe, (struct sockaddr *)&route, &length) == 0 &&
		route.sin_addr.s_addr != htonl(INADDR_ANY) && (ntohl(route.sin_addr.s_addr) >> 24) != 127)
	{
		result = route.sin_addr.s_addr;
	}
	closesocket(probe);
	return result;
}

posix_ulong posix_local_ipv4_address(void)
{
	ULONG size = 16 * 1024;
	IP_ADAPTER_ADDRESSES *adapters = NULL, *adapter;
	posix_ulong result = default_route_address();
	ULONG status;

	/* no route out (a network without the internet): the first adapter
	that is up and not loopback */
	if (result)
		return result;
	do
	{
		free(adapters);
		adapters = malloc(size);
		if (!adapters)
			return 0;
		status = GetAdaptersAddresses(AF_INET,
			GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, NULL, adapters, &size);
	}
	while (status == ERROR_BUFFER_OVERFLOW);
	if (status == NO_ERROR)
	{
		for (adapter = adapters; adapter && !result; adapter = adapter->Next)
		{
			IP_ADAPTER_UNICAST_ADDRESS *address;

			if (adapter->OperStatus != IfOperStatusUp || adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
				continue;
			for (address = adapter->FirstUnicastAddress; address; address = address->Next)
			{
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)address->Address.lpSockaddr;

				if (ipv4->sin_family == AF_INET && ipv4->sin_addr.s_addr != htonl(INADDR_LOOPBACK))
				{
					result = ipv4->sin_addr.s_addr;
					break;
				}
			}
		}
	}
	free(adapters);
	return result;
}

void posix_random_bytes(void *buffer, posix_ulong size)
{
	/* the keys and invites made from these must not be guessable */
	if (!BCRYPT_SUCCESS(BCryptGenRandom(NULL, buffer, size, BCRYPT_USE_SYSTEM_PREFERRED_RNG)))
		abort();
}

posix_ulong posix_resolve_ipv4(const char *host)
{
	struct addrinfo hints, *results;
	posix_ulong address = 0;

	start_winsock();
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	if (getaddrinfo(host, NULL, &hints, &results) != 0)
		return 0;
	if (results && results->ai_addr && results->ai_addr->sa_family == AF_INET)
		address = ((struct sockaddr_in *)results->ai_addr)->sin_addr.s_addr;
	freeaddrinfo(results);
	return address;
}

/* ---------- IPv6 (internet play's tunnel) */

int posix_socket_udp6(unsigned short port, unsigned short *bound_port)
{
	struct sockaddr_in6 address;
	int length = sizeof(address);
	DWORD only = 1;
	int result = posix_socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

	if (result < 0)
		return -1;
	memset(&address, 0, sizeof(address));
	address.sin6_family = AF_INET6;
	address.sin6_port = port;
	address.sin6_addr = in6addr_any;
	/* (the IPv4 tunnel has its own socket, on the same port) */
	if (setsockopt((SOCKET)result, IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&only, sizeof(only)) == SOCKET_ERROR ||
		bind((SOCKET)result, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR ||
		getsockname((SOCKET)result, (struct sockaddr *)&address, &length) == SOCKET_ERROR ||
		posix_socket_set_nonblocking(result, 1) < 0)
	{
		fail();
		closesocket((SOCKET)result);
		return -1;
	}
	if (bound_port)
		*bound_port = address.sin6_port;
	last_error = 0;
	return result;
}

int posix_socket_sendto6(int socket, const void *buffer, int length, const unsigned char *address,
	unsigned short port)
{
	struct sockaddr_in6 to;

	memset(&to, 0, sizeof(to));
	to.sin6_family = AF_INET6;
	to.sin6_port = port;
	memcpy(&to.sin6_addr, address, 16);
	return posix_socket_sendto(socket, buffer, length, 0, &to, sizeof(to));
}

int posix_socket_recvfrom6(int socket, void *buffer, int length, unsigned char *address, unsigned short *port)
{
	struct sockaddr_in6 from;
	int from_length = sizeof(from);
	int result;

	memset(&from, 0, sizeof(from));
	result = posix_socket_recvfrom(socket, buffer, length, 0, &from, &from_length);
	if (result >= 0)
	{
		memcpy(address, &from.sin6_addr, 16);
		*port = from.sin6_port;
	}
	return result;
}

int posix_local_ipv6_address(unsigned char *address)
{
	/* the address the default route leaves from, as for IPv4: a UDP socket
	"connected" to an internet address (a documentation one, 2001:db8::1;
	nothing is sent) has it */
	static const unsigned char destination[16] = { 0x20, 0x01, 0x0D, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
	struct sockaddr_in6 route;
	int length = sizeof(route);
	int found = 0;
	SOCKET probe;

	start_winsock();
	probe = WSASocketW(AF_INET6, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_NO_HANDLE_INHERIT);
	if (probe == INVALID_SOCKET)
		return 0;
	memset(&route, 0, sizeof(route));
	route.sin6_family = AF_INET6;
	route.sin6_port = htons(9);
	memcpy(&route.sin6_addr, destination, 16);
	if (connect(probe, (struct sockaddr *)&route, sizeof(route)) == 0 &&
		getsockname(probe, (struct sockaddr *)&route, &length) == 0 && route.sin6_family == AF_INET6)
	{
		memcpy(address, &route.sin6_addr, 16);
		found = 1;
	}
	closesocket(probe);
	return found;
}
