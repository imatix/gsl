/*===========================================================================*
 *                                                                           *
 *  sflenv.c - Environment manipulation                                      *
 *                                                                           *
 *  Copyright (c) 1991-2010 iMatix Corporation                               *
 *                                                                           *
 *  ------------------ GPL Licensed Source Code ------------------           *
 *  iMatix makes this software available under the GNU General               *
 *  Public License (GPL) license for open source projects.  For              *
 *  details of the GPL license please see www.gnu.org or read the            *
 *  file license.gpl provided in this package.                               *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or            *
 *  modify it under the terms of the GNU General Public License as           *
 *  published by the Free Software Foundation; either version 2 of           *
 *  the License, or (at your option) any later version.                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public                *
 *  License along with this program in the file 'license.gpl'; if            *
 *  not, write to the Free Software Foundation, Inc., 59 Temple              *
 *  Place - Suite 330, Boston, MA 02111-1307, USA.                           *
 *                                                                           *
 *  You can also license this software under iMatix's General Terms          *
 *  of Business (GTB) for commercial projects.  If you have not              *
 *  explicitly licensed this software under the iMatix GTB you may           *
 *  only use it under the terms of the GNU General Public License.           *
 *                                                                           *
 *  For more information, send an email to info@imatix.com.                  *
 *  --------------------------------------------------------------           *
 *===========================================================================*/

#include "prelude.h"                    /*  Universal header file            */
#include "sflconv.h"                    /*  Conversion functions             */
#include "sflstr.h"                     /*  String functions                 */
#include "sflsymb.h"                    /*  Symbol-table functions           */
#include "sfllist.h"                    /*  Linked-list functions            */
#include "sflmem.h"                     /*  Memory-allocation functions      */
#include "sflenv.h"                     /*  Prototypes for functions         */


/*  ---------------------------------------------------------------------[<]-
    Function: env_get_string

    Synopsis: Translates the specified environment variable and returns a
    static string containing the value.  If the variable is not defined in
    the environment, returns the specified default value.  Note: if you
    want to use the value in a program you should use strdupl() to make a
    copy.  The environment variable name is always translated into upper
    case.  The default value may be NULL.

    Examples:
    config_file = strdupl (env_get_string ("config", "default.cfg"));
    ---------------------------------------------------------------------[>]-*/

char *
env_get_string (
    const char *name,
    const char *default_value)
{
    char
        *variable_name,
        *variable_value;

    variable_name = mem_strdup (name);
    variable_value = getenv (strupc (variable_name));
    mem_free (variable_name);
    return (variable_value? variable_value: (char *) default_value);
}


/*  ---------------------------------------------------------------------[<]-
    Function: env_get_number

    Synopsis: Translates the specified environment variable and returns the
    long numeric value of the string.  If the variable is not defined in
    the environment, returns the specified default value.  The environment
    variable name is always translated into upper case.

    Examples:
    max_retries = env_get_number ("retries", 5);
    ---------------------------------------------------------------------[>]-*/

long
env_get_number (
    const char *name,
    long default_value)
{
    char
        *variable_value;

    variable_value = env_get_string (name, NULL);
    return (variable_value? atol (variable_value): default_value);
}


/*  ---------------------------------------------------------------------[<]-
    Function: env_get_boolean

    Synopsis: Translates the specified environment variable and returns the
    Boolean value of the string.  If the variable is not defined in the
    environment, returns the specified default value. The environment
    variable name is always translated into upper case.  The environment
    variable value is interpreted irrespective to upper/lower case, and
    looking at the first letter only.  T/Y/1 are TRUE, everything else is
    FALSE.  See conv_str_bool() for the conversion rules.

    Examples:
    enforce_security = env_get_number ("security", FALSE);
    ---------------------------------------------------------------------[>]-*/

Bool
env_get_boolean (
    const char *name,
    Bool default_value)
{
    char
        *variable_value;

    variable_value = env_get_string (name, NULL);
    return (variable_value?
           (conv_str_bool (variable_value) != 0): default_value);
}


/*  ---------------------------------------------------------------------[<]-
    Function: env2descr

    Synopsis: Returns a DESCR pointer containing the current process
    environment strings.  The descriptor is allocated using mem_alloc();
    you should use mem_free() to deallocate when you are finished.  Returns
    NULL if there was not enough memory to allocate the descriptor.
    ---------------------------------------------------------------------[>]-*/

DESCR *
env2descr (void)
{
    return (strt2descr (environ));
}


/*  ---------------------------------------------------------------------[<]-
    Function: descr2env

    Synopsis: Returns an environment block from the supplied descriptor
    data.  The returned block is an array of strings, terminated by a null
    pointer.  Each string is allocated independently using mem_alloc().
    Returns NULL if there was not enough memory to allocate the block.
    ---------------------------------------------------------------------[>]-*/

char **
descr2env (
    const DESCR *descr)
{
    return (descr2strt (descr));
}

