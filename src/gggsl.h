/*---------------------------------------------------------------------------
 *  gggsl.h - GSL/gsl control package header
 *
 *  Generated from gggsl by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#ifndef GGGSL_INCLUDED
#define GGGSL_INCLUDED
/*- Public definitions ------------------------------------------------------*/

typedef struct {
    int
        links;
    Bool
        ignorecase,                    /*  = use case for pretty-print       */
        cobol,                         /*  COBOL = 6 char line numbers       */
        template;                      /*  Template mode?                    */
    SCRIPT_LINE
        *line;                         /*  script_line item for current line */
    int
        shuffle;                       /*  Min whitespace size for shuffle   */
    char
        *terminator;                   /*  Line terminator string            */
    int
        argc;                          /*  Command line arguments            */
    char
        **argv;
    SYMTAB_ITEM
        *switches;                     /*  Original command-line switches    */
    char
        *output_file,                  /*  Output file name                  */
        *input_file;                   /*  Input file name                   */
    long
        output_line;                   /*  Line number in output file.       */
} GSL_CONTROL;

extern char
    *me,                               /*  Module name                       */
    *version;                          /*  GSL version                       */


/*- Global variables --------------------------------------------------------*/

extern CLASS_DESCRIPTOR
    gsl_class;

/*- Function prototypes -----------------------------------------------------*/

int register_gsl_classes (void);


#endif
