/*---------------------------------------------------------------------------
 *  ggxml.h - GSL/XML package header
 *
 *  Generated from ggxml by ggobjt.gsl using GSL/4.
 *  DO NOT MODIFY THIS FILE.
 *
 *  For documentation and updates see http://www.imatix.com.
 *---------------------------------------------------------------------------*/

#ifndef GGXML_INCLUDED
#define GGXML_INCLUDED
/*- Public definitions ------------------------------------------------------*/

void *
get_gsl_xml_item (XML_ITEM *xml_item);

XML_ITEM *
get_xml_item (void *gsl_xml_item);


/*- Global variables --------------------------------------------------------*/

extern CLASS_DESCRIPTOR
    XML_class;

extern CLASS_DESCRIPTOR
    XML_item_class;

extern CLASS_DESCRIPTOR
    XML_value_class;

/*- Function prototypes -----------------------------------------------------*/

int register_XML_classes (void);

int shutdown_XML_classes (void);

#endif
