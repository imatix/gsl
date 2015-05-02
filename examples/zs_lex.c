/*  =========================================================================
    zs_lex - the ZeroScript lexer

    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of the ZeroScript language, http://zeroscript.org.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

/*
@header
    The lexer breaks an input stream into words, which are function
    compositions and invocations, strings, numbers, and open or close
    lists. It does not validate any semantics.
@discuss
    Functions start with a letter and if followed by ':' are treated
    as composition, else invocation.

    Strings are quoted by < and >.
    
    Lists start with ( and end with ).

    Accepts a wide range of numeric expressions:
        All digits
        A single period at start, or embedded in number
        Commas, used for thousand seperators, in the right place
        +- as unary sign operators
        +-/:*x^v binary operators, evaluated ^v then *x/: then +-
        [0-9]+[eE][+-]?[0-9]+ used once as exponent
        Ki Mi Gi Ti Pi Ei used as suffix
        h k M G T P E Z Y used as suffix
        d c m u n p f a z y used as suffix
@end
*/

#include "zs_lex.h"             //  Our class API
#include "zs_lex_fsm.h"         //  Finite state machine engine

//  Structure of our class

struct _zs_lex_t {
    fsm_t *fsm;                 //  Our finite state machine
    event_t events [256];       //  Map characters to events
    const char *input;          //  Line of text we're parsing
    const char *input_ptr;      //  Next character to process
    uint token_size;            //  Size of token so far
    zs_lex_token_t type;        //  Token type
    char token [1025];          //  Current token, max size 1K
    char current;               //  Current character
};

static void
s_set_events (zs_lex_t *self, const char *chars, event_t event)
{
    while (*chars)
        self->events [(uint) *chars++] = event;
}

//  ---------------------------------------------------------------------------
//  Create a new zs_lex, return the reference if successful, or NULL
//  if construction failed due to lack of available memory.

zs_lex_t *
zs_lex_new (void)
{
    zs_lex_t *self = (zs_lex_t *) zmalloc (sizeof (zs_lex_t));
    if (self) {
        self->fsm = fsm_new (self);
        uint char_nbr;
        self->events [0] = finished_event;
        for (char_nbr = 1; char_nbr < 256; char_nbr++)
            self->events [char_nbr] = other_event;
        //  There are two ways to do this; either we define character
        //  classes that produce generic events depending on the current
        //  state (e.g. hyphen_event in function names, or minus_event in
        //  numbers), or else we define lower level events that the FSM
        //  sorts out. I've chosen the second design so decisions stay in
        //  the FSM.
        s_set_events (self, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", letter_event);
        s_set_events (self, "abcdefghijklmnopqrstuvwxyz", letter_event);
        s_set_events (self, "0123456789", digit_event);
        s_set_events (self, "-", hyphen_event);
        s_set_events (self, "+", plus_event);
        s_set_events (self, "/", slash_event);
        s_set_events (self, "_", underscore_event);
        s_set_events (self, ".", period_event);
        s_set_events (self, ",", comma_event);
        s_set_events (self, ":", colon_event);
        s_set_events (self, "*", asterisk_event);
        s_set_events (self, "^", caret_event);
        s_set_events (self, "%", percent_event);
        s_set_events (self, "<", open_quote_event);
        s_set_events (self, ">", close_quote_event);
        s_set_events (self, "(", open_list_event);
        s_set_events (self, ")", close_list_event);
        s_set_events (self, " \t", whitespace_event);
        s_set_events (self, "\n", newline_event);
    }
    return self;
}


//  ---------------------------------------------------------------------------
//  Destroy the zs_lex and free all memory used by the object.

void
zs_lex_destroy (zs_lex_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        zs_lex_t *self = *self_p;
        fsm_destroy (&self->fsm);
        free (self);
        *self_p = NULL;
    }
}


//  ---------------------------------------------------------------------------
//  Enable verbose tracing of lexer

