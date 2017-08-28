#
# Makefile for bundled PCRE in iMatix GSL/4.1
#
# Copyright (C) 2017 by the GSL community, based on GSL's own Makefile
# and earlier ./build.sh script.
#
# This file is licensed under the BSD license as follows:
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.
# * Neither the name of iMatix Corporation nor the names of its
# contributors may be used to endorse or promote products derived
# from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY IMATIX CORPORATION "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL IMATIX CORPORATION BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

INSTALL = /usr/bin/install -c

# Installation directories, good for Debian Packaging
DESTDIR ?= /usr/local
prefix = ${DESTDIR}
mandir = ${prefix}/share/man/man8
bindir = ${prefix}/bin
libdir = ${prefix}/lib
incdir = ${prefix}/include

RM ?= rm -f
CP ?= cp -f

# Default values for object, library, and executable extensions.
#
OBJ = .o
LIB = .a
EXE =
DBG =
CC = ./c -q
CXX = ./c -q -p
CCLIBS =

# Reset the suffixes that will be considered to just our own list.
#
# Make programs use the .SUFFIXES psuedo rule for this

.SUFFIXES:
.SUFFIXES: $(EXE) $(LIB) $(OBJ) .c .cc .cpp .d .i

# Objects depend on source files with the same name; and are compiled
# with the iMatix c script without any arguments.
#
.c$(OBJ):
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DBG) $<

.cpp$(OBJ):
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DBG) $<

# Original project has .cc files but the ./c script handles .cpp files
.cc.cpp:
	$(CP) "$<" "$@"

#
# Default rule -- all depends on everything.
# We do this so that we can define the rule at the end.
#
all: everything

# debug build
debug: DBG = -DDEBUG -g
debug: everything

#
# The following targets are not files. Mark them as such.
#
.PHONY: all everything install clean
#
# Dependencies
#

dftables$(OBJ): dftables.c
pcre_chartables$(OBJ): pcre_chartables.c
pcre_compile$(OBJ): pcre_compile.c
pcre_config$(OBJ): pcre_config.c
pcre_dfa_exec$(OBJ): pcre_dfa_exec.c
pcre_exec$(OBJ): pcre_exec.c
pcre_fullinfo$(OBJ): pcre_fullinfo.c
pcre_get$(OBJ): pcre_get.c
pcre_globals$(OBJ): pcre_globals.c
pcre_info$(OBJ): pcre_info.c
pcre_maketables$(OBJ): pcre_maketables.c
pcre_ord2utf8$(OBJ): pcre_ord2utf8.c
pcre_refcount$(OBJ): pcre_refcount.c
pcre_study$(OBJ): pcre_study.c
pcre_tables$(OBJ): pcre_tables.c
pcre_try_flipped$(OBJ): pcre_try_flipped.c
pcre_ucp_searchfuncs$(OBJ): pcre_ucp_searchfuncs.c
pcre_valid_utf8$(OBJ): pcre_valid_utf8.c
pcre_version$(OBJ): pcre_version.c
pcre_xclass$(OBJ): pcre_xclass.c
pcredemo$(OBJ): pcredemo.c
pcregrep$(OBJ): pcregrep.c
pcreposix$(OBJ): pcreposix.c
pcretest$(OBJ): pcretest.c
ucptable$(OBJ): ucptable.c

pcre_scanner_unittest$(OBJ): pcre_scanner_unittest.cpp
pcre_scanner$(OBJ): pcre_scanner.cpp
pcre_stringpiece_unittest$(OBJ): pcre_stringpiece_unittest.cpp
pcre_stringpiece$(OBJ): pcre_stringpiece.cpp
pcrecpp_unittest$(OBJ): pcrecpp_unittest.cpp
pcrecpp$(OBJ): pcrecpp.cpp

pcre_scanner_unittest.cpp: pcre_scanner_unittest.cc
pcre_scanner.cpp: pcre_scanner.cc
pcre_stringpiece_unittest.cpp: pcre_stringpiece_unittest.cc
pcre_stringpiece.cpp: pcre_stringpiece.cc
pcrecpp_unittest.cpp: pcrecpp_unittest.cc
pcrecpp.cpp: pcrecpp.cc

