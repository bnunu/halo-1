/*
POSIX_NET.C

BSD socket helpers behind the Winsock layer in xnet.c (see posix.h). Built
with the host ABI.
*/

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <limits.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/random.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include "posix.h"

/* Winsock error codes (winsockx.h) */
#define WSAEINTR 10004
#define WSAEBADF 10009
#define WSAEACCES 10013
#define WSAEFAULT 10014
#define WSAEINVAL 10022
#define WSAEMFILE 10024
#define WSAEWOULDBLOCK 10035
#define WSAEINPROGRESS 10036
#define WSAEALREADY 10037
#define WSAENOTSOCK 10038
#define WSAEDESTADDRREQ 10039
#define WSAEMSGSIZE 10040
#define WSAEPROTOTYPE 10041
#define WSAENOPROTOOPT 10042
#define WSAEPROTONOSUPPORT 10043
#define WSAEOPNOTSUPP 10045
#define WSAEAFNOSUPPORT 10047
#define WSAEADDRINUSE 10048
#define WSAEADDRNOTAVAIL 10049
#define WSAENETDOWN 10050
#define WSAENETUNREACH 10051
#define WSAENETRESET 10052
#define WSAECONNABORTED 10053
#define WSAECONNRESET 10054
#define WSAENOBUFS 10055
#define WSAEISCONN 10056
#define WSAENOTCONN 10057
#define WSAESHUTDOWN 10058
#define WSAETIMEDOUT 10060
#define WSAECONNREFUSED 10061
#define WSAEHOSTDOWN 10064
#define WSAEHOSTUNREACH 10065

/* Winsock SOL_SOCKET option values (winsockx.h) */
#define WINSOCK_SOL_SOCKET 0xffff
#define WINSOCK_SO_REUSEADDR 0x0004
#define WINSOCK_SO_KEEPALIVE 0x0008
#define WINSOCK_SO_BROADCAST 0x0020
#define WINSOCK_SO_LINGER 0x0080
#define WINSOCK_SO_SNDBUF 0x1001
#define WINSOCK_SO_RCVBUF 0x1002
#define WINSOCK_SO_ERROR 0x1007
#define WINSOCK_SO_TYPE 0x1008

static __thread int last_error;

static int fail(void)
{
	switch (errno)
	{
	case EINTR: last_error = WSAEINTR; break;
	case EBADF: last_error = WSAEBADF; break;
	case EACCES: case EPERM: last_error = WSAEACCES; break;
	case EFAULT: last_error = WSAEFAULT; break;
	case EMFILE: case ENFILE: last_error = WSAEMFILE; break;
	case EAGAIN: last_error = WSAEWOULDBLOCK; break;
	case EINPROGRESS: last_error = WSAEINPROGRESS; break;
	case EALREADY: last_error = WSAEALREADY; break;
	case ENOTSOCK: last_error = WSAENOTSOCK; break;
	case EDESTADDRREQ: last_error = WSAEDESTADDRREQ; break;
	case EMSGSIZE: last_error = WSAEMSGSIZE; break;
	case EPROTOTYPE: last_error = WSAEPROTOTYPE; break;
	case ENOPROTOOPT: last_error = WSAENOPROTOOPT; break;
	case EPROTONOSUPPORT: last_error = WSAEPROTONOSUPPORT; break;
	case EOPNOTSUPP: last_error = WSAEOPNOTSUPP; break;
	case EAFNOSUPPORT: last_error = WSAEAFNOSUPPORT; break;
	case EADDRINUSE: last_error = WSAEADDRINUSE; break;
	case EADDRNOTAVAIL: last_error = WSAEADDRNOTAVAIL; break;
	case ENETDOWN: last_error = WSAENETDOWN; break;
	case ENETUNREACH: last_error = WSAENETUNREACH; break;
	case ENETRESET: last_error = WSAENETRESET; break;
	case ECONNABORTED: last_error = WSAECONNABORTED; break;
	/* a send on a connection the other end reset (with MSG_NOSIGNAL):
	Winsock's WSAECONNRESET, which the game takes as the connection lost */
	case ECONNRESET: case EPIPE: last_error = WSAECONNRESET; break;
	case ESHUTDOWN: last_error = WSAESHUTDOWN; break;
	case EHOSTDOWN: last_error = WSAEHOSTDOWN; break;
	case ENOBUFS: case ENOMEM: last_error = WSAENOBUFS; break;
	case EISCONN: last_error = WSAEISCONN; break;
	case ENOTCONN: last_error = WSAENOTCONN; break;
	case ETIMEDOUT: last_error = WSAETIMEDOUT; break;
	case ECONNREFUSED: last_error = WSAECONNREFUSED; break;
	case EHOSTUNREACH: last_error = WSAEHOSTUNREACH; break;
	default: last_error = WSAEINVAL; break;
	}
	return -1;
}

