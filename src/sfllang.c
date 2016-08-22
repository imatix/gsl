/*===========================================================================*
 *                                                                           *
 *  sfllang.c - Multilingual date/time/number representation                 *
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
 *  published by the Free Software Foundation; either version 3 of           *
 *  the License, or (at your option) any later version.                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public                *
 *  License along with this program in the file 'license.gpl'; if            *
 *  not, see <http://www.gnu.org/licenses/>.                                 *
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
#include "sflstr.h"                     /*  String functions                 */
#include "sfldate.h"                    /*  Date functions                   */
#include "sfllist.h"                    /*  Memory-allocation functions      */
#include "sflmem.h"                     /*  Memory-allocation functions      */
#include "sfllang.h"                    /*  Function prototypes              */

#undef  __USE_UNICODE__
#if (defined (__USE_UNICODE__))
#   include "sflunic.h"*/
#endif

/* Use national characters                                                   */

#define __USE_NATIONAL_CHARS__


/* Support Unicode                                                           */

#if (defined (__USE_UNICODE__))
    typedef UCODE           uschar_t;
#   define  uschar_size     UCODE_SIZE
#   define  T(data)         L ## data
#   define STRING_COPY(dest, src)      wcscpy ((dest), (src))
#   define STRING_CMP(str1, str2)      wcscmp ((str1), (str2))
#else
    typedef char            uschar_t;
#   define  uschar_size     1
#   define  T(data)         data
#   define STRING_COPY(dest, src)      strcpy ((dest), (src))
#   define STRING_CMP(str1, str2)      strcmp ((str1), (str2))
#endif

/*  Constants                                                                */

#define GROUP_SIZE          80


/*  Macros                                                                   */

#define GET_GROUP1(n)               (int) (((n) % 1000000000L) / 1000000L)
#define GET_GROUP2(n)               (int) (((n) % 1000000L) / 1000L)
#define GET_GROUP3(n)               (int) ( (n) % 1000L)

#define GET_GROUP_HUNDREDS(g)       (int) (((g) % 1000L) / 100L)
#define GET_GROUP_TENS_ONES(g)      (int) ( (g) % 100L)
#define GET_GROUP_TENS(g)           (int) (((g) % 100L) / 10L)
#define GET_GROUP_ONES(g)           (int) ( (g) % 10L)


/*  Arrays                                                                   */

static char
    *language_str [] = {
        "--",
        "DA", "DE", "EN", "ES", "FB", "FR",
        "IS", "IT", "NL", "NO", "PO", "SV"
    };


static char
    *DA_months [] = {
        "januar", "februar", "marts", "april", "maj", "juni",
        "juli", "august", "september", "oktober", "november", "december"
    },
    *DE_months [] = {
        "Januar", "Februar", "Marsch", "April", "Mai", "Juni",
        "Juli", "August", "September", "Oktober", "November", "Dezember"
    },
    *EN_months [] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    },
    *ES_months [] = {
        "enero", "febrero", "marzo", "abril", "mayo", "junio",
        "julio", "agosto", "setiembre", "octubre", "noviembre", "diciembre"
    },
    *FR_months [] = {
        "janvier", "fevrier", "mars", "avril", "mai", "juin",
        "juillet", "ao/Cut", "septembre", "octobre", "novembre", "decembre"
    },
    *IS_months [] = {
        "januar", "februar", "marz", "april", "mai", "juni",
        "juli", "agust", "september", "oktober", "november", "desember"
    },
    *IT_months [] = {
        "gennaio", "febbraio", "marzo", "aprile", "maggio", "giugno",
        "luglio", "agosto", "settembre", "ottobre", "novembre", "dicembre"
    },
    *NL_months [] = {
        "januari", "februari", "mars", "april", "mei", "juni",
        "juli", "augustus", "september", "oktober", "november", "december"
    },
    *NO_months [] = {
        "januar", "februar", "mars", "april", "mai", "juni",
        "juli", "august", "september", "oktober", "november", "desember"
    },
    *PO_months [] = {
        "janeiro", "fevereiro", "mar‡o", "abril", "maio", "junho",
        "julho", "agosto", "setembro", "outubro", "novembro", "dezembro"
    },
    *SV_months [] = {
        "januari", "februari", "mars", "april", "maj", "juni",
        "juli", "augusti", "september", "oktober", "november", "december"
    };

static char
    *EN_days [] = {
        "Sunday", "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"
    },
    *FR_days [] = {
        "Dimanche", "Lundi", "Mardi", "Mercredi",
        "Jeudi", "Vendredi", "Samedi"
    },
    *NL_days [] = {
        "Zondag", "Maandag", "Dinsdag", "Woensdag",
        "Donderdag", "Vrijdag", "Zaterdag"
    };

static int
    RM_numeral [] = {
        1000, 900, 500, 400, 100, 90, 50,
          40,  10,   9,   5,   4,  1,  0
    };


static uschar_t
    *RM_text [] = {
        T("m"), T("dm"), T("d"), T("cd"), T("c"), T("xc"), T("l"),
        T("xl"), T("x"), T("ix"), T("v"), T("iv"), T("i"), T(" ")
   };


static uschar_t
    *IS_neuters [] = {
        T(""), T("eitt"), T("tvö"), T("thrju"), T("fjögur")
    };


static uschar_t
    *DA_units [] = {
        T("nul"), T("en"), T("to"), T("tre"), T("fire"), T("fem"), T("seks"), T("syv"),
        T("otte"), T("ni"), T("ti"), T("elve"), T("tolv"), T("tretten"), T("fjorten"),
        T("femten"), T("seksten"), T("sytten"), T("atten"), T("nitten")
    },
    *DE_units [] = {
        T("null"), T("ein"), T("zwei"), T("drei"), T("vier"), T("fünf"), T("sechs"),
        T("sieben"), T("acht"), T("neun"), T("zehn"), T("elf"), T("zwölf"),
        T("dreizehn"), T("vierzehn"), T("fünfzehn"), T("sechzehn"), T("siebzehn"),
        T("achtzehn"), T("neunzehn")
    },
    *EN_units [] = {
        T("zero"), T("one"), T("two"), T("three"), T("four"), T("five"), T("six"),
        T("seven"), T("eight"), T("nine"), T("ten"), T("eleven"), T("twelve"),
        T("thirteen"), T("fourteen"), T("fifteen"), T("sixteen"), T("seventeen"),
        T("eighteen"), T("nineteen")
    },
    *ES_units [] = {
        T("zero"), T("uno"), T("dos"), T("tres"), T("cuatro"), T("cinco"), T("seis"),
        T("siete"), T("ocho"), T("nueve"), T("diez"), T("once"), T("doce"), T("trece"),
        T("catorce"), T("quince"), T("dieciseis"), T("diecisiete"), T("dieciocho"),
        T("diecinueve")
    },
    *FR_units [] = {
        T("zero"), T("un"), T("deux"), T("trois"), T("quatre"), T("cinq"), T("six"),
        T("sept"), T("huit"), T("neuf"), T("dix"), T("onze"), T("douze"), T("treize"),
        T("quatorze"), T("quinze"), T("seize"), T("dix-sept"), T("dix-huit"),
        T("dix-neuf")
    },
    *IS_units [] = {
        T("null"), T("einn"), T("tweir"), T("thrir"), T("fjorir"), T("fimm"), T("sex"),
        T("sjö"), T("atta"), T("niu"), T("tiu"), T("ellefu"), T("tolf"), T("threttan"),
        T("fjortan"), T("fimtan"), T("sextan"), T("seytjan"), T("atjan"), T("nitjan")
    },
    *IT_units [] = {
        T("zero"), T("uno"), T("due"), T("tre"), T("quattro"), T("cinque"), T("sei"),
        T("sette"), T("otto"), T("nove"), T("dieci"), T("undici"), T("dodici"),
        T("tredici"), T("quatrodici"), T("quindici"), T("sedici"), T("diciassette"),
        T("diciotto"), T("dicianove")
    },
    *NL_units [] = {
        T("nul"), T("een"), T("twee"), T("drie"), T("vier"), T("vijf"), T("zes"),
        T("zeven"), T("acht"), T("negen"), T("tien"), T("elf"), T("twaalf"),
        T("dertien"), T("veertien"), T("vijftien"), T("zestien"), T("zeventien"),
        T("achttien"), T("negentien")
    },
    *NO_units [] = {
        T("null"), T("en"), T("to"), T("tre"), T("fire"), T("fem"), T("seks"), T("syv"),
        T("åtte"), T("ni"), T("ti"), T("elleve"), T("tolv"), T("tretten"), T("fjorten"),
        T("femten"), T("seksten"), T("sytten"), T("atten"), T("nitten")
    },
    *PT_units [] = {
        T("zero"), T("um"), T("dois"), T("tres"), T("quatro"), T("cinco"), T("seis"),
        T("sete"), T("oito"), T("nove"), T("dez"), T("onze"), T("doze"), T("treze"),
        T("catorze"), T("quinze"), T("dezasseis"), T("dezassete"), T("dezoito"),
        T("dezanove")
    },
    *SV_units [] = {
        T("noll"), T("en"), T("två"), T("tre"), T("fyra"), T("fem"), T("sex"), T("sju"),
        T("åtta"), T("nio"), T("tio"), T("elva"), T("tolv"), T("tretton"), T("fjorton"),
        T("femton"), T("sexton"), T("sjutton"), T("atton"), T("nitton")
    };


