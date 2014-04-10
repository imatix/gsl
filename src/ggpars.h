/*===========================================================================*
 *                                                                           *
 *  ggpars.h - Script parser functions                                       *
 *                                                                           *
 *  Copyright (c) 1996-2010 iMatix Corporation                               *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation; either version 2 of the License, or (at    *
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

#ifndef GGPARS_INCLUDED                 /*  Allow multiple inclusions        */
#define GGPARS_INCLUDED

/*- Type definitions --------------------------------------------------------*/

typedef        void *              JOBID;

/*  Function type for function to read next line of script                   */

typedef Bool (SCRIPT_READ) (JOBID job, char *text);

/*- Globals that control parsing symbols ------------------------------------*/

char g_esc_symbol;               /*  By default, '\\'                        */
char g_sub_symbol;               /*  By default, '$'                         */

/*- Macros ------------------------------------------------------------------*/

/*  Prototypes  */
void         gg_free             (SCRIPT_NODE *node);
XML_ITEM    *gg_xml              (SCRIPT_NODE *node);
char        *operator_text       (OPERATOR op);
char        *node_type_string    (SCRIPT_NODE_TYPE type);

void         init_script_node    (SCRIPT_NODE *node);

int          ggpars_init         (void);
int          gg_parse_template   (SCRIPT_READ *read,  JOBID job,
                                  QUEUE       *replyqueue);
int          gg_parse_gsl        (SCRIPT_READ *read,  JOBID job,
                                  QUEUE       *replyqueue);
int          gg_parse_expression (char        *expression,  JOBID job,
                                  QUEUE       *replyqueue);
int          ggpars_term         (void);

/*---------------------------------------------------------------------------*/

#endif