/*  ---------------------------------------------------------------------[<]-
    Function: env2symb

    Synopsis: Creates a symbol table containing the processes environment
    variables.  Each variable is stored as a name plus value.  The names
    of the variables are converted to upper case prior to being put into
    the symbol table.  You can destroy the symbol table using
    sym_delete_table() when you are finished with it.
    ---------------------------------------------------------------------[>]-*/

SYMTAB *
env2symb (void)
{
    SYMTAB
        *symtab;                        /*  Allocated symbol table           */
    char
        *next_entry,                    /*  Environment variable + value     */
        *equals;                        /*  Position of '=' in string        */
    int
        string_nbr;                     /*  Index into string table          */

    /*  We create the table here, instead of passing through strt2symb(),
        since we have to ensure that environment variable names are stored
        in uppercase.  Some systems (NT) return mixed-case names.            */

    symtab = sym_create_table ();
    if (symtab)
      {
        for (string_nbr = 0; environ [string_nbr]; string_nbr++)
          {
            next_entry = mem_strdup (environ [string_nbr]);
            equals = strchr (next_entry, '=');
            if (equals)
              {
                *equals = '\0';         /*  Cut into two strings             */
                strupc (next_entry);
                sym_assume_symbol (symtab, next_entry, equals + 1);
              }
            mem_free (next_entry);
          }
      }
    return (symtab);
}


/*  ---------------------------------------------------------------------[<]-
    Function: symb2env

    Synopsis: Returns an environment block from the supplied symbol table.
    The returned block is an array of strings, terminated by a null
    pointer.  Each string is allocated independently using mem_alloc().
    Returns NULL if there was not enough memory to allocate the block.
    Normalises the environment variable names as follows: converts all
    letters to uppercase, and non-alphanumeric characters to underlines.
    To free the array, use strtfree().  See also symb2strt().
    ---------------------------------------------------------------------[>]-*/

char **
symb2env (
    const SYMTAB *symtab)
{
    MEMTRN
        *memtrn;                        /*  Memory transation                */
    SYMBOL
        *symbol;                        /*  Pointer to symbol                */
    char
        **strings,                      /*  Returned string array            */
        *name_and_value,                /*  Name=value string                */
        *nameptr;                       /*  Pointer into name                */
    int
        string_nbr;                     /*  Index into symbol_array          */

    if (!symtab)
        return (NULL);                  /*  Return NULL if argument is null  */

    /*  Allocate the array of pointers with one slot for the final NULL      */
    memtrn  = mem_new_trans ();
    strings = memt_alloc (memtrn, sizeof (char *) * (symtab-> size + 1));
    if (strings)
      {
        string_nbr = 0;
        for (symbol = symtab-> symbols; symbol; symbol = symbol-> next)
          {
            /*  Allocate space for "name=value" plus final null char         */
            name_and_value = memt_alloc (memtrn,
                                        (strlen (symbol-> name)
                                         + strlen (symbol-> value) + 2));
            if (!name_and_value)        /*  Quit if no memory left           */
              {
                mem_rollback (memtrn);
                return (NULL);
              }
            /*  Get symbol name in uppercase, using underlines               */
            strcpy (name_and_value, symbol-> name);
            for (nameptr = name_and_value; *nameptr; nameptr++)
                if (isalnum (*nameptr))
                    *nameptr = toupper (*nameptr);
                else
                    *nameptr = '_';
            strcat (name_and_value, "=");
            strcat (name_and_value, symbol-> value);
            strings [string_nbr++] = name_and_value;
          }
        strings [string_nbr] = NULL;    /*  Store final null pointer         */
      }
    mem_commit (memtrn);
    return (strings);
}


/*  ---------------------------------------------------------------------[<]-
    Function: env_copy

    Synopsis: Returns an environment block which is a copy of the supplied
    environment block, in all new memory.  If no environment block is
    supplied the current process environment is copied.  The returned block
    is an array of strings, terminated by a null pointer.  Each string is
    allocated independently using mem_alloc().  Returns NULL if there was
    not enough memory to allocate the block.  No changes are made to the
    strings during copying. To free the array, use strtfree().
    ---------------------------------------------------------------------[>]-*/

char **
env_copy (
    char **environment)
{
    MEMTRN
        *memtrn;                        /*  Memory transation                */
    char
        **env = environment,            /*  Environment to copy              */
        **newenv = NULL;                /*  Copy of environment              */
    int
        size  = 0,     
        pos = 0;

    if (env == NULL)
        env = environ;        /*  Default is to copy the process environment */

    /*  Count the size of the environment                                    */
    for (size = 0; env [size] != NULL; env++)
        ;  /* EMPTY BODY */

    memtrn = mem_new_trans ();
    if (!memtrn)
        return NULL;

    newenv = memt_alloc (memtrn, ((size+1) * sizeof(char *)));
    if (!newenv)
      {
        mem_rollback (memtrn);
        return NULL;
      }

    for (pos = 0; pos < size; pos++)
      {
        newenv [pos] = memt_strdup (memtrn, env [pos]);
        if (newenv [pos] == NULL)
          {
            mem_rollback (memtrn);
            return NULL;
          }
      }  
    newenv [pos] = NULL;               /*  Terminate the array               */
    mem_commit (memtrn);               /*  Commit the memory allocations     */
   
    return newenv;
}