static uschar_t
    *DA_tens [] = {
        T("ti"), T("tyve"), T("tredive"), T("fyrre"), T("halvtreds"),
        T("treds"), T("halvfjerds"), T("firs"), T("halvfems")
    },
    *DE_tens [] = {
        T("zehn"), T("zwanzig"), T("dreißig"), T("vierzig"), T("fünfzig"),
        T("sechzig"), T("siebzig"), T("achtzig"), T("neunzig")
    },
    *EN_tens [] = {
        T("ten"), T("twenty"), T("thirty"), T("forty"), T("fifty"),
        T("sixty"), T("seventy"), T("eighty"), T("ninety")
    },
    *ES_tens [] = {
        T("diez"), T("veinti"), T("treinta"), T("cuarenta"), T("cincuenta"),
        T("sesenta"), T("setenta"), T("ochenta"), T("noventa")
    },
    *FB_tens [] = {
        T("dix"), T("vingt"), T("trente"), T("quarante"), T("cinquante"),
        T("soixante"), T("septante"), T("quatre-vingt"), T("nonante")
    },
    *FR_tens [] = {
        T("dix"), T("vingt"), T("trente"), T("quarante"), T("cinquante"),
        T("soixante"), T("soixante"), T("quatre-vingt"), T("quatre-vingt")
    },
    *IS_tens [] = {
        T("tiu"), T("tuttugu"), T("thrjatiu"), T("fjörutiu"), T("fumtiu"),
        T("sextiu"), T("sjötiu"), T("attatiu"), T("niutiu")
    },
    *IT_tens [] = {
        T("dieci"), T("venti"), T("trenta"), T("quaranta"), T("cinquanta"),
        T("sessanta"), T("settanta"), T("ottanta"), T("novanta")
    },
    *NL_tens [] = {
        T("tien"), T("twintig"), T("dertig"), T("veertig"), T("vijftig"),
        T("zestig"), T("zeventig"), T("tachtig"), T("negentig")
    },
    *NO_tens [] = {
        T("ti"), T("tyve"), T("tredve"), T("f¢rti"), T("femti"),
        T("seksti"), T("sytti"), T("åtti"), T("nitti")
    },
    *PT_tens [] = {
        T("dez"), T("vinte"), T("trinta"), T("quarenta"), T("cinquenta"),
        T("sessenta"), T("setenta"), T("oitenta"), T("noventa")
    },
    *SV_tens [] = {
        T("tio"), T("tjugo"), T("trettio"), T("fyrtio"), T("femtio"),
        T("sextio"), T("sjuttio"), T("åttio"), T("nittio")
    };


static uschar_t
    *ES_hundreds [] = {
        T("ciento"), T("doscientos"), T("trescientos"), T("cuatrocientos"),
        T("quinientos"), T("seiscientos"), T("setecientos"), T("ochocientos"),
        T("novecientos")
    },
    *PT_hundreds [] = {
        T("cento"), T("duzentos"), T("trezentos"), T("quatrocentos"),
        T("quinhentos"), T("seiscentos"), T("setcentos"), T("oitocentos"),
        T("novecentos")
    };


static uschar_t
    *EN_groups [] = {
        T("million"), T("thousand"), T("")
    },
    *FR_groups [] = {
        T("million"), T("mille"), T("")
    },
    *NL_groups [] = {
        T("Miljoen"), T("duizend"), T("")
    };


/*****************************************************************************/

/*  Prototypes                                                               */

static void     empty_buffer (uschar_t *buffer, int buffer_size, int *buffer_char_nbr);
static void     empty_group (uschar_t *group, int *group_char_nbr);

static void     convert_number_to_full_text (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number, char *language, int *group_start);

#if 0 /* Not used */
static void     append_name_to_group_delim (uschar_t *group, int *group_char_nbr, uschar_t *name);
#endif
static void     append_name_to_group (uschar_t *group, int *group_char_nbr, uschar_t *name);

static void     append_name_to_string_delim (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, uschar_t *name);
static void     append_name_to_string (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, uschar_t *name);

static void     append_group_to_string (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, uschar_t *group, int group_char_nbr);
static void     append_group_to_string_delim (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, uschar_t *group, int group_char_nbr);

#if 0 /* Not used */
static void     convert_cur_char_as_reqd (uschar_t *c1, uschar_t *c2);
#endif

static int      get_group (long number, int group_nbr);


static void en_number_to_text_conversion (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number, int *group_start);
static void en_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);
static void en_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);

static void fr_number_to_text_conversion (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number, int *group_start);
static void fr_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);
static void fr_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);

static void nl_number_to_text_conversion (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number, int *group_start);
static void nl_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);
static void nl_output_group_separator (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr, int buffer_char_nbr);
static void nl_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);

static void de_number_to_text_conversion (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number, int *group_start);
static void de_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);
static void de_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);

static void es_number_to_text_conversion (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number, int *group_start);
static void es_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);
static void es_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);

static void it_number_to_text_conversion (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number, int *group_start);
static void it_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);
static void it_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);

static void pt_number_to_text_conversion (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number, int *group_start);
static void pt_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);
static void pt_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);

static void fb_number_to_text_conversion (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number, int *group_start);
static void fb_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);

static void da_number_to_text_conversion (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number, int *group_start);
static void da_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);
static void da_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);

static void no_number_to_text_conversion (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number, int *group_start);
static void no_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);
static void no_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);

static void sv_number_to_text_conversion (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number, int *group_start);
static void sv_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);
static void sv_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);

static void is_number_to_text_conversion (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number, int *group_start);
static void is_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);
static void is_output_group_separator (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr, int buffer_char_nbr);
static void is_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr);

static void rm_number_to_text_conversion (uschar_t *buffer, int buffer_size, int *buffer_char_nbr, long number);


static int
    user_language = 0;                  /*  0 = default language             */

static Bool
    use_accents = TRUE;

static char
    **day_table    = EN_days,
    **month_table  = EN_months;


/*  Local function prototypes                                                */

static char *handle_accents (char *string);


/*  ---------------------------------------------------------------------[<]-
    Function: set_userlang

    Synopsis: Sets language used for date and numeric translation.
    The valid user languages are:
    <TABLE>
    USERLANG_DEFAULT    Default language (use hard-coded values)
    USERLANG_DA         Danish
    USERLANG_DE         German
    USERLANG_EN         English
    USERLANG_ES         Castillian Spanish
    USERLANG_FB         Belgian or Swiss French
    USERLANG_FR         French
    USERLANG_IS         Icelandic
    USERLANG_IT         Italian
    USERLANG_NL         Dutch
    USERLANG_NO         Norwegian
    USERLANG_PO         Portuguese
    USERLANG_SV         Swedish
    </TABLE>
    Returns 0 if okay, -1 if an unsupported language was specified.
    ---------------------------------------------------------------------[>]-*/

int
set_userlang (int language)
{
    /*  Order of this table is not critical                                  */
    static struct {
        int  language;
        char **days;
        char **months;
    } languages [] =
    {
        { USERLANG_DEFAULT, EN_days, EN_months },
        { USERLANG_DA,      EN_days, DA_months },
        { USERLANG_DE,      EN_days, DE_months },
        { USERLANG_EN,      EN_days, EN_months },
        { USERLANG_ES,      EN_days, ES_months },
        { USERLANG_FB,      FR_days, FR_months },
        { USERLANG_FR,      FR_days, FR_months },
        { USERLANG_IS,      EN_days, IS_months },
        { USERLANG_IT,      EN_days, IT_months },
        { USERLANG_NL,      NL_days, NL_months },
        { USERLANG_NO,      EN_days, NO_months },
        { USERLANG_PO,      EN_days, PO_months },
        { USERLANG_SV,      EN_days, SV_months },
        { -1,               NULL,    NULL      }
    };

    int
        index;

    for (index = 0; languages [index].language != -1; index++)
        if (languages [index].language == language)
          {
            user_language = language;
            day_table     = languages [index].days;
            month_table   = languages [index].months;
            return (0);
          }
    return (-1);
}


