#
# $Id: Makefile,v 1.1.1.1 2003-04-18 23:43:07 fjoe Exp $
#

PROG=		mudftp
SRCS=		mudftp.c config.c net.c
CFLAGS=		-Wall -W -Wno-shadow -g
NOMAN=		yes
NOOBJ=		yes

.include <bsd.prog.mk>
