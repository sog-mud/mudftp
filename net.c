/*
 * $Id: net.c,v 1.1.1.1 2003-04-18 23:43:07 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>

#include "net.h"

/* Connect to this mud; return fd or -1 if failed to connect */
int
connect_mud(const char *hostname, int port)
{
	struct sockaddr_in sockad;
	int fd;

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

	sockad.sin_port = htons (port);
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
	close(fd);
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
		res = read(fd, buf + pos, sizeof(buf) - pos);
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

	write_fd(fd, buf, strlen(buf));
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