/*  ---------------------------------------------------------------------[<]-
    Function: set_userlang_str

    Synopsis: Sets language used for date and numeric translation, using a
    string representation of the language. The valid user languages are:
    <TABLE>
    ""      Default language (use hard-coded values)
    "--"    Alternative form for default language
    "DA"    Danish
    "DE"    German
    "EN"    English
    "ES"    Castillian Spanish
    "FB"    Belgian or Swiss French
    "FR"    French
    "IS"    Icelandic
    "IT"    Italian
    "NL"    Dutch
    "NO"    Norwegian
    "PO"    Portuguese
    "SV"    Swedish
    </TABLE>
    Returns 0 if okay, -1 if an unsupported language was specified.
    ---------------------------------------------------------------------[>]-*/

int
set_userlang_str (const char *language)
{
    int
        index;

    if (strnull (language))
        return (set_userlang (USERLANG_DEFAULT));

    for (index = 0; index < USERLANG_TOP; index++)
        if (streq (language, language_str [index]))
            return (set_userlang (index));

    return (-1);
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_userlang

    Synopsis: Returns the current user language code.
    ---------------------------------------------------------------------[>]-*/

int
get_userlang (void)
{
    return (user_language);
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_userlang_str

    Synopsis: Returns the current user language as a 2-character string.
    ---------------------------------------------------------------------[>]-*/

char *
get_userlang_str (void)
{
    return (language_str [user_language]);
}


/*  ---------------------------------------------------------------------[<]-
    Function: set_accents

    Synopsis: Enables or disables native-language accents.  If enabled,
    accented characters in translated words are produced in the current
    system character set, if possible.  Otherwise, suitable translations
    are made into the 26-letter English alphabet.  By default, accents are
    enabled.
    ---------------------------------------------------------------------[>]-*/

int
set_accents (Bool accents)
{
    use_accents = accents;
    return (0);
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_accents

    Synopsis: Returns TRUE if accents are enabled, FALSE if not.
    ---------------------------------------------------------------------[>]-*/

Bool
get_accents (void)
{
    return (use_accents);
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_day_name

    Synopsis: Returns the name for the specified day, which must be a
    value from 0 (Sunday) to 6 (Saturday).  Accented characters are
    formatted according to the current accents setting.
    ---------------------------------------------------------------------[>]-*/

char *
get_day_name (int day)
{
    ASSERT (day >= 0 && day <= 6);
    return (handle_accents (day_table [day]));
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_day_abbrev

    Synopsis: Returns the abbreviation for the specified day, which must be
    a value from 0 (Sunday) to 6 (Saturday).  The abbreviation (3 letters)
    is converted into uppercase if the 'upper' argument is true.  Accented
    characters are formatted according to the current accents setting.
    ---------------------------------------------------------------------[>]-*/

char *
get_day_abbrev (int day, Bool upper)
{
    char
        abbrev [4];

    ASSERT (day >= 0 && day <= 6);

    strncpy (abbrev, day_table [day], 3);
    abbrev [3] = '\0';
    if (upper)
        strupc (abbrev);
    return (handle_accents (abbrev));
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_month_name

    Synopsis: Returns the name for the specified month, which must be a
    value from 1 to 12.  Accented characters are handled as per the current
    accents setting.
    ---------------------------------------------------------------------[>]-*/

char *
get_month_name (int month)
{
    ASSERT (month >= 1 && month <= 12);
    return (handle_accents (month_table [month - 1]));
}


/*  ---------------------------------------------------------------------[<]-
    Function: get_month_abbrev

    Synopsis: Returns the abbreviation for the specified month, which must
    be a value from 1 to 12.  The abbreviation (3 letters) is converted into
    uppercase if the 'upper' argument is true.  Accented characters are
    formatted according to the current accents setting.
    ---------------------------------------------------------------------[>]-*/

char *
get_month_abbrev (int month, Bool upper)
{
    char
        abbrev [4];

    ASSERT (month >= 1 && month <= 12);

    strncpy (abbrev, month_table [month - 1], 3);
    abbrev [3] = '\0';
    if (upper)
        strupc (abbrev);

    return (handle_accents (abbrev));
}


/*  handle_accents -- internal
 *
 *  Accepts string containing escaped accented characters and converts into
 *  local values.  Currently handles DOS and Unix character sets and this
 *  (reduced) set of accented characters:
 *
 *      /Uu     u umlaut
 *      /UU     U umlaut
 *      /Uo     o umlaut
 *      /UO     O umlaut
 *      /Ra     a ring
 *      /RA     A ring
 *      /So     slash o
 *      /SO     slash O
 *      /Bs     scharfes S
 *      /BS     scharfes S
 *      /Cu     circumflex u
 *      /CU     circumflex U
 */

static char *
handle_accents (char *string)
{
#if (defined (__UNIX__))
#   define CHAR_a_ring    '\345'
#   define CHAR_A_ring    '\305'
#   define CHAR_o_uml     '\366'
#   define CHAR_O_uml     '\326'
#   define CHAR_u_circ    '\373'
#   define CHAR_u_uml     '\374'
#   define CHAR_U_uml     '\334'
#elif (defined (__MSDOS__))
#   define CHAR_a_ring    '\206'
#   define CHAR_A_ring    '\217'
#   define CHAR_o_uml     '\224'
#   define CHAR_O_uml     '\231'
#   define CHAR_u_circ    '\226'
#   define CHAR_u_uml     '\201'
#   define CHAR_U_uml     '\232'
#else
#   define CHAR_a_ring    'a'
#   define CHAR_A_ring    'A'
#   define CHAR_o_uml     'o'
#   define CHAR_O_uml     'O'
#   define CHAR_u_circ    'u'
#   define CHAR_u_uml     'u'
#   define CHAR_U_uml     'U'
#endif
    return (string);                    /*  To be implemented                */
}


/*  ---------------------------------------------------------------------[<]-
    Function: timestamp_string

    Synopsis: Formats a timestamp according to a user-supplied pattern.  The
    result is returned in a buffer supplied by the caller; if this argument
    is NULL, allocates a buffer and returns that (the caller must then free
    the buffer using mem_free()).  The pattern consists of arbitrary text
    mixed with insertion symbols indicated by '%':
    <TABLE>
        %y        day of year, 001-366
        %yy       year 2 digits, 00-99
        %yyyy     year 4 digits, 0100-9999
        %mm       month, 01-12
        %mmm      month, Jan
        %mmmm     month, January
        %MMM      month, JAN
        %MMMM     month, JANUARY
        %dd       day, 01-31
        %ddd      day of week, Sun
        %dddd     day of week, Sunday
        %DDD      day of week, SUN
        %DDDD     day of week, SUNDAY
        %w        day of week, 1-7 (1=Sunday)
        %ww       week of year, 01-53
        %q        year quarter, 1-4
        %h        hour, 00-23
        %m        minute, 00-59
        %s        second, 00-59
        %c        centisecond, 00-99
        %%        literal character %
    </TABLE>
    ---------------------------------------------------------------------[>]-*/

char *
timestamp_string (char *buffer, const char *pattern)
{
    long
        date,                           /*  Current date                     */
        time;                           /*    and time                       */
    int
        century,                        /*  Century component of date        */
        year,                           /*  Year component of date           */
        month,                          /*  Month component of date          */
        day,                            /*  Day component of date            */
        hour,                           /*  Hour component of time           */
        minute,                         /*  Minute component of time         */
        second,                         /*  Second component of time         */
        centi,                          /*  1/100 sec component of time      */
        cursize;                        /*  Size of current component        */
    char
       *dest,                           /*  Store formatted data here        */
        ch;                             /*  Next character in picture        */

    get_date_time_now (&date, &time);

    century = GET_CENTURY (date);
    year    = GET_YEAR    (date);
    month   = GET_MONTH   (date);
    day     = GET_DAY     (date);
    hour    = GET_HOUR    (time);
    minute  = GET_MINUTE  (time);
    second  = GET_SECOND  (time);
    centi   = GET_CENTI   (time);

    if (buffer == NULL)
        buffer = mem_alloc (strlen (pattern) * 2);

    /*  Scan through picture, converting each component                      */
    dest = buffer;
    *dest = 0;                          /*  String is empty                  */
    while (*pattern)
      {
        ch = *pattern++;
        if (ch == '%' && *pattern)
          {
            ch = *pattern++;            /*  Count size of pattern after %    */
            for (cursize = 1; *pattern == ch; cursize++)
                pattern++;
          }
        else
          {
            *dest++ = ch;               /*  Something else - store and next  */
            *dest = 0;                  /*  Terminate the string nicely      */
            continue;
          }

        /*  Now process pattern itself                                       */
        switch (ch)
          {
            case 'y':
                if (cursize == 1)       /*  y     day of year, 001-366       */
                    sprintf (dest, "%03d", julian_date (date));
                else
                if (cursize == 2)       /*  yy    year 2 digits, 00-99       */
                    sprintf (dest, "%02d", year);
                else
                if (cursize == 4)       /*  yyyy  year 4 digits, 0100-9999   */
                    sprintf (dest, "%02d%02d", century, year);
                break;

            case 'm':
                if (cursize == 1)       /*  m     minute, 00-59              */
                    sprintf (dest, "%02d", minute);
                else
                if (cursize == 2)       /*  mm    month, 01-12               */
                    sprintf (dest, "%02d", month);
                else
                if (cursize == 3)       /*  mmm   month, 3 letters           */
                    strcpy (dest, get_month_abbrev (month, FALSE));
                else
                if (cursize == 4)       /*  mmmm  month, full name           */
                    strcpy (dest, get_month_name (month));
                break;

            case 'M':
                if (cursize == 3)       /*  MMM   month, 3-letters, ucase    */
                    strcpy (dest, get_month_abbrev (month, TRUE));
                else
                if (cursize == 4)       /*  MMMM  month, full name, ucase    */
                  {
                    strcpy (dest, get_month_name (month));
                    strupc (dest);
                  }
                break;

            case 'd':
                if (cursize == 2)       /*  dd    day, 01-31                 */
                    sprintf (dest, "%02d", day);
                else
                if (cursize == 3)       /*  ddd   day of week, Sun           */
                    strcpy (dest, get_day_abbrev (day_of_week (date), FALSE));
                else
                if (cursize == 4)       /*  dddd  day of week, Sunday        */
                    strcpy (dest, get_day_name (day_of_week (date)));
                break;

            case 'D':
                if (cursize == 3)       /*  DDD   day of week, SUN           */
                    strcpy (dest, get_day_abbrev (day_of_week (date), TRUE));
                else
                if (cursize == 4)       /*  DDDD  day of week, SUNDAY        */
                  {
                    strcpy (dest, get_day_name (day_of_week (date)));
                    strupc (dest);
                  }
                break;

            case 'w':
                if (cursize == 1)       /*  w     day of week, 1-7 (1=Sun)   */
                    sprintf (dest, "%d", day_of_week (date) + 1);
                else
                if (cursize == 2)       /*  ww    week of year, 01-53        */
                    sprintf (dest, "%d", week_of_year (date));
                break;

            case 'q':
                if (cursize == 1)       /*  q     year quarter, 1-4          */
                    sprintf (dest, "%d", year_quarter (date));
                break;

            case 'h':
                if (cursize == 1)       /*  h     hour, 00-23                */
                    sprintf (dest, "%02d", hour);
                break;

            case 's':
                if (cursize == 1)       /*  s     second, 00-59              */
                    sprintf (dest, "%02d", second);
                break;

            case 'c':
                if (cursize == 1)       /*  c     centisecond, 00-99         */
                    sprintf (dest, "%02d", centi);
                break;

            case '%':
                if (cursize == 1)       /*  %     literal '%'                */
                    strcpy (dest, "%");
                break;
        }
        if (*dest)                      /*  If something was output,         */
            while (*dest)               /*    skip to end of string          */
                dest++;
        else
          {
            while (cursize--)           /*  Else output ch once or more      */
                *dest++ = ch;           /*    and bump dest pointer          */
            *dest = 0;                  /*  Terminate the string nicely      */
          }
    }
    return (buffer);
}


/*****************************************************************************/

/*  ---------------------------------------------------------------------[<]-
    Function: certify_the_number

    Synopsis:
    ---------------------------------------------------------------------[>]-*/

char *
certify_the_number (char *buf, int buffer_size, long number,
                    char *language, int code_page)
{
    int
        group_nbr,
        output_group,
        digit_nbr,
        char_nbr,
        group_start [4],
        buffer_char_nbr;
#if (defined (__USE_UNICODE__))
    char
        *ascii_buffer;
#endif
    uschar_t
        *buffer,
        display_char [10];

    ASSERT (buf);

    buffer = mem_alloc (buffer_size * uschar_size);

    if (number < 0 || number > 999999999)
      {
        fprintf (stderr, "Overflow error.\nYou must supply a positive");
        fprintf (stderr, " integer less or equal to 999999999.\n");
        return (NULL);
      }

    empty_buffer (buffer, buffer_size, &buffer_char_nbr);

    convert_number_to_full_text (buffer, buffer_size, &buffer_char_nbr, number,
                                 language, group_start);

    if (lexcmp (language, "RM") != 0)
      {
        for (group_nbr = 3; group_nbr >= 1 && buffer_char_nbr > buffer_size - 1; group_nbr--)
          {
            buffer_char_nbr = group_start [group_nbr];

            for (output_group = group_nbr; output_group <= 3; output_group++)
              {
#if (defined (__USE_UNICODE__))
                swprintf (display_char, T("%9d"), get_group (number, output_group));
#else
                sprintf (display_char, T("%9d"), get_group (number, output_group));
#endif
    
                for (digit_nbr = 0; digit_nbr < 9; digit_nbr++)
                  {
                    if (display_char [digit_nbr] != (uschar_t)' ' && buffer_char_nbr < buffer_size - 1)
                        buffer [buffer_char_nbr++] = display_char [digit_nbr];
                  }

                char_nbr = buffer_char_nbr;
                while (char_nbr < buffer_size - 1)
                    buffer [char_nbr++] = (uschar_t)' ';

                buffer_char_nbr++;
              }
          }

#if (defined (__USE_UNICODE__))
        buffer [0] = towupper (buffer [0]);
#else
        buffer [0] = toupper (buffer [0]);
#endif
      }

#if (defined (__USE_UNICODE__))
    ascii_buffer = ucode2ascii_ex (buffer, code_page);

    mem_free (buffer);

    strcrop (ascii_buffer);
    strcpy (buf, ascii_buffer);

    mem_free (ascii_buffer);
#else
    strcrop (buffer);
    strcpy (buf, buffer);
    mem_free (buffer);
#endif

    return (buf);
}


static void
empty_buffer (uschar_t *buffer, int buffer_size, int *buffer_char_nbr)
{
    int
        i;

    for (i = 0; i < buffer_size - 1; i++)
        buffer [i] = (uschar_t)' ';

    buffer [i] = (uschar_t)'\0';
    *buffer_char_nbr = 0;
}


static void
convert_number_to_full_text (uschar_t *buffer, int buffer_size,
                             int *buffer_char_nbr, long number, char *language,
                             int *group_start)
{
    if (lexcmp (language, "FR") == 0)
        fr_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number, group_start);
    else
    if (lexcmp (language, "EN-GB") == 0)
        en_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number, group_start);
    else
    if (lexcmp (language, "NL") == 0)
        nl_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number, group_start);
    else
    if (lexcmp (language, "DE") == 0)
        de_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number, group_start);
    else
    if (lexcmp (language, "ES") == 0)
        es_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number, group_start);
    else
    if (lexcmp (language, "IT") == 0)
        it_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number, group_start);
    else
    if (lexcmp (language, "PT") == 0)
        pt_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number, group_start);
    else
    if (lexcmp (language, "FR-BE") == 0)
        fb_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number, group_start);
    else
    if (lexcmp (language, "DA") == 0)
        da_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number, group_start);
    else
    if (lexcmp (language, "NO") == 0)
        no_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number, group_start);
    else
    if (lexcmp (language, "SV") == 0)
        sv_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number, group_start);
    else
    if (lexcmp (language, "IS") == 0)
        is_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number, group_start);
    else
    if (lexcmp (language, "RM") == 0)
        rm_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number);
    else
        en_number_to_text_conversion (buffer, buffer_size, buffer_char_nbr, number, group_start);
}