void
zs_lex_verbose (zs_lex_t *self, bool verbose)
{
    fsm_set_animate (self->fsm, verbose);
}


//  ---------------------------------------------------------------------------
//  Start parsing buffer, return type of first token

zs_lex_token_t
zs_lex_first (zs_lex_t *self, const char *input)
{
    self->input = input;
    self->input_ptr = self->input;
    return zs_lex_next (self);
}


//  ---------------------------------------------------------------------------
//  Continue parsing buffer, return type of next token

zs_lex_token_t
zs_lex_next (zs_lex_t *self)
{
    parse_next_character (self);
    fsm_execute (self->fsm);
    return self->type;
}


//  ---------------------------------------------------------------------------
//  Return actual token value, if any

const char *
zs_lex_token (zs_lex_t *self)
{
    return self->token;
}


//  ---------------------------------------------------------------------------
//  Return position of last processed character in text

uint
zs_lex_offset (zs_lex_t *self)
{
    return (self->input_ptr - self->input);
}


//  *************************  Finite State Machine  *************************
//  These actions are called from the generated FSM code.

//  ---------------------------------------------------------------------------
//  start_new_token
//

static void
start_new_token (zs_lex_t *self)
{
    self->token_size = 0;
    self->type = zs_lex_null;
}


//  ---------------------------------------------------------------------------
//  store_the_character
//

static void
store_the_character (zs_lex_t *self)
{
    self->token [self->token_size++] = self->current;
    self->token [self->token_size] = 0;
}


//  ---------------------------------------------------------------------------
//  parse_next_character
//

static void
parse_next_character (zs_lex_t *self)
{
    self->current = *self->input_ptr;
    if (self->current)
        self->input_ptr++;      //  Don't advance past end of input
    fsm_set_next_event (self->fsm, self->events [(uint) self->current]);
}


//  ---------------------------------------------------------------------------
//  push_back_to_previous
//

static void
push_back_to_previous (zs_lex_t *self)
{
    //  This lets us handle tokens that are glued together
    if (self->input_ptr > self->input)
        self->input_ptr--;
}


//  ---------------------------------------------------------------------------
//  store_newline_character
//

static void
store_newline_character (zs_lex_t *self)
{
    self->current = '\n';
    store_the_character (self);
}


//  ---------------------------------------------------------------------------
//  have_function_token
//

static void
have_function_token (zs_lex_t *self)
{
    self->type = zs_lex_function;
}


//  ---------------------------------------------------------------------------
//  have_compose_token
//

static void
have_compose_token (zs_lex_t *self)
{
    self->type = zs_lex_compose;
}


//  ---------------------------------------------------------------------------
//  have_number_token
//

static void
have_number_token (zs_lex_t *self)
{
    self->type = zs_lex_number;
}


//  ---------------------------------------------------------------------------
//  have_string_token
//

static void
have_string_token (zs_lex_t *self)
{
    self->type = zs_lex_string;
}


//  ---------------------------------------------------------------------------
//  have_open_token
//

static void
have_open_token (zs_lex_t *self)
{
    self->type = zs_lex_open;
}


//  ---------------------------------------------------------------------------
//  have_close_token
//

static void
have_close_token (zs_lex_t *self)
{
    self->type = zs_lex_close;
}


//  ---------------------------------------------------------------------------
//  have_null_token
//

static void
have_null_token (zs_lex_t *self)
{
    self->type = zs_lex_null;
}


//  ---------------------------------------------------------------------------
//  have_invalid_token
//

static void
have_invalid_token (zs_lex_t *self)
{
    self->type = zs_lex_invalid;
}


//  ---------------------------------------------------------------------------
//  Return number of processing cycles used so far

uint64_t
zs_lex_cycles (zs_lex_t *self)
{
    return fsm_cycles (self->fsm);
}


//  ---------------------------------------------------------------------------
//  Selftest