static int succeed(int result)
{
	if (result < 0)
		return fail();
	last_error = 0;
	return result;
}

int posix_socket_last_error(void)
{
	return last_error;
}

int posix_socket(int family, int type, int protocol)
{
	return succeed(socket(family, type | SOCK_CLOEXEC, protocol));
}

int posix_socket_close(int socket)
{
	return succeed(close(socket));
}

int posix_socket_bind(int socket, const void *address, int address_length)
{
	return succeed(bind(socket, address, (socklen_t)address_length));
}

int posix_socket_connect(int socket, const void *address, int address_length)
{
	/* A non-blocking connect that is under way is EINPROGRESS here but
	WSAEWOULDBLOCK in Winsock, which is what the game waits on before it
	selects for the socket becoming writeable (connect_endpoint,
	transport_endpoint_winsock.c); as WSAEINPROGRESS it gave up at once,
	and every system link join failed, a split screen game's join of its
	own host included. */
	int result = connect(socket, address, (socklen_t)address_length);

	if (result < 0 && errno == EINPROGRESS)
	{
		last_error = WSAEWOULDBLOCK;
		return -1;
	}
	return succeed(result);
}

int posix_socket_listen(int socket, int backlog)
{
	return succeed(listen(socket, backlog));
}

int posix_socket_accept(int socket, void *address, int *address_length)
{
	socklen_t length = address_length ? (socklen_t)*address_length : 0;
	int result = accept4(socket, address, address_length ? &length : NULL, SOCK_CLOEXEC);

	if (address_length)
		*address_length = (int)length;
	return succeed(result);
}

int posix_socket_send(int socket, const void *buffer, int length, int flags)
{
	return succeed((int)send(socket, buffer, (size_t)length, flags | MSG_NOSIGNAL));
}

int posix_socket_sendto(int socket, const void *buffer, int length, int flags,
	const void *address, int address_length)
{
	return succeed((int)sendto(socket, buffer, (size_t)length, flags | MSG_NOSIGNAL,
		address, (socklen_t)address_length));
}

int posix_socket_recv(int socket, void *buffer, int length, int flags)
{
	return succeed((int)recv(socket, buffer, (size_t)length, flags));
}

int posix_socket_recvfrom(int socket, void *buffer, int length, int flags,
	void *address, int *address_length)
{
	struct iovec vector;
	struct msghdr message;
	int result;

	vector.iov_base = buffer;
	vector.iov_len = (size_t)length;
	memset(&message, 0, sizeof(message));
	message.msg_name = address && address_length ? address : NULL;
	message.msg_namelen = address && address_length ? (socklen_t)*address_length : 0;
	message.msg_iov = &vector;
	message.msg_iovlen = 1;
	result = (int)recvmsg(socket, &message, flags);
	if (address_length)
		*address_length = (int)message.msg_namelen;
	/* a datagram larger than the buffer: both give its start, but Winsock
	with WSAEMSGSIZE, which the game takes as an error, not as the datagram */
	if (result >= 0 && (message.msg_flags & MSG_TRUNC))
	{
		last_error = WSAEMSGSIZE;
		return -1;
	}
	return succeed(result);
}