static void
append_name_to_string_delim (uschar_t *buffer, int buffer_size,
                             int *buffer_char_nbr, uschar_t *name)
{
    append_name_to_string (buffer, buffer_size, buffer_char_nbr, name);
    (*buffer_char_nbr)++;
}


static void
append_name_to_string (uschar_t *buffer, int buffer_size, int *buffer_char_nbr,
                       uschar_t *name)
{
    int
        i;
    uschar_t
#if (!defined (__USE_NATIONAL_CHARS__))
        c2,
#endif
        c1;

    for (i = 0; name [i] != (uschar_t)' ' && name [i] != (uschar_t)'\0'; i++)
      {
        c1 = name [i];
#if (defined (__USE_NATIONAL_CHARS__))
        if (*buffer_char_nbr < buffer_size - 1)
            buffer [(*buffer_char_nbr)++] = c1;
#else
        convert_cur_char_as_reqd (&c1, &c2);
        if (*buffer_char_nbr < buffer_size - 1)
            buffer [(*buffer_char_nbr)++] = c1;
        if (c2 != (uschar_t)' ')
            if (*buffer_char_nbr < buffer_size - 1)
                buffer [(*buffer_char_nbr)++] = c2;
#endif
      }
}


/*****************************************************************************/

static void
en_number_to_text_conversion (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, long number,
                              int *group_start)
{
    int
        i,
        group_value,
        group_char_nbr;
    uschar_t
        group [GROUP_SIZE];

    for (i = 1; i <= 3; i++)
      {
        empty_group (group, &group_char_nbr);
        group_value = get_group (number, i);
        group_start [i] = *buffer_char_nbr;
        if (group_value > 0)
          {
            en_output_group_hundreds  (group, &group_char_nbr, group_value, i);
            en_output_group_tens_ones (group, &group_char_nbr, group_value, i);

            if (i == 3 && group_value < 100 && *buffer_char_nbr > 0)
                append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("and"));

            append_group_to_string_delim (buffer, buffer_size, buffer_char_nbr, group, group_char_nbr);
            append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, EN_groups [i - 1]);
          }
      }
}