void
zs_lex_test (bool verbose)
{
    printf (" * zs_lex: ");
    if (verbose)
        printf ("\n");

    //  @selftest
    zs_lex_t *lex = zs_lex_new ();
    zs_lex_verbose (lex, verbose);

    assert (zs_lex_first (lex, "1234") == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_null);
    assert (zs_lex_next (lex) == zs_lex_null);
    
    assert (zs_lex_first (lex, "1234 4567") == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_null);
    
    assert (zs_lex_first (lex, "<Hello, World>") == zs_lex_string);
    assert (zs_lex_next (lex) == zs_lex_null);
    
    assert (zs_lex_first (lex, "<Hello,>\n<World>") == zs_lex_string);
    assert (zs_lex_next (lex) == zs_lex_string);
    assert (zs_lex_next (lex) == zs_lex_null);
    
    assert (zs_lex_first (lex, "<Here is a long string") == zs_lex_null);
    assert (zs_lex_first (lex, " which continues over two lines>") == zs_lex_string);
    assert (zs_lex_next (lex) == zs_lex_null);
    
    assert (zs_lex_first (lex, "pi: ( 22/7 )") == zs_lex_compose);
    assert (zs_lex_next (lex) == zs_lex_open);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_close);
    assert (zs_lex_next (lex) == zs_lex_null);
    
    assert (zs_lex_first (lex, "twopi:( pi 2 times )") == zs_lex_compose);
    assert (zs_lex_next (lex) == zs_lex_open);
    assert (zs_lex_next (lex) == zs_lex_function);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_function);
    assert (zs_lex_next (lex) == zs_lex_close);
    assert (zs_lex_next (lex) == zs_lex_null);

    assert (zs_lex_first (lex, "something(22/7*2)") == zs_lex_function);
    assert (zs_lex_next (lex) == zs_lex_open);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_close);
    assert (zs_lex_next (lex) == zs_lex_null);

    assert (zs_lex_first (lex, "1 +1 -1 .1 0.1") == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_null);
    
    assert (zs_lex_first (lex, "3.141592653589793238462643383279502884197169") == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_null);
    
    assert (zs_lex_first (lex, "1/2 1:2 1024*1024 10^10 1v2 99:70") == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_null);

    assert (zs_lex_first (lex, "1E10 3.14e+000 1,000,000") == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_null);

    assert (zs_lex_first (lex, "2k 2M 2G 2T 2P 2E 2Z 2Y") == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_null);

    assert (zs_lex_first (lex, "2Ki 2Mi 2Gi 2Ti 2Pi 2Ei") == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_null);

    assert (zs_lex_first (lex, "2d 2c 2m 2u 2n 2p 2f 2a 2z 2y") == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_null);

    assert (zs_lex_first (lex, "2*3 2^64-1") == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_number);
    assert (zs_lex_next (lex) == zs_lex_null);

    //  Test various invalid tokens
    assert (zs_lex_first (lex, "[Hello, World>") == zs_lex_invalid);
    assert (zs_lex_first (lex, "<Hello,>?<World>") == zs_lex_string);
    assert (zs_lex_next (lex) == zs_lex_invalid);
    assert (zs_lex_first (lex, "echo ( some text }") == zs_lex_function);
    assert (zs_lex_next (lex) == zs_lex_open);
    assert (zs_lex_next (lex) == zs_lex_function);
    assert (zs_lex_next (lex) == zs_lex_function);
    assert (zs_lex_next (lex) == zs_lex_invalid);
    assert (zs_lex_next (lex) == zs_lex_null);
    assert (zs_lex_first (lex, ",1") == zs_lex_invalid);
    assert (zs_lex_first (lex, "1?2") == zs_lex_invalid);

    if (verbose)
        printf ("%ld cycles done\n", (long) zs_lex_cycles (lex));
    zs_lex_destroy (&lex);
    //  @end
    printf ("OK\n");
}
