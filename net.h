/*
 * $Id: net.h,v 1.1.1.1 2003-04-18 23:43:07 fjoe Exp $
 */

#ifndef _NET_H_
#define _NET_H_

int	connect_mud(const char *hostname, int port);
void	disconnect_mud(int fd);

char	*read_line_mud(int fd, int log);
void	write_mud(int fd, const char *fmt, ...);

void	write_fd(int fd, const char *buf, size_t len);

#endif /* _NET_H_ */