/*  -------------------------------------------------------------------------
 *  find_in_env
 *
 *  Find the location of the environment variable "name" within the 
 *  array of environment variables, and if found return the pointer to its
 *  entry in the array.  Otherwise return NULL.
 */

static char **
find_in_env (
    const char *name )
{
    int
        name_len  = -1;
    char
        **name_loc  = NULL;
    char
        *equal_pos = NULL;

    ASSERT (name);
    if (!name)
        return (NULL);

    name_len = strlen(name);
    equal_pos = strchr (name, '=');              /*  Ignore trailing =       */
    if (equal_pos)
        name_len = equal_pos - name;

    for (name_loc = environ; *name_loc; ++name_loc)
      {
        if (strncmp(*name_loc, name, name_len) == 0)
            return name_loc;
      }
    return (NULL);                               /*  Not in environment      */
}


/*  ---------------------------------------------------------------------[<]-
    Function: env_set

    Synopsis: If the environment variable "name" is not in the environment
    add name=value to the environment.  If the environment variable "name"
    is in the environment, and overwrite is true (1), then replace the 
    existing value of "name" with "name=value".  Otherwise (environment 
    variable "name" is in the enviornment, overwrite is false (0)) do nothing.
    On most platforms, simply calls the system setenv() function.  Note: we
    use malloc() to ensure that all of the environment is allocated with the
    same memory manager.  We also don't free the memory of things we overwrite,
    because we cannot tell how they were allocated.  This is a memory leak,
    but hopefully a minor one.  Returns 0 on success, -1 if could not add to
    environment. Safety: Not thread safe, buffer safe.
    ---------------------------------------------------------------------[>]-*/

int
env_set (
    const char *name, 
    const char *value, 
    int         overwrite)
{
    int
        name_len  = -1,
        value_len = -1;
    char
        **name_loc = NULL,
        *equal_pos = NULL;

    ASSERT (name);
    ASSERT (value);
    if (!name || !value)
        return (-1);

    name_len  = strlen (name);
    value_len = strlen (value);
    name_loc  = find_in_env (name);
    if (name_loc) 
      {
        /*  Name is already in the environment                               */
        if (!overwrite)
            return 0;

        /*  We are to replace the value, possibly we can just overwrite the  */
        /*  existing string, if there is enough space for it.                */
        if ((int) strlen (*name_loc) >= (name_len + 1 + value_len))
          {
            strncpy (((*name_loc) + name_len + 1), value, value_len);
            *((*name_loc) + name_len + value_len + 1) = '\0';
            return (0);
          }
        else
            *name_loc = NULL;                    /*  Throw out old value     */
      }
    else
      {
        /*  Name is not already in the environment, have to make the         */
        /*  environment bigger to allow space for it.                        */
        int   env_entries = 0;
        char **tmp_env   = NULL;

        for (name_loc = environ; *name_loc; ++name_loc, ++env_entries)
            ;

        tmp_env = malloc ((env_entries + 2) * sizeof (char *));
        if (!tmp_env)
            return -1;

        memcpy (tmp_env, environ, (env_entries * sizeof (char *)));

        tmp_env [env_entries] = NULL;            /*  For our new entry       */
        name_loc = (tmp_env + env_entries);
        tmp_env [env_entries + 1] = NULL;        /*  Terminate array         */

        environ = tmp_env;
      }

    /*  Now name_loc points at an entry that we can use, so we just need     */
    /*  to allocate memory for it, and fill in the name=value string.        */
    equal_pos = strchr (name, '=');              /*  Ignore trailing =       */
    if (equal_pos)
        name_len = equal_pos - name;

    *name_loc = malloc (name_len + value_len + 2);
    if (! *name_loc)
        return (-1);

    strncpy(*name_loc, name, name_len);
    *((*name_loc) + name_len) = '=';
    strncpy (((*name_loc) + name_len + 1), value, value_len);
    *((*name_loc) + name_len + value_len + 1) = '\0';

    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: env_clear

    Synopsis: Remove all occurances of the environment variable "name" from
    the environment.  On most systems, calls the standard unsetenv() function.
    Safety: Not thread safe, buffer safe.
    ---------------------------------------------------------------------[>]-*/

void
env_clear (
    const char *name)
{
    char
        **name_loc  = NULL;

    ASSERT (name);
    if (!name)
        return;

    while ((name_loc = find_in_env (name)))
      {
        /*  Found name in the environment, shuffle things along so that      */
        /*  it is no longer there.                                           */
        for (; *name_loc; ++name_loc)
            *name_loc = *(name_loc + 1);
      }
}

