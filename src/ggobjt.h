/*===========================================================================*
 *                                                                           *
 *  ggobjt.h - Object functions                                              *
 *                                                                           *
 *  Copyright (c) 1996-2010 iMatix Corporation                               *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation; either version 3 of the License, or (at    *
 *  your option) any later version.                                          *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful, but      *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  General Public License for more details.                                 *
 *                                                                           *
 *  For information on alternative licensing for OEMs, please contact        *
 *  iMatix Corporation.                                                      *
 *                                                                           *
 *===========================================================================*/

#ifndef GGOBJT_INCLUDED                 /*  Allow multiple inclusions        */
#define GGOBJT_INCLUDED

/*- Type definitions --------------------------------------------------------*/


/*- Global variables --------------------------------------------------------*/

extern char
    object_error [LINE_MAX + 1];

/*- Functions ---------------------------------------------------------------*/

void          initialise_objects     (void);
void          destroy_objects        (void);
int           object_register        (PLUGIN_INITIALISE *init,
                                      function          shutdown);
int           initialise_classes     (THREAD *thread);
GSL_FUNCTION *locate_method          (CLASS_DESCRIPTOR *class,
                                      const char *name);
int           build_method_arguments (SCRIPT_NODE *fn_node,
                                      RESULT_NODE ***arg);
Bool          arguments_are_defined  (int argc,
                                      RESULT_NODE **argv,
                                      RESULT_NODE *result);
void         *get_class_item         (THREAD *gsl_thread,
                                      const char *class);

/*---------------------------------------------------------------------------*/

#endif