int posix_socket_shutdown(int socket, int how)
{
	return succeed(shutdown(socket, how));
}

int posix_socket_set_nonblocking(int socket, int nonblocking)
{
	int flags = fcntl(socket, F_GETFL);

	if (flags < 0)
		return fail();
	flags = nonblocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
	return succeed(fcntl(socket, F_SETFL, flags));
}

int posix_socket_set_nodelay(int socket)
{
	int value = 1;

	return succeed(setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value)));
}

int posix_socket_bytes_available(int socket, posix_ulong *count)
{
	int available = 0;
	int result = ioctl(socket, FIONREAD, &available);

	if (result >= 0)
		*count = (posix_ulong)available;
	return succeed(result);
}

static int translate_option(int level, int name, int *host_level, int *host_name)
{
	if (level != WINSOCK_SOL_SOCKET)
	{
		/* IPPROTO_IP / IPPROTO_TCP option numbers are shared */
		*host_level = level;
		*host_name = name;
		return 0;
	}
	*host_level = SOL_SOCKET;
	switch (name)
	{
	case WINSOCK_SO_REUSEADDR: *host_name = SO_REUSEADDR; return 0;
	case WINSOCK_SO_KEEPALIVE: *host_name = SO_KEEPALIVE; return 0;
	case WINSOCK_SO_BROADCAST: *host_name = SO_BROADCAST; return 0;
	case WINSOCK_SO_LINGER: *host_name = SO_LINGER; return 0;
	case WINSOCK_SO_SNDBUF: *host_name = SO_SNDBUF; return 0;
	case WINSOCK_SO_RCVBUF: *host_name = SO_RCVBUF; return 0;
	case WINSOCK_SO_ERROR: *host_name = SO_ERROR; return 0;
	case WINSOCK_SO_TYPE: *host_name = SO_TYPE; return 0;
	default: return -1;
	}
}

int posix_socket_setsockopt(int socket, int level, int name, const void *value, int length)
{
	int host_level, host_name;

	if (translate_option(level, name, &host_level, &host_name) != 0)
	{
		/* Xbox-only options such as SO_ENCRYPT have nothing to do here */
		last_error = 0;
		return 0;
	}
	return succeed(setsockopt(socket, host_level, host_name, value, (socklen_t)length));
}

int posix_socket_getsockopt(int socket, int level, int name, void *value, int *length)
{
	int host_level, host_name;
	socklen_t socket_length = (socklen_t)*length;
	int result;

	if (translate_option(level, name, &host_level, &host_name) != 0)
	{
		last_error = WSAENOPROTOOPT;
		return -1;
	}
	result = getsockopt(socket, host_level, host_name, value, &socket_length);
	*length = (int)socket_length;
	return succeed(result);
}

int posix_socket_getsockname(int socket, void *address, int *address_length)
{
	socklen_t length = (socklen_t)*address_length;
	int result = getsockname(socket, address, &length);

	*address_length = (int)length;
	return succeed(result);
}

int posix_socket_getpeername(int socket, void *address, int *address_length)
{
	socklen_t length = (socklen_t)*address_length;
	int result = getpeername(socket, address, &length);

	*address_length = (int)length;
	return succeed(result);
}