static void
empty_group (uschar_t *group, int *group_char_nbr)
{
    int
        i;

    for (i = 0; i < GROUP_SIZE - 1; i++)
        group [i] = (uschar_t)' ';

    group [i] = (uschar_t)'\0';
    *group_char_nbr = 0;
}


static void
append_group_to_string_delim (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, uschar_t *group,
                              int group_char_nbr)
{
    append_group_to_string (buffer, buffer_size, buffer_char_nbr, group,
                            group_char_nbr);
    (*buffer_char_nbr)++;
}


static void
append_group_to_string (uschar_t *buffer, int buffer_size,
                        int *buffer_char_nbr, uschar_t *group,
                        int group_char_nbr)
{
    int
        i;

    for (i = 0; i < group_char_nbr; i++)
      {
        if (*buffer_char_nbr < buffer_size - 1)
            buffer [(*buffer_char_nbr)++] = group [i];
      }
}


static void
en_output_group_hundreds (uschar_t *group, int *group_char_nbr,
                          int group_value, int group_nbr)
{
    int
        hundreds,
        tens_ones;

    hundreds  = GET_GROUP_HUNDREDS  (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (hundreds > 0)
      {
        append_name_to_group (group, group_char_nbr, EN_units [hundreds]);
        (*group_char_nbr)++;
        append_name_to_group (group, group_char_nbr, T("hundred"));
        if (tens_ones > 0)
          {
            (*group_char_nbr)++;
            append_name_to_group (group, group_char_nbr, T("and"));
            (*group_char_nbr)++;
          }
      }
}


#if 0
/* Not used according to compiler. Please fix. */
static void
append_name_to_group_delim (uschar_t *group, int *group_char_nbr,
                            uschar_t *name)
{
    append_name_to_group (group, group_char_nbr, name);
    (*group_char_nbr)++;
}
#endif


static void
append_name_to_group (uschar_t *group, int *group_char_nbr, uschar_t *name)
{
    int
        i;
    uschar_t
#if (!defined (__USE_NATIONAL_CHARS__))
        c2,
#endif
        c1;

    for (i = 0; name [i] != (uschar_t)' ' && name [i] != (uschar_t)'\0'; i++)
      {
        c1 = name [i];
#if (defined (__USE_NATIONAL_CHARS__))
        if (*group_char_nbr < GROUP_SIZE - 1)
            group [(*group_char_nbr)++] = c1;
#else
        convert_cur_char_as_reqd (&c1, &c2);
        if (*group_char_nbr < GROUP_SIZE - 1)
            group [(*group_char_nbr)++] = c1;
        if (c2 != (uschar_t)' ')
            if (*group_char_nbr < GROUP_SIZE - 1)
                group [(*group_char_nbr)++] = c2;
#endif
      }
}


#if 0
/* Not used according to compiler. Please fix. */
static void
convert_cur_char_as_reqd (uschar_t *c1, uschar_t *c2)
{
    *c2 = (uschar_t)' ';
    if (*c1 == (uschar_t)'\345')        /* 'å'                               */
      {
        *c1 = (uschar_t)'a';
        *c2 = (uschar_t)'a';
      }
    else
    if (*c1 == (uschar_t)'\308'         /* 'ö'                               */
     || *c1 == (uschar_t)'\242')        /* '¢'                               */
      {
        *c1 = (uschar_t)'o';
        *c2 = (uschar_t)'e';
      }
    else
    if (*c1 == (uschar_t)'\374')        /* 'ü'                               */
      {
        *c1 = (uschar_t)'u';
        *c2 = (uschar_t)'e';
      }
    else
    if (*c1 == (uschar_t)'\255')        /* '¡'                               */
      {
        *c1 = (uschar_t)'u';
      }
    else
    if (*c1 == (uschar_t)'\207')        /* 'ç'                               */
      {
        *c1 = (uschar_t)'c';
      }
    else
    if (*c1 == (uschar_t)'\221'         /* 'æ'                               */
     || *c1 == (uschar_t)'\204')        /* 'ä'                               */
      {
        *c1 = (uschar_t)'a';
        *c2 = (uschar_t)'e';
      }
    else
    if (*c1 == (uschar_t)'\337')        /* 'ß'                               */
      {
        *c1 = (uschar_t)'s';
        *c2 = (uschar_t)'s';
      }
}
#endif


static void
en_output_group_tens_ones (uschar_t *group, int *group_char_nbr,
                           int group_value, int group_nbr)
{
    int
        tens_ones,
        tens,
        ones;

    ones      = GET_GROUP_ONES      (group_value);
    tens      = GET_GROUP_TENS      (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (tens_ones > 0)
      {
        if (tens_ones < 20)
          {
            append_name_to_group (group, group_char_nbr, EN_units [tens_ones]);
            (*group_char_nbr)++;
          }
        else
          {
            append_name_to_group (group, group_char_nbr, EN_tens [tens - 1]);
            if (ones > 0)
              {
                if (*group_char_nbr < GROUP_SIZE - 1)
                    group [(*group_char_nbr)++] = (uschar_t)'-';

                append_name_to_group (group, group_char_nbr, EN_units [ones]);
                (*group_char_nbr)++;
              }
          }
      }
}


/*****************************************************************************/

static void
fr_number_to_text_conversion (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, long number,
                              int *group_start)
{
    int
        i,
        group_value,
        group_char_nbr;
    uschar_t
        group [GROUP_SIZE];

    for (i = 1; i <= 3; i++)
      {
        empty_group (group, &group_char_nbr);
        group_value = get_group (number, i);
        group_start [i] = *buffer_char_nbr;
        if (group_value > 0)
          {
            fr_output_group_hundreds (group, &group_char_nbr, group_value, i);
            fr_output_group_tens_ones (group, &group_char_nbr, group_value, i);

            if (i == 2 && group_value == 1)
                empty_group (group, &group_char_nbr);

            append_group_to_string (buffer, buffer_size, buffer_char_nbr, group, group_char_nbr);
            append_name_to_string (buffer, buffer_size, buffer_char_nbr, FR_groups [i - 1]);

            if (i == 1 && group_value > 1)
                append_name_to_string (buffer, buffer_size, buffer_char_nbr, T("s"));

            (*buffer_char_nbr)++;
          }
      }
}


static void
fr_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        hundreds,
        tens_ones;

    hundreds  = GET_GROUP_HUNDREDS  (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (hundreds > 0)
      {
        if (hundreds == 1)
          {
            append_name_to_group (group, group_char_nbr, T("cent"));
             (*group_char_nbr)++;
          }
        else
          {
            append_name_to_group (group, group_char_nbr, FR_units [hundreds]);
            (*group_char_nbr)++;
            if (tens_ones > 0 || group_nbr < 3)
              {
                append_name_to_group (group, group_char_nbr, T("cent"));
                (*group_char_nbr)++;
              }
            else
              {
                append_name_to_group (group, group_char_nbr, T("cents"));
                (*group_char_nbr)++;
              }
          }
      }
}


static void
fr_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        ones,
        tens,
        tens_ones,
        score;

    ones      = GET_GROUP_ONES      (group_value);
    tens      = GET_GROUP_TENS      (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (tens_ones > 0)
      {
        if (tens_ones < 20)
          {
            append_name_to_group (group, group_char_nbr, FR_units [tens_ones]);
            (*group_char_nbr)++;
          }
        else
          {
            append_name_to_group (group, group_char_nbr, FR_tens [tens - 1]);
            if (tens_ones == 21
             || tens_ones == 31
             || tens_ones == 41
             || tens_ones == 51
             || tens_ones == 61
             || tens_ones == 71)
              {
                (*group_char_nbr)++;
                append_name_to_group (group, group_char_nbr, T("et"));
                (*group_char_nbr)++;
              }
            else
            if (ones > 0
             || tens_ones == 70
             || tens_ones == 90)
              {
                if (*group_char_nbr < GROUP_SIZE - 1)
                    group[(*group_char_nbr)++] = (uschar_t)'-';
              }
            else
            if (tens_ones != 80)
                (*group_char_nbr)++;
            if (tens_ones < 70)
              {
                if (ones > 0)
                  {
                    append_name_to_group (group, group_char_nbr, FR_units [ones]);
                    (*group_char_nbr)++;
                  }
              }
            else
            if (tens_ones > 69)
              {
                score = tens_ones - 60;
                if (score > 19)
                    score -= 20;

                if (score > 0)
                  {
                    append_name_to_group (group, group_char_nbr, FR_units [score]);
                    (*group_char_nbr)++;
                  }
                else
                if (tens_ones == 80)
                  {
                    append_name_to_group (group, group_char_nbr, T("s"));
                    (*group_char_nbr)++;
                  }
              }
          }
      }
}


/*****************************************************************************/

static void
nl_number_to_text_conversion (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, long number,
                              int *group_start)
{
    int
        i,
        group_value,
        group_char_nbr;
    uschar_t
        group [GROUP_SIZE];

    for (i = 1; i <= 3; i++)
      {
        empty_group (group, &group_char_nbr);
        group_value = get_group (number, i);
        group_start [i] = *buffer_char_nbr;
        if (group_value > 0)
          {
            nl_output_group_hundreds (group, &group_char_nbr, group_value, i);
            nl_output_group_separator (group, &group_char_nbr, group_value, i, *buffer_char_nbr);
            nl_output_group_tens_ones (group, &group_char_nbr, group_value, i);

            if (i == 1)
                group_char_nbr++;
            if (i == 2 && group_value == 1)
                empty_group (group, &group_char_nbr);

            append_group_to_string (buffer, buffer_size, buffer_char_nbr, group, group_char_nbr);
            append_name_to_string (buffer, buffer_size, buffer_char_nbr, NL_groups [i - 1]);

            if (i == 1 || i == 2)
                (*buffer_char_nbr)++;

          }
      }
}


static void
nl_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        hundreds;

    hundreds = GET_GROUP_HUNDREDS (group_value);

    if (hundreds > 0)
      {
        if (hundreds > 1)
            append_name_to_group (group, group_char_nbr, NL_units [hundreds]);

        append_name_to_group (group, group_char_nbr, T("honderd"));
      }
}


static void
nl_output_group_separator (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr, int buffer_char_nbr)
{
    int
        hundreds,
        tens_ones;

    tens_ones = GET_GROUP_TENS_ONES (group_value);
    hundreds  = GET_GROUP_HUNDREDS  (group_value);

    if (tens_ones > 0 && tens_ones < 13)
      {
        if (hundreds > 0)
          {
            (*group_char_nbr)++;
            append_name_to_group (group, group_char_nbr, T("en"));
            (*group_char_nbr)++;
          }
        else
        if (group_nbr == 3 && buffer_char_nbr > 0)
          {
            append_name_to_group (group, group_char_nbr, T("en"));
            (*group_char_nbr)++;
          }
      }
}


static void
nl_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        ones,
        tens,
        tens_ones;

    ones      = GET_GROUP_ONES      (group_value);
    tens      = GET_GROUP_TENS      (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (tens_ones > 0)
      {
        if (tens_ones < 20)
            append_name_to_group (group, group_char_nbr, NL_units [tens_ones]);
        else
          {
            if (ones > 0)
              {
                append_name_to_group (group, group_char_nbr, NL_units [ones]);
                append_name_to_group (group, group_char_nbr, T("en"));
              }
            append_name_to_group (group, group_char_nbr, NL_tens [tens - 1]);
          }
      }
}


/*****************************************************************************/

static void
de_number_to_text_conversion (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, long number,
                              int *group_start)
{
    int
        i,
        group_value,
        group_char_nbr;
    uschar_t
        group [GROUP_SIZE];

    for (i = 1; i <= 3; i++)
      {
        empty_group (group, &group_char_nbr);
        group_value = get_group (number, i);
        group_start [i] = *buffer_char_nbr;
        if (group_value > 0)
          {
            de_output_group_hundreds (group, &group_char_nbr, group_value, i);
            de_output_group_tens_ones (group, &group_char_nbr, group_value, i);

            if (i == 1)
              {
                if (group_value == 1)
                  {
                    append_name_to_group (group, &group_char_nbr, T("eine"));
                    group_char_nbr++;
                  }
                else
                    group_char_nbr++;
              }
            if (i == 2 && group_value == 1)
                empty_group (group, &group_char_nbr);

            append_group_to_string (buffer, buffer_size, buffer_char_nbr, group, group_char_nbr);

            if (i == 1)
                if (group_value == 1)
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("Million"));
                else
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("Millionen"));
            else
            if (i == 2)
                append_name_to_string (buffer, buffer_size, buffer_char_nbr, T("taussend"));

          }
      }
}


