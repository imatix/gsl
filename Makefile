#
# Makefile for iMatix GSL/4.1
#
# Basic Makefile to build GSL starting from project root directory,
# allowing a simple build against system or bundled libpcre.
#
# Copyright (C) 2017 by the GSL community
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

all: gsl

gsl: gsl-shared

pcre/libpcre.a:
	cd pcre && $(MAKE) libpcre.a

gsl-shared:
	cd src && $(MAKE) all && cp -f gsl $@

gsl-static: pcre/libpcre.a
	cd src && $(MAKE) CCLIBS="../pcre/libpcre.a" all && cp -f gsl $@

check install uninstall:
	cd src && $(MAKE) $@

clean:
	cd src && $(MAKE) $@
	cd pcre && $(MAKE) $@
