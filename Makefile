#
# $Id: Makefile,v 1.2 2003-04-19 00:15:38 fjoe Exp $
#

PROG=		mudftp
SRCS=		mudftp.c config.c net.c
CFLAGS=		-Wall -W -Wno-shadow -g
NOMAN=		yes
NOOBJ=		yes

PREFIX?=	/usr/local
BINDIR=		${PREFIX}/bin

.include <bsd.prog.mk>