static void
de_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        hundreds;

    hundreds = GET_GROUP_HUNDREDS (group_value);

    if (hundreds > 0)
      {
        if (hundreds > 1)
            append_name_to_group (group, group_char_nbr, DE_units [hundreds]);

        append_name_to_group (group, group_char_nbr, T("hundert"));
      }
}


static void
de_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        ones,
        tens,
        tens_ones;

    ones      = GET_GROUP_ONES      (group_value);
    tens      = GET_GROUP_TENS      (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (tens_ones > 0)
      {
        if (tens_ones == 1)
          {
            if (group_nbr == 3)
                append_name_to_group (group, group_char_nbr, T("eins"));
            else
                append_name_to_group (group, group_char_nbr, T("ein"));
          }
        else
        if (tens_ones < 20)
            append_name_to_group (group, group_char_nbr, DE_units [tens_ones]);
        else
          {
            if (ones > 0)
              {
                append_name_to_group (group, group_char_nbr, DE_units [ones]);
                append_name_to_group (group, group_char_nbr, T("und"));
              }
            append_name_to_group (group, group_char_nbr, DE_tens [tens - 1]);
          }
      }
}


/*****************************************************************************/

static void
es_number_to_text_conversion (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, long number,
                              int *group_start)
{
    int
        i,
        group_value,
        group_char_nbr;
    uschar_t
        group [GROUP_SIZE];

    for (i = 1; i <= 3; i++)
      {
        empty_group (group, &group_char_nbr);
        group_value = get_group (number, i);
        group_start [i] = *buffer_char_nbr;
        if (group_value > 0)
          {
            es_output_group_hundreds (group, &group_char_nbr, group_value, i);
            es_output_group_tens_ones (group, &group_char_nbr, group_value, i);

            if (i == 1 && group_value == 1)
              {
                append_name_to_group (group, &group_char_nbr, T("un"));
                group_char_nbr++;
              }
            else
            if (i == 2 && group_value == 1)
                empty_group (group, &group_char_nbr);

            append_group_to_string (buffer, buffer_size, buffer_char_nbr, group, group_char_nbr);

            if (i == 1)
                if (group_value == 1)
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("millon"));
                else
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("millones"));
            else
            if (i == 2)
                append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("mil"));

          }
      }
}


static void
es_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        hundreds;

    hundreds = GET_GROUP_HUNDREDS (group_value);

    if (hundreds > 0)
      {
        if (group_value == 100)
          {
            append_name_to_group (group, group_char_nbr, T("cien"));
            (*group_char_nbr)++;
          }
        else
          {
            append_name_to_group (group, group_char_nbr, ES_hundreds [hundreds]);
            (*group_char_nbr)++;
          }
      }
}


static void
es_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        ones,
        tens,
        tens_ones;

    ones      = GET_GROUP_ONES      (group_value);
    tens      = GET_GROUP_TENS      (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (tens_ones > 0)
      {
        if (tens_ones < 21)
          {
            append_name_to_group (group, group_char_nbr, ES_units [tens_ones]);
            (*group_char_nbr)++;
          }
        else
          {
            append_name_to_group (group, group_char_nbr, ES_tens [tens - 1]);
            if (ones > 0)
              {
                if (tens_ones > 30)
                  {
                    (*group_char_nbr)++;
                    append_name_to_group (group, group_char_nbr, T("y"));
                    (*group_char_nbr)++;
                  }

                append_name_to_group (group, group_char_nbr, ES_units [ones]);
                (*group_char_nbr)++;
              }
            else
                (*group_char_nbr)++;
          }
      }
}


/*****************************************************************************/

static void
it_number_to_text_conversion (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, long number,
                              int *group_start)
{
    int
        i,
        group_value,
        group_char_nbr;
    uschar_t
        group [GROUP_SIZE];

    for (i = 1; i <= 3; i++)
      {
        empty_group (group, &group_char_nbr);
        group_value = get_group (number, i);
        group_start [i] = *buffer_char_nbr;
        if (group_value > 0)
          {
            it_output_group_hundreds (group, &group_char_nbr, group_value, i);
            it_output_group_tens_ones (group, &group_char_nbr, group_value, i);

            if (i == 1 && group_value == 1)
              {
                append_name_to_group (group, &group_char_nbr, T("un"));
                group_char_nbr++;
              }
            else
            if (i == 2 && group_value == 1)
                empty_group (group, &group_char_nbr);

            append_group_to_string (buffer, buffer_size, buffer_char_nbr, group, group_char_nbr);

            if (i == 1)
			  {
                if (group_value == 1)
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("milione"));
                else
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("milioni"));
			  }
            else
            if (i == 2)
			  {
                if (group_value == 1)
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("mille"));
                else
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("mila"));
			  }
		  }
      }
}


