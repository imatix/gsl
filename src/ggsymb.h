/*---------------------------------------------------------------------------
 *  ggsymb.h - GSL/symb package header
 *
 *  Generated from ggsymb by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#ifndef GGSYMB_INCLUDED
#define GGSYMB_INCLUDED
/*- Public definitions ------------------------------------------------------*/

typedef struct {
    int
        links;
    char
        *name;
    SYMTAB
        *symtab;
    Bool
        done,
        loop;                           /*  To check for self-references     */
} SYMTAB_ITEM;


/*- Global variables --------------------------------------------------------*/

extern CLASS_DESCRIPTOR
    symb_class;

/*- Function prototypes -----------------------------------------------------*/

int register_symb_classes (void);


#endif
