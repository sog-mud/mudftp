#
# $Id: Makefile,v 1.3 2003-04-19 07:47:41 fjoe Exp $
#

PROG=		mudftp
SRCS=		mudftp.c conf.c net.c
CFLAGS=		-Wall -W -Wno-shadow -g
NOMAN=		yes
NOOBJ=		yes

PREFIX?=	/usr/local
BINDIR=		${PREFIX}/bin

.include <bsd.prog.mk>