static void
it_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        hundreds;

    hundreds = GET_GROUP_HUNDREDS (group_value);

    if (hundreds > 0)
      {
        if (hundreds > 1)
            append_name_to_group (group, group_char_nbr, IT_units [hundreds]);

        append_name_to_group (group, group_char_nbr, T("cento"));
        (*group_char_nbr)++;
      }
}


static void
it_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        ones,
        tens,
        tens_ones;

    ones      = GET_GROUP_ONES      (group_value);
    tens      = GET_GROUP_TENS      (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (tens_ones > 0)
      {
        if (tens_ones < 20)
          {
            append_name_to_group (group, group_char_nbr, IT_units [tens_ones]);
            (*group_char_nbr)++;
          }
        else
          {
            append_name_to_group (group, group_char_nbr, IT_tens [tens - 1]);
            if (ones > 0)
              {
                if (ones == 1 || ones == 8)
                    (*group_char_nbr)--;

                append_name_to_group (group, group_char_nbr, IT_units [ones]);
                (*group_char_nbr)++;
              }
            else
                (*group_char_nbr)++;
          }
      }
}


/*****************************************************************************/

static void
pt_number_to_text_conversion (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, long number,
                              int *group_start)
{
    int
        i,
        group_value,
        group_char_nbr;
    uschar_t
        group [GROUP_SIZE];

    for (i = 1; i <= 3; i++)
      {
        empty_group (group, &group_char_nbr);
        group_value = get_group (number, i);
        group_start [i] = *buffer_char_nbr;
        if (group_value > 0)
          {
            pt_output_group_hundreds (group, &group_char_nbr, group_value, i);
            pt_output_group_tens_ones (group, &group_char_nbr, group_value, i);

            if (i == 2 && group_value == 1)
                empty_group (group, &group_char_nbr);
            else
                group_char_nbr++;

            append_group_to_string (buffer, buffer_size, buffer_char_nbr, group, group_char_nbr);

            if (i == 1)
              {
                if (group_value == 1)
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("milhao"));
                else
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("milhoes"));
                if (get_group (number, 2) > 0 || get_group (number, 3) > 0)
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("e"));
             }
            else
            if (i == 2)
              {
                append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("mil"));
                if (get_group (number, 3) > 0)
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("e"));
              }
          }
      }
}


static void
pt_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        hundreds,
        tens_ones;

    hundreds  = GET_GROUP_HUNDREDS  (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (hundreds > 0)
      {
        if (group_value == 100)
            append_name_to_group (group, group_char_nbr, T("cem"));
        else
            append_name_to_group (group, group_char_nbr, PT_hundreds [hundreds]);

        if (tens_ones > 0)
          {
            (*group_char_nbr)++;
            append_name_to_group (group, group_char_nbr, T("e"));
            (*group_char_nbr)++;
          }
      }
}


static void
pt_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        ones,
        tens,
        tens_ones;

    ones      = GET_GROUP_ONES      (group_value);
    tens      = GET_GROUP_TENS      (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (tens_ones > 0)
      {
        if (tens_ones < 20)
            append_name_to_group (group, group_char_nbr, PT_units [tens_ones]);
        else
          {
            append_name_to_group (group, group_char_nbr, PT_tens [tens - 1]);
            if (ones > 0)
              {
                (*group_char_nbr)++;
                append_name_to_group (group, group_char_nbr, T("e"));
                (*group_char_nbr)++;
                append_name_to_group (group, group_char_nbr, PT_units [ones]);
              }
          }
      }
}


/*****************************************************************************/

static void
fb_number_to_text_conversion (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, long number,
                              int *group_start)
{
    int
        i,
        group_value,
        group_char_nbr;
    uschar_t
        group [GROUP_SIZE];

    for (i = 1; i <= 3; i++)
      {
        empty_group (group, &group_char_nbr);
        group_value = get_group (number, i);
        group_start [i] = *buffer_char_nbr;
        if (group_value > 0)
          {
            fr_output_group_hundreds (group, &group_char_nbr, group_value, i);
            fb_output_group_tens_ones (group, &group_char_nbr, group_value, i);

            if (i == 2 && group_value == 1)
                empty_group (group, &group_char_nbr);

            append_group_to_string (buffer, buffer_size, buffer_char_nbr, group, group_char_nbr);
            append_name_to_string (buffer, buffer_size, buffer_char_nbr, FR_groups [i - 1]);

            if (i == 1 && group_value > 1)
                append_name_to_string (buffer, buffer_size, buffer_char_nbr, T("s"));

            (*buffer_char_nbr)++;
          }
      }
}


static void
fb_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        ones,
        tens,
        tens_ones;

    ones      = GET_GROUP_ONES      (group_value);
    tens      = GET_GROUP_TENS      (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (tens_ones > 0)
      {
        if (tens_ones < 20)
          {
            append_name_to_group (group, group_char_nbr, FR_units [tens_ones]);
            (*group_char_nbr)++;
          }
        else
          {
            append_name_to_group (group, group_char_nbr, FB_tens [tens - 1]);
            if (tens_ones == 21
             || tens_ones == 31
             || tens_ones == 41
             || tens_ones == 51
             || tens_ones == 61
             || tens_ones == 71)
              {
                (*group_char_nbr)++;
                append_name_to_group (group, group_char_nbr, T("et"));
                (*group_char_nbr)++;
              }
            else
            if (ones > 0)
              {
                if (*group_char_nbr < GROUP_SIZE - 1)
                    group [(*group_char_nbr)++] = (uschar_t)'-';
              }
            else
            if (tens_ones == 80)
              {
                append_name_to_group (group, group_char_nbr, T("s"));
                (*group_char_nbr)++;
              }
            else
                (*group_char_nbr)++;

            if (ones > 0)
              {
                append_name_to_group (group, group_char_nbr, FR_units [ones]);
                (*group_char_nbr)++;
              }
          }
      }
}


/*****************************************************************************/

static void
da_number_to_text_conversion (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, long number,
                              int *group_start)
{
    int
        i,
        group_value,
        group_char_nbr;
    uschar_t
        group [GROUP_SIZE];

    for (i = 1; i <= 3; i++)
      {
        empty_group (group, &group_char_nbr);
        group_value = get_group (number, i);
        group_start [i] = *buffer_char_nbr;
        if (group_value > 0)
          {
            da_output_group_hundreds (group, &group_char_nbr, group_value, i);
            da_output_group_tens_ones (group, &group_char_nbr, group_value, i);

            if (i == 2 && group_value == 1)
                empty_group (group, &group_char_nbr);
            else
            if (i == 3 && group_value < 100 && *buffer_char_nbr > 0)
                append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("og"));

            append_group_to_string (buffer, buffer_size, buffer_char_nbr, group, group_char_nbr);

            if (i == 1)
                if (group_value == 1)
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("Million"));
                else
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("Millioner"));
            else
            if (i == 2)
                append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("tusind"));

          }
      }
}


static void
da_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        hundreds,
        tens_ones;

    hundreds  = GET_GROUP_HUNDREDS  (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (hundreds > 0)
      {
        if (hundreds > 1)
          {
            append_name_to_group (group, group_char_nbr, DA_units [hundreds]);
            (*group_char_nbr)++;
          }

        append_name_to_group (group, group_char_nbr, T("hundrede"));
        (*group_char_nbr)++;

        if (tens_ones > 0)
          {
            append_name_to_group (group, group_char_nbr, T("og"));
            (*group_char_nbr)++;
          }
      }
}


static void
da_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        ones,
        tens,
        tens_ones;

    ones      = GET_GROUP_ONES      (group_value);
    tens      = GET_GROUP_TENS      (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (tens_ones > 0)
      {
        if (tens_ones < 20)
          {
            append_name_to_group (group, group_char_nbr, DA_units [tens_ones]);
            (*group_char_nbr)++;
          }
        else
          {
            if (ones > 0)
              {
                append_name_to_group (group, group_char_nbr, DA_units [ones]);
                (*group_char_nbr)++;
                append_name_to_group (group, group_char_nbr, T("og"));
                (*group_char_nbr)++;
              }
            append_name_to_group (group, group_char_nbr, DA_tens [tens - 1]);
            (*group_char_nbr)++;
          }
      }
}


/*****************************************************************************/

