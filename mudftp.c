/*
 * $Id: mudftp.c,v 1.1.1.1 2003-04-18 23:43:07 fjoe Exp $
 */

#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32

#include <errno.h>
#include <winsock.h>
#include <time.h>
#define strncasecmp strnicmp

#else

#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>

#endif

#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "net.h"
#include "config.h"

/*
 * Receive this file, writing it into fp
 */
void
get_file(int fd, char *filename, int ofd, int lines)
{
	char *data;

	/* Wait for that many lines */
	printf("Retrieving %s", filename);
	for (; lines; lines--) {
		data = read_line_mud(fd, 0);
		printf(".");
		write_fd(ofd, data, strlen(data));
#ifdef WIN32
		write_fd(ofd, "\r\n", 2);
#else
		write_fd(ofd, "\n", 1);
#endif
	}
	printf("\nDone.\n");
}

/* Read the file from fp, then send it to the MUD */
void
put_file(int fd, char *filename, FILE *fp)
{
	char checksum[256];
	static char	text[16384];
	char	*pc, *pos, *data;
	int	c;
	int	lines = 0;

	printf("Sending %s", filename);
	/* Read the file, discarding \r's and counting lines */
	for (pc = text; pc - text < (int)sizeof (text) - 2;) {
		if ((c = fgetc (fp)) == EOF)
			break;

		if (c == '\n') {
			*pc++ = '\n';
			lines++;
		} else if (c != '\r')
			*pc++ = c;
	}

	/* Make sure the last line is whole */
	if (pc == text || *(pc -1) != '\n') {
		lines++;
		*pc++ = '\n';
	}
	*pc = '\0';

	write_mud(fd, "PUT %s %d\n", filename, lines);
	lines = 0;
	for (pos = text; pos < pc;) {
		char   *nl;

		printf(".");
		nl = strchr (pos, '\n');
		*nl = 0;
		write_mud(fd, "%s\n", pos);
		pos = nl + 1;
	}
	printf("\nWaiting...\n");
	data = read_line_mud(fd, 1);
	if (1 != sscanf(data, "OK %256s\n", checksum))
		exit(1);
	printf("Done.\n");
}

void spawn_editor (const char *fname)
{
	char command[256];
	char *out = command;
	const char *s;
	int got_filename = 0;

	for (s = editor; *s && out < command + sizeof(command) - 2; s++) {
		if (*s == '%') {
			out += sprintf(out, "%s", fname);
			got_filename = 1;
		} else
			*out++ = *s;
	}

	if (!got_filename)
		out += sprintf(out, " %s", fname);
	*out = '\0';
	system(command);
}

int
main(int argc, char **argv)
{
	char *data;

	int sock;
	mud_t *m;
	char checksum[256];

	if (argc < 2) {
		fprintf(stderr, "Usage: mudftp mud\n");
		exit(1);
	}

	read_config();
	if ((m = mud_lookup(argv[1], 1)) == NULL) {
		fprintf(stderr, "%s: No such mud defined in config\n", argv[1]);
		exit(1);
	}

	/* Try to connect */
	printf("Connecting...\n");
	if ((sock = connect_mud(m->hostname, m->port)) < 0)
		exit(1);

	printf("Authorization in progress...\n");
	write_mud(sock, "%s %s\n", m->username, m->password);
	/* Wait for OK/greeting message */
	data = read_line_mud(sock, 1);
	if (!!strncasecmp(data, "OK", 2))
		exit(1);

	write_mud(sock, "PUSH\n");
	data = read_line_mud(sock, 1);
	if (!!strncasecmp(data, "OK", 2))
		exit(1);

	/* Now do various stuff depending operations mode */
	for (;;) {
		char remote_file[MAXPATHLEN];
		int lines;
		char filename[MAXPATHLEN];
		int tfd;
		char *tmp;
		FILE *fp;

		if ((tmp = getenv("MUDFTP_TEMP")) == NULL
		||  (tmp = getenv("TEMP")) == NULL
		||  (tmp = getenv("TMP")) == NULL)
			tmp = "/tmp";
		strlcpy(filename, tmp, sizeof(filename));
		strlcat(filename, "/tmp.XXXXXX", sizeof(filename));
		if ((tfd = mkstemp(filename)) < 0) {
			/* XXX do not exit */
			fprintf(stderr,
				"mkstemp: %s: %s\n", filename, strerror(errno));
			exit(1);
		}

		fprintf(stderr, "Waiting for server to start the transfer...\n");
		data = read_line_mud(sock, 1);
		if (3 != sscanf (data, "SENDING %256s %d %256s\n", remote_file, &lines, checksum)) {
			fprintf(stderr,
			    "Unexpected message when waiting for push data: %s\n", data);
			exit(1);
		}
                get_file(sock, remote_file, tfd, lines);
		close(tfd);

		/* Spawn the editor */
		spawn_editor(filename);

		/* Upload the file back */
		if ((fp = fopen(filename, "rb")) == NULL) {
			/* XXX do not exit */
			fprintf(stderr,
			    "fopen: %s: %s\n", filename, strerror(errno));
			exit(1);
		}

		put_file(sock, remote_file, fp);
		fclose(fp);
		unlink(filename);
	}

	write_mud(sock, "QUIT\n");
	disconnect_mud(sock);
	exit(0);
}
