/*
 * $Id: conf.h,v 1.1.1.1 2003-04-18 23:43:07 fjoe Exp $
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

/* One mud */
typedef struct mud mud_t;
struct mud
{
	mud_t *next;

	char *name;     /* nickname used on command line */
	int port;       /* port of the mudFTP server there */

	char *hostname;
	char *username;
	char *password;
};

/* Editor */
extern char *editor;

void read_config(void); /* Read in the configuration file */
mud_t *mud_lookup(const char *name, int def);

#endif /* _CONFIG_H_ */
