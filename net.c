/*
 * $Id: net.c,v 1.2 2003-04-19 09:28:16 fjoe Exp $
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef WIN32

#include <winsock.h>
#include <time.h>
#include <io.h>

#else

#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#endif

#include "config.h"
#include "net.h"

#ifdef WIN32
static void write_sock(int fd, const char *buf, size_t len);
#else
#define write_sock write_fd
#endif

/* Connect to this mud; return fd or -1 if failed to connect */
int
connect_mud(const char *hostname, int port)
{
	struct sockaddr_in sockad;
	int fd;

#ifdef WIN32
	WSADATA wsadata;
#define WSVERS MAKEWORD(2,2)
	if (WSAStartup(WSVERS, &wsadata) != 0) {
		fprintf(stderr, "Failed to initialize winsock.\n");
		exit(1);
	}
#endif

	/* Check if inet_addr will accept this as a valid address */
	/* rather than assuming whatever starts with a digit is an ip */
	if ((sockad.sin_addr.s_addr = inet_addr (hostname)) == (unsigned long) -1) {
		struct hostent *h;

		if ((h = gethostbyname (hostname)) == NULL) {
			perror("connect_mud: gethostbyname");
			return -1;
		}
		memcpy ((char *) &sockad.sin_addr, h->h_addr, sizeof (sockad.sin_addr));
	}

	sockad.sin_port = htons ((short) port);
	sockad.sin_family = AF_INET;

	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("connect_mud: socket");
		return -1;
	}

	if (connect (fd, (struct sockaddr *) &sockad, sizeof (sockad))) {
		perror("connect_mud: connect");
		return -1;
	}

	return fd;
}

void
disconnect_mud(int fd)
{
#ifdef WIN32
	closesocket(fd);
	WSACleanup();
#else
	close(fd);
#endif
}

/* Read data from MUD; return exactly one line */
char *
read_line_mud(int fd, int log)
{
	static char buf[4096];
	static char output[sizeof(buf)];
	static int pos;
	int res = 0;
	char *newline;
	size_t	len;

	while (!pos || (newline = (char *) memchr (buf, '\n', pos)) == NULL) {
#ifdef WIN32
		res = recv(fd, buf + pos, sizeof(buf) - pos, 0);
#else
		res = read(fd, buf + pos, sizeof(buf) - pos);
#endif
		if (res < 0) {
			perror ("read_line_mud: read");
			exit(1);
		} else if (res == 0) {
			fprintf (stderr, "Remote closed connection\n");
			exit(1);
		}

		pos += res;
	}

	/*
	 * Copy out output
	 */
	len = newline - buf;
	memcpy(output, buf, len);
	output[len] = '\0';
	if (len > 0 && output[len - 1] == '\r')
		output[len - 1] = '\0';

	/*
	 * Shift input buffer
	 */
	if (newline[1] == '\r') {
		len++;
		newline++;
	}
	memmove(buf, newline + 1, pos - len);
	pos -= len + 1;
	buf[pos] = '\0';
#if 0
	fprintf(stderr, "pos=%d, nl=%d, last read=%d\n",
		pos, len, res);
#endif

	if (log)
		fprintf(stderr, "==> [%s]\n", output);
	return output;
}

void
write_mud(int fd, const char *fmt, ...)
{
	char buf[16384];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	write_sock(fd, buf, strlen(buf));
}

void
write_fd(int fd, const char *buf, size_t len)
{
	const char *p = buf;

	while (len > 0) {
		ssize_t nw;

		nw = write(fd, p, len);
		if (nw < 0) {
			perror("write_fd: write");
			exit(1);
		}

		p += nw;
		len -= nw;
	}
}

#ifdef WIN32
void
write_sock(int fd, const char *buf, size_t len)
{
	const char *p = buf;

	while (len > 0) {
		ssize_t nw;

		nw = send(fd, p, len, 0);
		if (nw < 0) {
			perror("write_fd: write");
			exit(1);
		}

		p += nw;
		len -= nw;
	}
}
#endif