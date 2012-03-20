#! /bin/bash
#   Builds pcre library on Linux
#

./c -q -l dftables
./dftables pcre_chartables.c

./c pcre_chartables
./c pcre_compile
./c pcre_config
./c pcre_dfa_exec
./c pcre_exec
./c pcre_fullinfo
./c pcre_get
./c pcre_globals
./c pcre_info
./c pcre_maketables
./c pcre_ord2utf8
./c pcre_refcount
./c pcre_study
./c pcre_tables
./c pcre_try_flipped
./c pcre_ucp_searchfuncs
./c pcre_valid_utf8
./c pcre_version
./c pcre_xclass

./c -r libpcre pcre_chartables
./c -r libpcre pcre_compile
./c -r libpcre pcre_config
./c -r libpcre pcre_dfa_exec
./c -r libpcre pcre_exec
./c -r libpcre pcre_fullinfo
./c -r libpcre pcre_get
./c -r libpcre pcre_globals
./c -r libpcre pcre_info
./c -r libpcre pcre_maketables
./c -r libpcre pcre_ord2utf8
./c -r libpcre pcre_refcount
./c -r libpcre pcre_study
./c -r libpcre pcre_tables
./c -r libpcre pcre_try_flipped
./c -r libpcre pcre_ucp_searchfuncs
./c -r libpcre pcre_valid_utf8
./c -r libpcre pcre_version
./c -r libpcre pcre_xclass

./c -l pcregrep
./c -l pcredemo
./c -l pcretest

echo -n "Install PCRE to /usr/local? (y/n) "
read answer
if [ $answer == "y" ]; then
    echo "Installing pcre.h into /usr/local/include..."
    cp pcre.h /usr/local/include
    echo "Installing libpcre.a into /usr/local/lib..."
    cp libpcre.a /usr/local/lib
fi