int posix_socket_select(int *read, int *read_count, int *write, int *write_count,
	int *error, int *error_count, posix_long timeout_seconds, posix_long timeout_microseconds, int infinite)
{
	/* poll, which takes any descriptor (select none from FD_SETSIZE on,
	which a process allowed more files has), with select's readiness: read
	for data, the end or an error, write for room or an error, error for
	urgent data or (as Winsock's) a connect that failed */
	static const short events[3] = { POLLIN, POLLOUT, POLLPRI };
	static const short ready[3] = { POLLIN | POLLHUP | POLLERR, POLLOUT | POLLERR, POLLPRI | POLLERR };
	/* (larger sets, as internet play's thread waits on, in a buffer each
	thread keeps: not one allocation each time) */
	static __thread struct pollfd *buffer;
	static __thread int buffer_size;
	int *lists[3] = { read, write, error };
	int *counts[3] = { read_count, write_count, error_count };
	struct pollfd stack[256];
	struct pollfd *descriptors = stack;
	long long milliseconds = (long long)timeout_seconds * 1000 + ((long long)timeout_microseconds + 999) / 1000;
	int total = 0;
	int list, index;
	int result;

	for (list = 0; list < 3; list++)
	{
		if (!lists[list] || !counts[list])
			lists[list] = NULL;
		else
			total += *counts[list];
	}
	if (total > (int)(sizeof(stack) / sizeof(*stack)))
	{
		if (total > buffer_size)
		{
			struct pollfd *larger = realloc(buffer, sizeof(*buffer) * (size_t)total);

			if (!larger)
			{
				errno = ENOMEM;
				return fail();
			}
			buffer = larger;
			buffer_size = total;
		}
		descriptors = buffer;
	}
	total = 0;
	for (list = 0; list < 3; list++)
	{
		for (index = 0; lists[list] && index < *counts[list]; index++, total++)
		{
			descriptors[total].fd = lists[list][index];
			descriptors[total].events = events[list];
			descriptors[total].revents = 0;
		}
	}
	result = poll(descriptors, (nfds_t)total, infinite ? -1 :
		(int)(milliseconds < 0 ? 0 : milliseconds > INT_MAX ? INT_MAX : milliseconds));
	for (index = 0; index < total && result > 0; index++)
	{
		/* (as select fails on a descriptor that is not open) */
		if (descriptors[index].revents & POLLNVAL)
		{
			errno = EBADF;
			result = -1;
		}
	}
	if (result < 0)
		return fail();
	result = 0;
	total = 0;
	for (list = 0; list < 3; list++)
	{
		int kept = 0;

		for (index = 0; lists[list] && index < *counts[list]; index++, total++)
		{
			int descriptor = lists[list][index];
			int pending = 0;
			socklen_t length = sizeof(pending);

			if (!(descriptors[total].revents & ready[list]))
				continue;
			/* Winsock reports a socket writeable once its connect has
			succeeded; one whose connect failed is not (it is in the error
			set), where POSIX reports it writeable with the failure in
			SO_ERROR. The game takes writeable as connected
			(connect_endpoint). */
			if (list == 1 && getsockopt(descriptor, SOL_SOCKET, SO_ERROR, &pending, &length) == 0 && pending)
			{
				errno = pending;
				fail();
				continue;
			}
			lists[list][kept++] = descriptor;
		}
		if (lists[list])
			*counts[list] = kept;
		result += kept;
	}
	/* like Winsock, a select with nothing ready leaves the last error as it
	was: after a connect under way, still WSAEWOULDBLOCK, which the game
	reads as not connected yet */
	if (result > 0)
		last_error = 0;
	return result;
}

/* the first IPv4 address of an interface that is up, running, not
loopback and has these flags; or 0 */
static posix_ulong interface_address(unsigned int flags)
{
	struct ifaddrs *addresses, *entry;
	posix_ulong result = 0;

	if (getifaddrs(&addresses) != 0)
		return 0;
	for (entry = addresses; entry; entry = entry->ifa_next)
	{
		if (entry->ifa_addr && entry->ifa_addr->sa_family == AF_INET &&
			(entry->ifa_flags & (IFF_UP | IFF_RUNNING | flags)) == (IFF_UP | IFF_RUNNING | flags) &&
			!(entry->ifa_flags & IFF_LOOPBACK))
		{
			struct sockaddr_in *address = (struct sockaddr_in *)entry->ifa_addr;

			if ((ntohl(address->sin_addr.s_addr) >> 24) != 127)
			{
				result = address->sin_addr.s_addr;
				break;
			}
		}
	}
	freeifaddrs(addresses);
	return result;
}

