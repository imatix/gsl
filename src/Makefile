#
#   Makefile for iMatix GSL/4.1
#
#   Copyright (c) 1996-2010 iMatix Corporation
#   All rights reserved.
#
#   This file is licensed under the BSD license as follows:
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#   * Neither the name of iMatix Corporation nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY IMATIX CORPORATION "AS IS" AND ANY
#   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL IMATIX CORPORATION BE
#   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
#   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

INSTALL = /usr/bin/install -c

# Installation directories, good for Debian Packaging
prefix = ${DESTDIR}/usr
exec_prefix = ${prefix}
mandir = ${prefix}/share/man/man8
bindir = ${exec_prefix}/local/bin

#   Default values for object, library, and executable extensions.
#
OBJ = .o
LIB = .a
EXE =
DBG =
CC  = ./c -q
export CCLIBS = -lpcre

#   Reset the suffixes that will be considered to just our own list.
#
#   Make programs use the .SUFFIXES psuedo rule for this

.SUFFIXES:
.SUFFIXES: $(EXE) $(LIB) $(OBJ) .c .d .i

#   Objects depend on source files with the same name; and are compiled
#   with the iMatix c script without any arguments.
#
.c$(OBJ):
	$(CC) $(DBG) $<

#
#   Default rule -- all depends on everything.
#   We do this so that we can define the rule at the end.
#
all: everything

# debug build
debug: DBG = -DDEBUG -g
debug: everything

#
#   The following targets are not files. Mark them as such.
#
.PHONY: all everything install clean
#
#   Dependencies
#
libgsl$(LIB): ggobjt$(OBJ) ggcomm$(OBJ) ggscop$(OBJ) ggcode$(OBJ) ggcodem$(OBJ) ggpars$(OBJ) ggparsm$(OBJ) ggconv$(OBJ) ggdiag$(OBJ) ggenvt$(OBJ) ggfile$(OBJ) gggsl$(OBJ) ggmath$(OBJ) ggpcre$(OBJ) ggproc$(OBJ) ggscrp$(OBJ) ggsock$(OBJ) ggstrn$(OBJ) ggsymb$(OBJ) ggthrd$(OBJ) ggtime$(OBJ) ggxml$(OBJ) sflbits$(OBJ) sflcvds$(OBJ) sflcvts$(OBJ) sflfile$(OBJ) sfllbuf$(OBJ) sflnode$(OBJ) sflsock$(OBJ) sfltron$(OBJ) smtlog$(OBJ) smtmsg$(OBJ) smtsimu$(OBJ) smttst2$(OBJ) sflcomp$(OBJ) sflcvns$(OBJ) sfldate$(OBJ) sflfind$(OBJ) sfllist$(OBJ) sflprint$(OBJ) sflsort$(OBJ) sfluid$(OBJ) smtoper$(OBJ) smtslot$(OBJ) smttst3$(OBJ) sflcons$(OBJ) sflcvsb$(OBJ) sfldes$(OBJ) sflfort$(OBJ) sflmail$(OBJ) sflproc$(OBJ) sflstr$(OBJ) sflxml$(OBJ) smtpipe$(OBJ) smtsmtp$(OBJ) smtxlog$(OBJ) sflconv$(OBJ) sflcvsd$(OBJ) sfldescr$(OBJ) sflheap$(OBJ) sflmath$(OBJ) sflscrit$(OBJ) sflsymb$(OBJ) sflxmll$(OBJ) smtpop$(OBJ) smtsock$(OBJ) sflcryp$(OBJ) sflcvsn$(OBJ) sfldir$(OBJ) sflhttp$(OBJ) sflmem$(OBJ) sflsearch$(OBJ) sflsyst$(OBJ) sflxmls$(OBJ) smtrdns$(OBJ) smttime$(OBJ) sflcvbs$(OBJ) sflcvst$(OBJ) sflenv$(OBJ) sflini$(OBJ) sflmesg$(OBJ) sflsha$(OBJ) sfltok$(OBJ) smtecho$(OBJ) smtrdnsl$(OBJ) smttran$(OBJ) sflcvdp$(OBJ) sflcvtp$(OBJ) sflexdr$(OBJ) sfllang$(OBJ) sflmime$(OBJ) sflslot$(OBJ) sfltree$(OBJ) smtlib$(OBJ) smtserv$(OBJ) smttst1$(OBJ)
	$(CC) -r libgsl$(LIB) ggobjt$(OBJ) ggcomm$(OBJ) ggscop$(OBJ) ggcode$(OBJ) ggcodem$(OBJ) ggpars$(OBJ) ggparsm$(OBJ) ggconv$(OBJ) ggdiag$(OBJ) ggenvt$(OBJ) ggfile$(OBJ) gggsl$(OBJ) ggmath$(OBJ) ggpcre$(OBJ) ggproc$(OBJ) ggscrp$(OBJ) ggsock$(OBJ) ggstrn$(OBJ) ggsymb$(OBJ) ggthrd$(OBJ) ggtime$(OBJ) ggxml$(OBJ) sflbits$(OBJ) sflcvds$(OBJ) sflcvts$(OBJ) sflfile$(OBJ) sfllbuf$(OBJ) sflnode$(OBJ) sflsock$(OBJ) sfltron$(OBJ) smtlog$(OBJ) smtmsg$(OBJ) smtsimu$(OBJ) smttst2$(OBJ) sflcomp$(OBJ) sflcvns$(OBJ) sfldate$(OBJ) sflfind$(OBJ) sfllist$(OBJ) sflprint$(OBJ) sflsort$(OBJ) sfluid$(OBJ) smtoper$(OBJ) smtslot$(OBJ) smttst3$(OBJ) sflcons$(OBJ) sflcvsb$(OBJ) sfldes$(OBJ) sflfort$(OBJ) sflmail$(OBJ) sflproc$(OBJ) sflstr$(OBJ) sflxml$(OBJ) smtpipe$(OBJ) smtsmtp$(OBJ) smtxlog$(OBJ) sflconv$(OBJ) sflcvsd$(OBJ) sfldescr$(OBJ) sflheap$(OBJ) sflmath$(OBJ) sflscrit$(OBJ) sflsymb$(OBJ) sflxmll$(OBJ) smtpop$(OBJ) smtsock$(OBJ) sflcryp$(OBJ) sflcvsn$(OBJ) sfldir$(OBJ) sflhttp$(OBJ) sflmem$(OBJ) sflsearch$(OBJ) sflsyst$(OBJ) sflxmls$(OBJ) smtrdns$(OBJ) smttime$(OBJ) sflcvbs$(OBJ) sflcvst$(OBJ) sflenv$(OBJ) sflini$(OBJ) sflmesg$(OBJ) sflsha$(OBJ) sfltok$(OBJ) smtecho$(OBJ) smtrdnsl$(OBJ) smttran$(OBJ) sflcvdp$(OBJ) sflcvtp$(OBJ) sflexdr$(OBJ) sfllang$(OBJ) sflmime$(OBJ) sflslot$(OBJ) sfltree$(OBJ) smtlib$(OBJ) smtserv$(OBJ) smttst1$(OBJ)
