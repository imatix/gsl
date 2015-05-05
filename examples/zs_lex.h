/*  =========================================================================
    zs_lex - the ZeroScript lexer

    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of the ZeroScript language, http://zeroscript.org.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

#ifndef ZS_LEX_H_INCLUDED
#define ZS_LEX_H_INCLUDED

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
typedef struct _zs_lex_t zs_lex_t;

typedef enum {
    zs_lex_function,        //  function name
    zs_lex_compose,         //  function name followed by :
    zs_lex_string,          //  string
    zs_lex_number,          //  number expression
    zs_lex_open,            //  open list
    zs_lex_close,           //  close list
    zs_lex_invalid,         //  Syntax error
    zs_lex_null,            //  Nothing to return
    zs_lex_tokens           //  Size of this set
} zs_lex_token_t;

    
//  @interface
//  Create a new zs_lex, return the reference if successful, or NULL
//  if construction failed due to lack of available memory.
zs_lex_t *
    zs_lex_new (void);

//  Destroy the zs_lex and free all memory used by the object.
void
    zs_lex_destroy (zs_lex_t **self_p);

//  Enable verbose tracing of lexer
void
    zs_lex_verbose (zs_lex_t *self, bool verbose);

//  Start parsing buffer, return type of first token
zs_lex_token_t
    zs_lex_first (zs_lex_t *self, const char *input);

//  Continue parsing buffer, return type of next token
zs_lex_token_t
    zs_lex_next (zs_lex_t *self);

//  Return actual token value, if any
const char *
    zs_lex_token (zs_lex_t *self);

//  Return position of last processed character in text
uint
    zs_lex_offset (zs_lex_t *self);

//  Return number of processing cycles used so far
uint64_t
    zs_lex_cycles (zs_lex_t *self);

//  Self test of this class
void
    zs_lex_test (bool animate);
//  @end

#ifdef __cplusplus
}
#endif

#endif