posix_ulong posix_local_ipv4_address(void)
{
	struct sockaddr_in route;
	socklen_t length = sizeof(route);
	posix_ulong result = 0;
	int probe;

#ifdef __ANDROID__
	/* a phone's default route may be its mobile data (on Wi-Fi without the
	internet, or sharing its connection), which the local network cannot
	reach: first the local network's interface (Wi-Fi, or the one it
	shares its connection on), which alone of them has broadcasts */
	result = interface_address(IFF_BROADCAST);
	if (result)
		return result;
#endif
	/* the address the default route leaves from: a UDP socket "connected"
	to an internet address (a documentation one; nothing is sent) has it */
	probe = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (probe >= 0)
	{
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
		close(probe);
	}
	/* no route out (a network without the internet): the first interface
	that is up, running and not loopback */
	return result ? result : interface_address(0);
}

void posix_random_bytes(void *buffer, posix_ulong size)
{
	unsigned char *cursor = buffer;

	while (size)
	{
		ssize_t count = getrandom(cursor, size, 0);

		if (count < 0 && errno == EINTR)
			continue;
		if (count <= 0)
			break;
		cursor += count;
		size -= (posix_ulong)count;
	}
	if (size)
	{
		/* a kernel without getrandom, or a sandbox that refuses it */
		int descriptor = open("/dev/urandom", O_RDONLY | O_CLOEXEC);

		while (descriptor >= 0 && size)
		{
			ssize_t count = read(descriptor, cursor, size);

			if (count < 0 && errno == EINTR)
				continue;
			if (count <= 0)
				break;
			cursor += count;
			size -= (posix_ulong)count;
		}
		if (descriptor >= 0)
			close(descriptor);
	}
	/* the keys and invites made from these must not be guessable */
	if (size)
	{
		fputs("no random numbers from the system: cannot continue\n", stderr);
		abort();
	}
}