gsl$(OBJ): gsl.c ggpriv.h ggcomm.h ggpars.h ggscop.h ggcode.h ggobjt.h version.h sflbits.h sfldescr.h sflfind.h sflini.h sflmem.h sflprocx.h sflstr.h sfluid.h smthead.h smtserv.h sflcomp.h sfldes.h sflfort.h sfllang.h sflmesg.h sflsearch.h sflsymb.h sflxml.h smtlib.h smtsimu.h sflcons.h sfldir.h sfl.h sfllbuf.h sflmime.h sflsha.h sflsyst.h sflxmll.h smtmem.h smtsslm.h sflconv.h sflenv.h sflhead.h sfllist.h sflnode.h sflslot.h sfltok.h sflxmls.h smtmsg.h smttime.h sflcryp.h sflexdr.h sflheap.h sflmail.h sflprint.h sflsock.h sfltree.h smt3.h smtpriv.h smtxlog.h sfldate.h sflfile.h sflhttp.h sflmath.h sflproc.h sflsort.h sfltron.h smtdefn.h smtrdnsl.h
gsl$(EXE): gsl$(OBJ) libgsl$(LIB)
	$(CC) -L gsl
ggobjt$(OBJ):    ggobjt.c
ggcomm$(OBJ):    ggcomm.c
ggscop$(OBJ):    ggscop.c
ggcode$(OBJ):    ggcode.c
ggcodem$(OBJ):   ggcodem.c
ggpars$(OBJ):    ggpars.c
ggparsm$(OBJ):   ggparsm.c
ggconv$(OBJ):    ggconv.c
ggdiag$(OBJ):    ggdiag.c
ggenvt$(OBJ):    ggenvt.c
ggfile$(OBJ):    ggfile.c
gggsl$(OBJ):     gggsl.c
ggmath$(OBJ):    ggmath.c
ggpcre$(OBJ):    ggpcre.c
ggproc$(OBJ):    ggproc.c
ggscrp$(OBJ):    ggscrp.c
ggsock$(OBJ):    ggsock.c
ggstrn$(OBJ):    ggstrn.c
ggsymb$(OBJ):    ggsymb.c
ggthrd$(OBJ):    ggthrd.c
ggtime$(OBJ):    ggtime.c
ggxml$(OBJ):     ggxml.c
sflbits$(OBJ):   sflbits.c
sflcomp$(OBJ):   sflcomp.c
sflcons$(OBJ):   sflcons.c
sflconv$(OBJ):   sflconv.c
sflcryp$(OBJ):   sflcryp.c
sflcvbs$(OBJ):   sflcvbs.c
sflcvdp$(OBJ):   sflcvdp.c
sflcvds$(OBJ):   sflcvds.c
sflcvns$(OBJ):   sflcvns.c
sflcvsb$(OBJ):   sflcvsb.c
sflcvsd$(OBJ):   sflcvsd.c
sflcvsn$(OBJ):   sflcvsn.c
sflcvst$(OBJ):   sflcvst.c
sflcvtp$(OBJ):   sflcvtp.c
sflcvts$(OBJ):   sflcvts.c
sfldate$(OBJ):   sfldate.c
sfldes$(OBJ):    sfldes.c
sfldescr$(OBJ):  sfldescr.c
sfldir$(OBJ):    sfldir.c
sflenv$(OBJ):    sflenv.c
sflexdr$(OBJ):   sflexdr.c
sflfile$(OBJ):   sflfile.c
sflfind$(OBJ):   sflfind.c
sflfort$(OBJ):   sflfort.c
sflheap$(OBJ):   sflheap.c
sflhttp$(OBJ):   sflhttp.c
sflini$(OBJ):    sflini.c
sfllang$(OBJ):   sfllang.c
sfllbuf$(OBJ):   sfllbuf.c
sfllist$(OBJ):   sfllist.c
sflmail$(OBJ):   sflmail.c
sflmath$(OBJ):   sflmath.c
sflmem$(OBJ):    sflmem.c
sflmesg$(OBJ):   sflmesg.c
sflmime$(OBJ):   sflmime.c
sflnode$(OBJ):   sflnode.c
sflprint$(OBJ):  sflprint.c
sflproc$(OBJ):   sflproc.c
sflscrit$(OBJ):  sflscrit.c
sflsearch$(OBJ): sflsearch.c
sflsha$(OBJ):    sflsha.c
sflslot$(OBJ):   sflslot.c
sflsock$(OBJ):   sflsock.c
sflsort$(OBJ):   sflsort.c
sflstr$(OBJ):    sflstr.c
sflsymb$(OBJ):   sflsymb.c
sflsyst$(OBJ):   sflsyst.c
sfltok$(OBJ):    sfltok.c
sfltree$(OBJ):   sfltree.c
sfltron$(OBJ):   sfltron.c
sfluid$(OBJ):    sfluid.c
sflxml$(OBJ):    sflxml.c
sflxmll$(OBJ):   sflxmll.c
sflxmls$(OBJ):   sflxmls.c
smtecho$(OBJ):   smtecho.c
smtlib$(OBJ):    smtlib.c
smtlog$(OBJ):    smtlog.c
smtmsg$(OBJ):    smtmsg.c
smtoper$(OBJ):   smtoper.c
smtpipe$(OBJ):   smtpipe.c
smtpop$(OBJ):    smtpop.c
smtrdns$(OBJ):   smtrdns.c
smtrdnsl$(OBJ):  smtrdnsl.c
smtserv$(OBJ):   smtserv.c
smtsimu$(OBJ):   smtsimu.c
smtslot$(OBJ):   smtslot.c
smtsmtp$(OBJ):   smtsmtp.c
smtsock$(OBJ):   smtsock.c
smttime$(OBJ):   smttime.c
smttran$(OBJ):   smttran.c
smttst1$(OBJ):   smttst1.c
smttst2$(OBJ):   smttst2.c
smttst3$(OBJ):   smttst3.c
smtxlog$(OBJ):   smtxlog.c
everything: libgsl$(LIB) gsl$(EXE)
	./gsl -v
install:
	$(INSTALL) -m 755 -d $(bindir)
	$(INSTALL) -m 755 gsl $(bindir)
clean:
	rm *.o *.a gsl