BUILD_TOOLS = dftables$(EXE)
BUILD_PROGS = pcredemo$(EXE) pcregrep$(EXE) pcretest$(EXE)
BUILD_LIBS = libpcre$(LIB)

# These require to be `configure`'d and are not required for GSL usage.
# So there is basic support for build if someone wants to extend it
# later, but not worth the effort for GSL bundling use-case.
BUILD_CXX_TESTS = pcrecpp_unittest$(EXE) pcre_stringpiece_unittest$(EXE) pcre_scanner_unittest$(EXE)

everything: $(BUILD_TOOLS) $(BUILD_LIBS) $(BUILD_PROGS)
	@echo "Build products ($^) are up to date now"

dftables$(EXE): dftables$(OBJ)
	$(CC) -l $@

pcrecpp_unittest$(EXE): pcrecpp_unittest$(OBJ) pcrecpp$(OBJ)

pcre_stringpiece_unittest$(EXE): pcre_stringpiece_unittest$(OBJ) pcre_stringpiece$(OBJ)

pcre_scanner_unittest$(EXE): pcre_scanner_unittest$(OBJ) pcre_scanner$(OBJ)

$(BUILD_PROGS): $(BUILD_LIBS)
	$(CC) -l "`basename "$@" "$(EXE)"`"

$(BUILD_CXX_TESTS): $(BUILD_LIBS)
	$(CXX) -l "`basename "$@" "$(EXE)"`"

pcre_chartables.c: dftables$(EXE)
	./dftables$(EXE) pcre_chartables.c

LIBPCRE_DEPS = \
    pcre_chartables$(OBJ) \
    pcre_compile$(OBJ) \
    pcre_config$(OBJ) \
    pcre_dfa_exec$(OBJ) \
    pcre_exec$(OBJ) \
    pcre_fullinfo$(OBJ) \
    pcre_get$(OBJ) \
    pcre_globals$(OBJ) \
    pcre_info$(OBJ) \
    pcre_maketables$(OBJ) \
    pcre_ord2utf8$(OBJ) \
    pcre_refcount$(OBJ) \
    pcre_study$(OBJ) \
    pcre_tables$(OBJ) \
    pcre_try_flipped$(OBJ) \
    pcre_ucp_searchfuncs$(OBJ) \
    pcre_valid_utf8$(OBJ) \
    pcre_version$(OBJ) \
    pcre_xclass$(OBJ)

libpcre$(LIB): $(LIBPCRE_DEPS)
	@for F in $^ ; do \
	    B="`basename "$$F" "$(OBJ)"`" && \
	    echo "./c -r libpcre $$B" && \
	    ./c -r libpcre "$$B" ; \
	 done

install:
	$(INSTALL) -m 755 -d "$(libdir)"
	$(INSTALL) -m 755 -d "$(incdir)"
	$(INSTALL) -m 755 pcre.h "$(incdir)"
	$(INSTALL) -m 755 pcrecpp.h "$(incdir)"
	$(INSTALL) -m 755 libpcre.a "$(libdir)"

uninstall:
	$(RM) "$(incdir)/pcre.h"
	$(RM) "$(incdir)/pcrecpp.h"
	$(RM) "$(libdir)/libpcre.a"
	@for D in $(bindir) $(incdir) $(libdir) ; \ do
	    if test -d "$$D" ; then \
	        if test `find "$$D" | wc -l` -le 1 ; then \
	            echo "$(RM) -r $$D" >&2 ; \
	            $(RM) -r "$$D" ; \
	        else true; fi ; \
	    else true ; fi; \
	 done

clean:
	$(RM) *.$(OBJ) *.$(LIB) *.o *.a *.so *.cpp $(BUILD_PROGS) $(BUILD_LIBS) $(BUILD_TOOLS) $(BUILD_CXX_TESTS)