posix_ulong posix_resolve_ipv4(const char *host)
{
	struct addrinfo hints, *results;
	posix_ulong address = 0;

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
	socklen_t length = sizeof(address);
	int only = 1;
	int result = socket(AF_INET6, SOCK_DGRAM | SOCK_CLOEXEC, 0);

	if (result < 0)
		return fail();
	memset(&address, 0, sizeof(address));
	address.sin6_family = AF_INET6;
	address.sin6_port = port;
	address.sin6_addr = in6addr_any;
	/* (the IPv4 tunnel has its own socket, on the same port) */
	if (setsockopt(result, IPPROTO_IPV6, IPV6_V6ONLY, &only, sizeof(only)) < 0 ||
		bind(result, (struct sockaddr *)&address, sizeof(address)) < 0 ||
		getsockname(result, (struct sockaddr *)&address, &length) < 0 ||
		posix_socket_set_nonblocking(result, 1) < 0)
	{
		fail();
		close(result);
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
	socklen_t length = sizeof(route);
	int found = 0;
	int probe = socket(AF_INET6, SOCK_DGRAM | SOCK_CLOEXEC, 0);

	if (probe < 0)
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
	close(probe);
	return found;
}

/* ---------- the process and the desktop */

int posix_command_line_argument(int index, char *buffer, posix_ulong size)
{
#ifdef __ANDROID__
	(void)index;
	(void)buffer;
	(void)size;
	return 0;
#else
	char command_line[4096];
	ssize_t length;
	ssize_t offset = 0;
	int descriptor = open("/proc/self/cmdline", O_RDONLY);

	if (descriptor < 0 || !size)
		return descriptor >= 0 ? (close(descriptor), 0) : 0;
	length = read(descriptor, command_line, sizeof(command_line) - 1);
	close(descriptor);
	if (length <= 0)
		return 0;
	command_line[length] = '\0';
	/* the arguments are NUL-separated */
	while (index-- > 0)
	{
		offset += (ssize_t)strlen(command_line + offset) + 1;
		if (offset >= length)
			return 0;
	}
	snprintf(buffer, size, "%s", command_line + offset);
	return 1;
#endif
}

posix_ulong posix_process_id(void)
{
	return (posix_ulong)getpid();
}

int posix_user_secret(unsigned char *secret, int size)
{
#ifdef __ANDROID__
	(void)secret;
	(void)size;
	return 0;
#else
	/* in the user's runtime directory (theirs alone), else their home */
	const char *runtime = getenv("XDG_RUNTIME_DIR");
	const char *home = getenv("HOME");
	char path[1024];
	int attempt;

	if (runtime && *runtime)
		snprintf(path, sizeof(path), "%s/halo-ce-universal.key", runtime);
	else if (home && *home)
		snprintf(path, sizeof(path), "%s/.halo-ce-universal.key", home);
	else
		return 0;
	for (attempt = 0; attempt < 3; attempt++)
	{
		struct stat status;
		int descriptor = open(path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
		int ok;

		if (descriptor >= 0)
		{
			/* only one of the user's that no one else can read */
			ok = fstat(descriptor, &status) == 0 && S_ISREG(status.st_mode) && status.st_uid == getuid() &&
				!(status.st_mode & 077);
			if (ok && read(descriptor, secret, (size_t)size) != size)
			{
				/* a key cut short (a write that failed, or was stopped) is made
				again, but not one another copy is writing now */
				ok = 0;
				if (status.st_mtime + 2 < time(NULL) && unlink(path) == 0)
				{
					close(descriptor);
					continue;
				}
			}
			close(descriptor);
			return ok;
		}
		if (errno != ENOENT)
			return 0;
		descriptor = open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
		/* (another copy of the game made it first: read that one) */
		if (descriptor < 0)
			continue;
		posix_random_bytes(secret, (posix_ulong)size);
		ok = write(descriptor, secret, (size_t)size) == size;
		close(descriptor);
		if (!ok)
			unlink(path);
		return ok;
	}
	return 0;
#endif
}

#ifndef __ANDROID__
/* runs a program with its arguments and waits for it; its exit status, or -1 */
static int run_program(char *const arguments[])
{
	extern char **environ;
	pid_t process;
	int status;

	if (posix_spawnp(&process, arguments[0], NULL, NULL, arguments, environ) != 0)
		return -1;
	if (waitpid(process, &status, 0) < 0)
		return -1;
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
#endif

int posix_register_url_scheme(const char *scheme, const char *description)
{
#ifdef __ANDROID__
	(void)scheme;
	(void)description;
	return 0;
#else
	/* a desktop entry declaring the executable as the scheme's handler, and
	the scheme's default application set to it (as xdg-open reads it) */
	char executable[1024], directory[1024], path[1200], name[128], entry[2048], existing[2048];
	const char *data_home = getenv("XDG_DATA_HOME");
	const char *home = getenv("HOME");
	ssize_t length = readlink("/proc/self/exe", executable, sizeof(executable) - 1);
	FILE *file;
	size_t existing_length = 0;

	if (length <= 0)
		return 0;
	executable[length] = '\0';
	if (data_home && *data_home)
		snprintf(directory, sizeof(directory), "%s/applications", data_home);
	else if (home && *home)
		snprintf(directory, sizeof(directory), "%s/.local/share/applications", home);
	else
		return 0;
	snprintf(name, sizeof(name), "halo-ce-universal-%s.desktop", scheme);
	snprintf(path, sizeof(path), "%s/%s", directory, name);
	snprintf(entry, sizeof(entry),
		"[Desktop Entry]\n"
		"Type=Application\n"
		"Name=%s\n"
		"Exec=\"%s\" %%u\n"
		"NoDisplay=true\n"
		"MimeType=x-scheme-handler/%s;\n",
		description, executable, scheme);
	/* unchanged since the last start: nothing to do */
	file = fopen(path, "r");
	if (file)
	{
		existing_length = fread(existing, 1, sizeof(existing) - 1, file);
		fclose(file);
		existing[existing_length] = '\0';
		if (!strcmp(existing, entry))
			return 1;
	}
	mkdir(directory, 0755);
	file = fopen(path, "w");
	if (!file)
		return 0;
	fputs(entry, file);
	fclose(file);
	{
		char mime_type[160];
		char *arguments[] = { "xdg-mime", "default", name, mime_type, NULL };

		snprintf(mime_type, sizeof(mime_type), "x-scheme-handler/%s", scheme);
		run_program(arguments);
	}
	return 1;
#endif
}

/* ---------- Discord's local socket */

int posix_discord_connect(void)
{
#ifdef __ANDROID__
	return -1;
#else
	/* where Discord (and its Flatpak and Snap packages) put discord-ipc-N */
	static const char *const variables[] = { "XDG_RUNTIME_DIR", "TMPDIR", "TMP", "TEMP" };
	static const char *const subdirectories[] = { "", "app/com.discordapp.Discord/", "snap.discord/",
		".flatpak/dev.vencord.Vesktop/xdg-run/" };
	const char *directories[5];
	int directory_count = 0;
	int index;

	for (index = 0; index < (int)(sizeof(variables) / sizeof(*variables)); index++)
	{
		const char *value = getenv(variables[index]);

		if (value && *value)
			directories[directory_count++] = value;
	}
	directories[directory_count++] = "/tmp";
	for (index = 0; index < directory_count; index++)
	{
		int subdirectory;

		for (subdirectory = 0; subdirectory < (int)(sizeof(subdirectories) / sizeof(*subdirectories)); subdirectory++)
		{
			int number;

			for (number = 0; number < 10; number++)
			{
				struct sockaddr_un address;
				int socket_descriptor;

				memset(&address, 0, sizeof(address));
				address.sun_family = AF_UNIX;
				snprintf(address.sun_path, sizeof(address.sun_path), "%s/%sdiscord-ipc-%d",
					directories[index], subdirectories[subdirectory], number);
				if (access(address.sun_path, F_OK) != 0)
					continue;
				/* (not blocking: a client that does not take connections is
				passed over) */
				socket_descriptor = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
				if (socket_descriptor < 0)
					return -1;
				if (connect(socket_descriptor, (struct sockaddr *)&address, sizeof(address)) == 0)
				{
#ifdef SO_PEERCRED
					/* only this user's Discord (in /tmp another user may make
					the socket, and would be given the invite) */
					struct ucred credentials;
					socklen_t length = sizeof(credentials);

					if (getsockopt(socket_descriptor, SOL_SOCKET, SO_PEERCRED, &credentials, &length) == 0 &&
						credentials.uid == getuid())
					{
						return socket_descriptor;
					}
#else
					return socket_descriptor;
#endif
				}
				close(socket_descriptor);
			}
		}
	}
	return -1;
#endif
}

int posix_discord_write(int handle, const void *buffer, int length)
{
	for (;;)
	{
		ssize_t written = send(handle, buffer, (size_t)length, MSG_NOSIGNAL | MSG_DONTWAIT);

		if (written >= 0)
			return (int)written;
		if (errno != EINTR)
			return errno == EAGAIN ? 0 : -1;
	}
}

int posix_discord_read(int handle, void *buffer, int length)
{
	ssize_t count = recv(handle, buffer, (size_t)length, MSG_DONTWAIT);

	if (count > 0)
		return (int)count;
	if (count < 0 && (errno == EAGAIN || errno == EINTR))
		return 0;
	return -1;
}

void posix_discord_close(int handle)
{
	if (handle >= 0)
		close(handle);
}