static void
no_number_to_text_conversion (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, long number,
                              int *group_start)
{
    int
        i,
        group_value,
        group_char_nbr;
    uschar_t
        group [GROUP_SIZE];

    for (i = 1; i <= 3; i++)
      {
        empty_group (group, &group_char_nbr);
        group_value = get_group (number, i);
        group_start [i] = *buffer_char_nbr;
        if (group_value > 0)
          {
            no_output_group_hundreds (group, &group_char_nbr, group_value, i);
            no_output_group_tens_ones (group, &group_char_nbr, group_value, i);

            if (i == 2 && group_value == 1)
                empty_group (group, &group_char_nbr);

            append_group_to_string (buffer, buffer_size, buffer_char_nbr, group, group_char_nbr);

            if (i == 1)
                if (group_value == 1)
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("Million"));
                else
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("Millioner"));
            else
            if (i == 2)
                append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("tusen"));
          }
      }
}


static void
no_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        hundreds,
        tens_ones;

    hundreds  = GET_GROUP_HUNDREDS  (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (hundreds > 0)
      {
        if (hundreds > 1)
          {
            append_name_to_group (group, group_char_nbr, NO_units [hundreds]);
            (*group_char_nbr)++;
          }

        append_name_to_group (group, group_char_nbr, T("hundre"));
        (*group_char_nbr)++;

        if (tens_ones > 0)
          {
            append_name_to_group (group, group_char_nbr, T("og"));
            (*group_char_nbr)++;
          }
      }
}


static void
no_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        ones,
        tens,
        tens_ones;

    ones      = GET_GROUP_ONES      (group_value);
    tens      = GET_GROUP_TENS      (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (tens_ones > 0)
      {
        if (tens_ones < 20)
          {
            append_name_to_group (group, group_char_nbr, NO_units [tens_ones]);
            (*group_char_nbr)++;
          }
        else
          {
            if (ones > 0)
              {
                append_name_to_group (group, group_char_nbr, NO_units [ones]);
                (*group_char_nbr)++;
                append_name_to_group (group, group_char_nbr, T("og"));
                (*group_char_nbr)++;
              }
            append_name_to_group (group, group_char_nbr, NO_tens [tens - 1]);
            (*group_char_nbr)++;
          }
      }
}


/*****************************************************************************/

static void
sv_number_to_text_conversion (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, long number,
                              int *group_start)
{
    int
        i,
        group_value,
        group_char_nbr;
    uschar_t
        group [GROUP_SIZE];

    for (i = 1; i <= 3; i++)
      {
        empty_group (group, &group_char_nbr);
        group_value = get_group (number, i);
        group_start [i] = *buffer_char_nbr;
        if (group_value > 0)
          {
            sv_output_group_hundreds (group, &group_char_nbr, group_value, i);
            sv_output_group_tens_ones (group, &group_char_nbr, group_value, i);

            if (i == 1)
                group_char_nbr++;
            else
            if (i == 2 && group_value == 1)
                empty_group (group, &group_char_nbr);

            append_group_to_string (buffer, buffer_size, buffer_char_nbr, group, group_char_nbr);

            if (i == 1)
                if (group_value == 1)
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("Miljon"));
                else
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("Miljoner"));
            else
            if (i == 2)
                append_name_to_string (buffer, buffer_size, buffer_char_nbr, T("tusen"));

          }
      }
}


static void
sv_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        hundreds;

    hundreds  = GET_GROUP_HUNDREDS  (group_value);

    if (hundreds > 0)
      {
        if (hundreds > 1)
            append_name_to_group (group, group_char_nbr, SV_units [hundreds]);

        append_name_to_group (group, group_char_nbr, T("hundra"));
      }
}


static void
sv_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        ones,
        tens,
        tens_ones;

    ones      = GET_GROUP_ONES      (group_value);
    tens      = GET_GROUP_TENS      (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (tens_ones > 0)
      {
        if (tens_ones < 20)
            append_name_to_group (group, group_char_nbr, SV_units [tens_ones]);
        else
          {
            append_name_to_group (group, group_char_nbr, SV_tens [tens - 1]);
            if (ones > 0)
                append_name_to_group (group, group_char_nbr, SV_units [ones]);
          }
      }
}


/*****************************************************************************/

static void
is_number_to_text_conversion (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, long number,
                              int *group_start)
{
    int
        i,
        group_value,
        group_char_nbr;
    uschar_t
        group [GROUP_SIZE];

    for (i = 1; i <= 3; i++)
      {
        empty_group (group, &group_char_nbr);
        group_value = get_group (number, i);
        group_start [i] = *buffer_char_nbr;
        if (group_value > 0)
          {
            is_output_group_hundreds (group, &group_char_nbr, group_value, i);
            is_output_group_separator (group, &group_char_nbr, group_value, i, *buffer_char_nbr);
            is_output_group_tens_ones (group, &group_char_nbr, group_value, i);

            if (i == 2 && group_value == 1)
                empty_group (group, &group_char_nbr);

            append_group_to_string (buffer, buffer_size, buffer_char_nbr, group, group_char_nbr);

            if (i == 1)
                if (group_value == 1)
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("Miljon"));
                else
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("Miljoner"));
            else
            if (i == 2)
                    append_name_to_string_delim (buffer, buffer_size, buffer_char_nbr, T("thusund"));

          }
      }
}


static void
is_output_group_hundreds (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        hundreds;

    hundreds  = GET_GROUP_HUNDREDS  (group_value);

    if (hundreds > 0)
      {
        if (hundreds > 1)
          {
            if (hundreds < 5)
              {
                append_name_to_group (group, group_char_nbr, IS_neuters [hundreds]);
                (*group_char_nbr)++;
              }
            else
              {
                append_name_to_group (group, group_char_nbr, IS_units [hundreds]);
                (*group_char_nbr)++;
              }

            append_name_to_group (group, group_char_nbr, T("hundrud"));
            (*group_char_nbr)++;
          }
        else
          {
            append_name_to_group (group, group_char_nbr, T("hundrad"));
            (*group_char_nbr)++;
          }
      }
}


static void
is_output_group_separator (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr, int buffer_char_nbr)
{
    int
        tens_ones,
        hundreds;

    tens_ones = GET_GROUP_TENS_ONES (group_value);
    hundreds  = GET_GROUP_HUNDREDS  (group_value);

    if (tens_ones > 0 && tens_ones < 20)
      {
        if (hundreds > 0 || (group_nbr == 3 && buffer_char_nbr > 0))
          {
            append_name_to_group (group, group_char_nbr, T("og"));
            (*group_char_nbr)++;
          }
      }
}


static void
is_output_group_tens_ones (uschar_t *group, int *group_char_nbr, int group_value, int group_nbr)
{
    int
        ones,
        tens,
        tens_ones;

    ones      = GET_GROUP_ONES      (group_value);
    tens      = GET_GROUP_TENS      (group_value);
    tens_ones = GET_GROUP_TENS_ONES (group_value);

    if (tens_ones > 0)
      {
        if (group_value < 5 && group_nbr < 3)
          {
            append_name_to_group (group, group_char_nbr, IS_neuters [group_value]);
            (*group_char_nbr)++;
          }
        else
        if (tens_ones < 20)
          {
            append_name_to_group (group, group_char_nbr, IS_units [tens_ones]);
            (*group_char_nbr)++;
          }
        else
          {
            append_name_to_group (group, group_char_nbr, IS_tens [tens - 1]);
            (*group_char_nbr)++;
            if (ones > 0)
              {
                append_name_to_group (group, group_char_nbr, T("og"));
                (*group_char_nbr)++;
                append_name_to_group (group, group_char_nbr, IS_units [ones]);
                (*group_char_nbr)++;
              }
          }
      }
}


/*****************************************************************************/

static void
rm_number_to_text_conversion (uschar_t *buffer, int buffer_size,
                              int *buffer_char_nbr, long number)
{
    int
        i,
        roman_value;

    if (number < 0 || number > 4999)
      {
        fprintf (stderr, "Overflow error.\nYou must supply a positive");
        fprintf (stderr, " integer less or equal to 4999.\n");
        exit (0);
      }

    roman_value = number;
    for (i = 0; STRING_CMP (RM_text [i], T(" ")) != 0; i++)
        while (roman_value >= RM_numeral [i])
        {
            append_name_to_string (buffer, buffer_size, buffer_char_nbr, RM_text [i]);
            roman_value -= RM_numeral [i];
        }
}


/*****************************************************************************/

static int
get_group (long number, int group_nbr)
{
    int
        group_value = 0;

    ASSERT (group_nbr >= 1 && group_nbr <= 3);
    if (group_nbr == 1)
        group_value = GET_GROUP1 (number);
    else
    if (group_nbr == 2)
        group_value = GET_GROUP2 (number);
    else
    if (group_nbr == 3)
        group_value = GET_GROUP3 (number);

    return (group_value);
}
