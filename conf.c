/*
 * $Id: conf.c,v 1.1.1.1 2003-04-18 23:43:07 fjoe Exp $
 *
 * Reading of a configuration file
 */

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#define strcasecmp stricmp
#define strncasecmp strnicmp
#else
#include <unistd.h>
#endif

#include "config.h"

static mud_t *mud_list;
char *editor;

#define CONFIG_FILE_1 "/.mudftp"
#define CONFIG_FILE_2 "mudftp.cfg" // used to be "config"

FILE *
open_config()
{
	char filename[256];
	char *env;
	FILE *fp;

	/* If MUDFTP is defined, it points at our config file */
	if ((env = getenv("MUDFTP")))
		return fopen(env, "r");

	/* Let's try ~/.mudftp first */
	if ((env = getenv("HOME"))) {
		strlcpy(filename, env, sizeof(filename));
		strlcat(filename, CONFIG_FILE_1, sizeof(filename));

		if ((fp = fopen(filename, "r")) != NULL)
			return fp;
	}

	/* Last try: file "config" in current directory */
	/* This is what non-UNIX users will do */
	return fopen(CONFIG_FILE_2, "r");
}

void
read_config(void)
{
	char buf[1024];
	FILE *fp;
	struct stat sb;

	if ((fp = open_config()) == NULL) {
#ifdef WIN32
		fprintf(stderr, "Unable to open config file MUDFTP.CFG or the one defined by $MUDFTP.\n");
#else
		fprintf(stderr, "Unable to open config file ~/" CONFIG_FILE_1 " or " CONFIG_FILE_2 " or the one defined by $MUDFTP.\n");
#endif
		exit (1);
	}
	if (fstat(fileno(fp), &sb) < 0) {
		perror("fstat");
		exit(1);
	}
	if (sb.st_mode & S_IROTH) {
		fprintf(stderr, "Error: config file is readable by others\n");
		exit(1);
	}
	if (sb.st_mode & S_IWOTH) {
		fprintf(stderr, "Error: config gile is writeable by others\n");
		exit(1);
	}

	editor = getenv("EDITOR");
	if (editor == NULL)
		editor = strdup("vi");
	while (fgets(buf, sizeof(buf), fp)) {
		char keyword[64];
		int len;
		char *p;

		if ((p = strchr(buf, '\n')) != NULL)
			*p = '\0';
		if ((p = strchr(buf, '#')) != NULL)
			*p = '\0';

		if (1 != sscanf(buf, "%64s %n", keyword, &len))
			continue; /* empty line */

		if (!strcasecmp(keyword, "mud")) {
			mud_t *m;
			char arg[256];
			char username[64], password[64], hostname[128];
			int port;

			if (5 != sscanf(buf+len, "%64s %64s %d %64s %64s", arg, hostname, &port, username, password)) {
				fprintf(stderr, "MUD must be followed by hostname, port, username and password\n");
				exit(1);
			}

			if (port < 1024) {
				fprintf(stderr, "Insane port number: %s:%d\n", arg, port);
				exit(1);
			}

			m = mud_lookup(arg, 0);
			if (m != NULL) {
				free(m->hostname);
				free(m->username);
				free(m->password);
			} else {
				m = malloc(sizeof(*m));
				m->name = strdup(arg);
				m->next = mud_list;
				mud_list = m;
			}
			m->port = port;
			m->hostname = strdup(hostname);
			m->username = strdup(username);
			m->password = strdup(password);
		} else if (!strcasecmp(keyword, "editor")) {
			if (!buf[len]) {
				fprintf(stderr, "Did not supply editor program after editor\n");
				exit(1);
			}
			free(editor);
			editor = strdup(buf+len);
		} else {
			fprintf(stderr, "Unknown keyword in config file: %s\n", keyword);
			exit(1);
		}
	}

	fclose (fp);
}

mud_t *
mud_lookup(const char *name, int def)
{
	mud_t *m;

	for (m = mud_list; m; m = m->next) {
		if (!strcasecmp(m->name, name)
		||  (def && !strcasecmp(m->name, "default")))
			return m;
	}

	return NULL;
}
