/*
 * $Id: config.h,v 1.1 2003-04-19 09:28:16 fjoe Exp $
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef WIN32

#define strcasecmp stricmp
#define strncasecmp strnicmp
#define mkdir(n, m) _mkdir(n)
#define lstat(n, sb) stat(n, sb)
#define arc4random rand
#define S_ISDIR(m)      (((m) & _S_IFMT) == _S_IFDIR)

size_t strlcpy(char *dst, const char *src, size_t siz);
size_t strlcat(char *dst, const char *src, size_t siz);
int mkstemp(char *path);

#define MAXPATHLEN	_MAX_PATH

typedef int ssize_t;
typedef unsigned int uint32_t;

#define __P(a)	a
#define __warn_references(f, w)

#endif /* WIN32 */

#endif /* _CONFIG_H_ */