/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 29 "giscanner/scannerparser.y" /* yacc.c:339  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <glib/gstdio.h>
#include "sourcescanner.h"
#include "scannerparser.h"

extern FILE *yyin;
extern int lineno;
extern char linebuf[2000];
extern char *yytext;

extern int yylex (GISourceScanner *scanner);
static void yyerror (GISourceScanner *scanner, const char *str);

extern void ctype_free (GISourceType * type);

static int last_enum_value = -1;
static gboolean is_bitfield;

/**
 * parse_c_string_literal:
 * @str: A string containing a C string literal
 *
 * Based on g_strcompress(), but also handles
 * hexadecimal escapes.
 */
static char *
parse_c_string_literal (const char *str)
{
  const gchar *p = str, *num;
  gchar *dest = g_malloc (strlen (str) + 1);
  gchar *q = dest;

  while (*p)
    {
      if (*p == '\\')
        {
          p++;
          switch (*p)
            {
            case '\0':
              g_warning ("parse_c_string_literal: trailing \\");
              goto out;
            case '0':  case '1':  case '2':  case '3':  case '4':
            case '5':  case '6':  case '7':
              *q = 0;
              num = p;
              while ((p < num + 3) && (*p >= '0') && (*p <= '7'))
                {
                  *q = (*q * 8) + (*p - '0');
                  p++;
                }
              q++;
              p--;
              break;
	    case 'x':
	      *q = 0;
	      p++;
	      num = p;
	      while ((p < num + 2) && (g_ascii_isxdigit(*p)))
		{
		  *q = (*q * 16) + g_ascii_xdigit_value(*p);
		  p++;
		}
              q++;
              p--;
	      break;
            case 'b':
              *q++ = '\b';
              break;
            case 'f':
              *q++ = '\f';
              break;
            case 'n':
              *q++ = '\n';
              break;
            case 'r':
              *q++ = '\r';
              break;
            case 't':
              *q++ = '\t';
              break;
            default:            /* Also handles \" and \\ */
              *q++ = *p;
              break;
            }
        }
      else
        *q++ = *p;
      p++;
    }
out:
  *q = 0;

  return dest;
}

enum {
  IRRELEVANT = 1,
  NOT_GI_SCANNER = 2,
  FOR_GI_SCANNER = 3,
};

static void
update_skipping (GISourceScanner *scanner)
{
  GList *l;
  for (l = scanner->conditionals.head; l != NULL; l = g_list_next (l))
    {
      if (GPOINTER_TO_INT (l->data) == NOT_GI_SCANNER)
        {
           scanner->skipping = TRUE;
           return;
        }
    }

  scanner->skipping = FALSE;
}

static void
push_conditional (GISourceScanner *scanner,
                  gint type)
{
  g_assert (type != 0);
  g_queue_push_head (&scanner->conditionals, GINT_TO_POINTER (type));
}

static gint
pop_conditional (GISourceScanner *scanner)
{
  gint type = GPOINTER_TO_INT (g_queue_pop_head (&scanner->conditionals));

  if (type == 0)
    {
      gchar *filename = g_file_get_path (scanner->current_file);
      fprintf (stderr, "%s:%d: mismatched %s", filename, lineno, yytext);
      g_free (filename);
    }

  return type;
}

static void
warn_if_cond_has_gi_scanner (GISourceScanner *scanner,
                             const gchar *text)
{
  /* Some other conditional that is not __GI_SCANNER__ */
  if (strstr (text, "__GI_SCANNER__"))
    {
      gchar *filename = g_file_get_path (scanner->current_file);
      fprintf (stderr, "%s:%d: the __GI_SCANNER__ constant should only be used with simple #ifdef or #endif: %s",
               filename, lineno, text);
      g_free (filename);
    }
}

static void
toggle_conditional (GISourceScanner *scanner)
{
  switch (pop_conditional (scanner))
    {
    case FOR_GI_SCANNER:
      push_conditional (scanner, NOT_GI_SCANNER);
      break;
    case NOT_GI_SCANNER:
      push_conditional (scanner, FOR_GI_SCANNER);
      break;
    case 0:
      break;
    default:
      push_conditional (scanner, IRRELEVANT);
      break;
    }
}

static void
set_or_merge_base_type (GISourceType *type,
                        GISourceType *base)
{
  if (base->type == CTYPE_INVALID)
    {
      g_assert (base->base_type == NULL);

      type->storage_class_specifier |= base->storage_class_specifier;
      type->type_qualifier |= base->type_qualifier;
      type->function_specifier |= base->function_specifier;
      type->is_bitfield |= base->is_bitfield;

      ctype_free (base);
    }
  else
    {
      g_assert (type->base_type == NULL);

      type->base_type = base;
    }
}


#line 270 "giscanner/scannerparser.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_YY_GISCANNER_SCANNERPARSER_H_INCLUDED
# define YY_YY_GISCANNER_SCANNERPARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IDENTIFIER = 258,
    TYPEDEF_NAME = 259,
    INTEGER = 260,
    FLOATING = 261,
    BOOLEAN = 262,
    CHARACTER = 263,
    STRING = 264,
    INTL_CONST = 265,
    INTUL_CONST = 266,
    ELLIPSIS = 267,
    ADDEQ = 268,
    SUBEQ = 269,
    MULEQ = 270,
    DIVEQ = 271,
    MODEQ = 272,
    XOREQ = 273,
    ANDEQ = 274,
    OREQ = 275,
    SL = 276,
    SR = 277,
    SLEQ = 278,
    SREQ = 279,
    EQ = 280,
    NOTEQ = 281,
    LTEQ = 282,
    GTEQ = 283,
    ANDAND = 284,
    OROR = 285,
    PLUSPLUS = 286,
    MINUSMINUS = 287,
    ARROW = 288,
    AUTO = 289,
    BOOL = 290,
    BREAK = 291,
    CASE = 292,
    CHAR = 293,
    CONST = 294,
    CONTINUE = 295,
    DEFAULT = 296,
    DO = 297,
    DOUBLE = 298,
    ELSE = 299,
    ENUM = 300,
    EXTENSION = 301,
    EXTERN = 302,
    FLOAT = 303,
    FOR = 304,
    GOTO = 305,
    IF = 306,
    INLINE = 307,
    INT = 308,
    LONG = 309,
    REGISTER = 310,
    RESTRICT = 311,
    RETURN = 312,
    SHORT = 313,
    SIGNED = 314,
    SIZEOF = 315,
    STATIC = 316,
    STRUCT = 317,
    SWITCH = 318,
    THREAD_LOCAL = 319,
    TYPEDEF = 320,
    UNION = 321,
    UNSIGNED = 322,
    VOID = 323,
    VOLATILE = 324,
    WHILE = 325,
    FUNCTION_MACRO = 326,
    OBJECT_MACRO = 327,
    IFDEF_GI_SCANNER = 328,
    IFNDEF_GI_SCANNER = 329,
    IFDEF_COND = 330,
    IFNDEF_COND = 331,
    IF_COND = 332,
    ELIF_COND = 333,
    ELSE_COND = 334,
    ENDIF_COND = 335
  };
#endif
/* Tokens.  */
#define IDENTIFIER 258
#define TYPEDEF_NAME 259
#define INTEGER 260
#define FLOATING 261
#define BOOLEAN 262
#define CHARACTER 263
#define STRING 264
#define INTL_CONST 265
#define INTUL_CONST 266
#define ELLIPSIS 267
#define ADDEQ 268
#define SUBEQ 269
#define MULEQ 270
#define DIVEQ 271
#define MODEQ 272
#define XOREQ 273
#define ANDEQ 274
#define OREQ 275
#define SL 276
#define SR 277
#define SLEQ 278
#define SREQ 279
#define EQ 280
#define NOTEQ 281
#define LTEQ 282
#define GTEQ 283
#define ANDAND 284
#define OROR 285
#define PLUSPLUS 286
#define MINUSMINUS 287
#define ARROW 288
#define AUTO 289
#define BOOL 290
#define BREAK 291
#define CASE 292
#define CHAR 293
#define CONST 294
#define CONTINUE 295
#define DEFAULT 296
#define DO 297
#define DOUBLE 298
#define ELSE 299
#define ENUM 300
#define EXTENSION 301
#define EXTERN 302
#define FLOAT 303
#define FOR 304
#define GOTO 305
#define IF 306
#define INLINE 307
#define INT 308
#define LONG 309
#define REGISTER 310
#define RESTRICT 311
#define RETURN 312
#define SHORT 313
#define SIGNED 314
#define SIZEOF 315
#define STATIC 316
#define STRUCT 317
#define SWITCH 318
#define THREAD_LOCAL 319
#define TYPEDEF 320
#define UNION 321
#define UNSIGNED 322
#define VOID 323
#define VOLATILE 324
#define WHILE 325
#define FUNCTION_MACRO 326
#define OBJECT_MACRO 327
#define IFDEF_GI_SCANNER 328
#define IFNDEF_GI_SCANNER 329
#define IFDEF_COND 330
#define IFNDEF_COND 331
#define IF_COND 332
#define ELIF_COND 333
#define ELSE_COND 334
#define ENDIF_COND 335

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 234 "giscanner/scannerparser.y" /* yacc.c:355  */

  char *str;
  GList *list;
  GISourceSymbol *symbol;
  GISourceType *ctype;
  StorageClassSpecifier storage_class_specifier;
  TypeQualifier type_qualifier;
  FunctionSpecifier function_specifier;
  UnaryOperator unary_operator;

#line 481 "giscanner/scannerparser.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (GISourceScanner* scanner);

#endif /* !YY_YY_GISCANNER_SCANNERPARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 498 "giscanner/scannerparser.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  77
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2610

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  105
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  77
/* YYNRULES -- Number of rules.  */
#define YYNRULES  257
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  427

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   335

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    94,     2,     2,     2,    96,    89,     2,
      81,    82,    90,    91,    88,    92,    87,    95,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   102,   104,
      97,   103,    98,   101,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    85,     2,    86,    99,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    83,   100,    84,    93,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   326,   326,   335,   351,   357,   363,   370,   371,   375,
     383,   398,   412,   419,   420,   424,   425,   429,   433,   437,
     441,   445,   449,   456,   457,   461,   462,   466,   470,   493,
     500,   507,   511,   519,   523,   527,   531,   535,   539,   546,
     547,   559,   560,   566,   574,   585,   586,   592,   601,   602,
     614,   623,   624,   630,   636,   642,   651,   652,   658,   667,
     668,   677,   678,   687,   688,   697,   698,   709,   710,   721,
     722,   729,   730,   737,   738,   739,   740,   741,   742,   743,
     744,   745,   746,   747,   751,   752,   753,   760,   766,   784,
     791,   796,   801,   814,   815,   820,   825,   830,   838,   842,
     849,   850,   854,   858,   862,   866,   870,   874,   881,   885,
     889,   893,   897,   901,   905,   909,   913,   917,   921,   922,
     923,   931,   951,   956,   964,   969,   977,   978,   985,  1005,
    1010,  1011,  1016,  1024,  1028,  1036,  1039,  1040,  1044,  1055,
    1062,  1069,  1076,  1083,  1090,  1099,  1099,  1108,  1116,  1124,
    1136,  1140,  1144,  1148,  1155,  1162,  1167,  1171,  1176,  1180,
    1185,  1190,  1200,  1207,  1216,  1221,  1225,  1236,  1249,  1250,
    1257,  1261,  1268,  1273,  1278,  1283,  1290,  1296,  1305,  1306,
    1310,  1315,  1316,  1324,  1328,  1333,  1338,  1343,  1348,  1354,
    1364,  1370,  1383,  1390,  1391,  1392,  1396,  1397,  1403,  1404,
    1405,  1406,  1407,  1408,  1412,  1413,  1414,  1418,  1419,  1423,
    1424,  1428,  1429,  1433,  1434,  1438,  1439,  1440,  1444,  1445,
    1446,  1447,  1448,  1449,  1450,  1451,  1452,  1453,  1457,  1458,
    1459,  1460,  1461,  1467,  1468,  1472,  1473,  1474,  1478,  1479,
    1483,  1484,  1490,  1497,  1504,  1508,  1525,  1530,  1535,  1540,
    1545,  1550,  1557,  1562,  1570,  1571,  1572,  1573
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "\"identifier\"", "\"typedef-name\"",
  "INTEGER", "FLOATING", "BOOLEAN", "CHARACTER", "STRING", "INTL_CONST",
  "INTUL_CONST", "ELLIPSIS", "ADDEQ", "SUBEQ", "MULEQ", "DIVEQ", "MODEQ",
  "XOREQ", "ANDEQ", "OREQ", "SL", "SR", "SLEQ", "SREQ", "EQ", "NOTEQ",
  "LTEQ", "GTEQ", "ANDAND", "OROR", "PLUSPLUS", "MINUSMINUS", "ARROW",
  "AUTO", "BOOL", "BREAK", "CASE", "CHAR", "CONST", "CONTINUE", "DEFAULT",
  "DO", "DOUBLE", "ELSE", "ENUM", "EXTENSION", "EXTERN", "FLOAT", "FOR",
  "GOTO", "IF", "INLINE", "INT", "LONG", "REGISTER", "RESTRICT", "RETURN",
  "SHORT", "SIGNED", "SIZEOF", "STATIC", "STRUCT", "SWITCH",
  "THREAD_LOCAL", "TYPEDEF", "UNION", "UNSIGNED", "VOID", "VOLATILE",
  "WHILE", "FUNCTION_MACRO", "OBJECT_MACRO", "IFDEF_GI_SCANNER",
  "IFNDEF_GI_SCANNER", "IFDEF_COND", "IFNDEF_COND", "IF_COND", "ELIF_COND",
  "ELSE_COND", "ENDIF_COND", "'('", "')'", "'{'", "'}'", "'['", "']'",
  "'.'", "','", "'&'", "'*'", "'+'", "'-'", "'~'", "'!'", "'/'", "'%'",
  "'<'", "'>'", "'^'", "'|'", "'?'", "':'", "'='", "';'", "$accept",
  "primary_expression", "strings", "identifier",
  "identifier_or_typedef_name", "postfix_expression",
  "argument_expression_list", "unary_expression", "unary_operator",
  "cast_expression", "multiplicative_expression", "additive_expression",
  "shift_expression", "relational_expression", "equality_expression",
  "and_expression", "exclusive_or_expression", "inclusive_or_expression",
  "logical_and_expression", "logical_or_expression",
  "conditional_expression", "assignment_expression", "assignment_operator",
  "expression", "constant_expression", "declaration",
  "declaration_specifiers", "init_declarator_list", "init_declarator",
  "storage_class_specifier", "type_specifier", "struct_or_union_specifier",
  "struct_or_union", "struct_declaration_list", "struct_declaration",
  "specifier_qualifier_list", "struct_declarator_list",
  "struct_declarator", "enum_specifier", "enum_keyword", "enumerator_list",
  "$@1", "enumerator", "type_qualifier", "function_specifier",
  "declarator", "direct_declarator", "pointer", "type_qualifier_list",
  "parameter_list", "parameter_declaration", "identifier_list",
  "type_name", "abstract_declarator", "direct_abstract_declarator",
  "typedef_name", "initializer", "initializer_list", "statement",
  "labeled_statement", "compound_statement", "block_item_list",
  "block_item", "expression_statement", "selection_statement",
  "iteration_statement", "jump_statement", "translation_unit",
  "external_declaration", "function_definition", "declaration_list",
  "function_macro", "object_macro", "function_macro_define",
  "object_macro_define", "preproc_conditional", "macro", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,    40,    41,   123,   125,    91,    93,    46,    44,    38,
      42,    43,    45,   126,    33,    47,    37,    60,    62,    94,
     124,    63,    58,    61,    59
};
# endif

#define YYPACT_NINF -237

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-237)))

#define YYTABLE_NINF -15

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    2256,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,    39,  2505,
    2505,  -237,    23,  -237,    61,  2505,  2505,  -237,  2133,  -237,
    -237,   -47,  1783,  -237,  -237,  -237,  -237,  -237,    34,   125,
    -237,  -237,   -45,  -237,  1092,    87,    33,  -237,  -237,  2541,
    -237,   -35,  -237,  -237,   -29,  -237,  -237,  -237,  -237,    66,
    -237,  -237,  -237,  -237,  -237,   -36,    14,  1825,  1825,    36,
    1881,  1226,  -237,  -237,  -237,  -237,  -237,  -237,  -237,   116,
    -237,   157,  -237,  1783,  -237,    84,   216,   154,    11,   278,
       4,    67,    35,   130,   -11,  -237,  -237,   122,  -237,  -237,
     125,    34,  -237,   471,  1335,  -237,    39,  -237,  2372,  2334,
    1391,    87,  2541,  2179,  -237,    32,  2541,  2541,   -10,    66,
    -237,  -237,    69,  1825,  1825,  1923,  -237,  -237,   131,  1226,
    -237,  1979,   565,  -237,  -237,   105,   120,   159,  -237,  -237,
    -237,   213,  1433,  1923,   213,  -237,  1783,  1783,  1783,  1783,
    1783,  1783,  1783,  1783,  1783,  1783,  1783,  1783,  1783,  1783,
    1783,  1783,  1783,  1783,  1923,  -237,  -237,  -237,  -237,   145,
     152,  1783,   155,   166,   816,   190,   213,   205,   882,   210,
     234,  -237,  -237,   181,   192,   -44,  -237,   219,  -237,  -237,
    -237,   563,  -237,  -237,  -237,  -237,  -237,  1335,  -237,  -237,
    -237,  -237,  -237,  -237,    47,   110,  -237,   121,  -237,   243,
    -237,  -237,  -237,  1783,   -20,  -237,   228,  -237,  2216,  -237,
      12,   231,  -237,   204,  -237,    66,   249,   254,  1979,   747,
     255,  1159,   250,  -237,  -237,  -237,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,  1783,  -237,  1783,  2063,  1489,   212,
    -237,   215,  1783,  -237,  -237,   124,  -237,    60,  -237,  -237,
    -237,  -237,    84,    84,   216,   216,   154,   154,   154,   154,
      11,    11,   278,     4,    67,    35,   130,   -39,  -237,   237,
    -237,   816,   270,   924,   241,  1923,  -237,   -18,  1923,  1923,
     816,  -237,  -237,  -237,  -237,   217,  1997,  -237,    22,  -237,
    -237,  2469,  -237,  -237,  -237,    32,  -237,  1783,  -237,  -237,
    -237,  1783,  -237,    15,  -237,  -237,  -237,   655,  -237,  -237,
    -237,  -237,   158,   259,  -237,   260,   215,  2431,  1531,  -237,
    -237,  1783,  -237,  1923,   816,  -237,   272,   988,   -17,  -237,
     163,  -237,   167,   178,  -237,  -237,  1293,  -237,  -237,  -237,
    -237,  -237,   273,  -237,  -237,  -237,  -237,   179,  -237,   271,
    -237,   250,  -237,  1923,  1587,    57,  1030,   816,   816,   816,
    -237,  -237,  -237,  -237,  -237,   182,   816,   187,  1629,  1685,
      65,   312,  -237,  -237,   257,  -237,   816,   816,   191,   816,
     194,  1727,   816,  -237,  -237,  -237,   816,  -237,   816,   816,
     202,  -237,  -237,  -237,  -237,   816,  -237
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,   257,   192,   105,   117,   109,   150,   114,   144,   152,
     103,   113,   154,   111,   112,   106,   151,   110,   115,   104,
     124,   107,   102,   125,   116,   108,   153,   242,   243,   246,
     247,   248,   249,   250,   251,   252,   253,   236,     0,    91,
      93,   118,     0,   119,     0,    95,    97,   120,     0,   233,
     235,     0,     0,   254,   255,   256,   237,    12,     0,   165,
      89,   157,     0,    98,   100,   156,     0,    90,    92,     0,
      13,   123,    14,   145,   143,    94,    96,     1,   234,     0,
       3,     6,     4,     5,    10,     0,     0,     0,     0,     0,
       0,     0,    33,    34,    35,    36,    37,    38,    15,     7,
       2,    25,    39,     0,    41,    45,    48,    51,    56,    59,
      61,    63,    65,    67,    69,    87,   245,     0,   168,   167,
     164,     0,    88,     0,     0,   240,     0,   239,     0,     0,
       0,   155,   130,     0,   126,   135,   132,     0,     0,     0,
     145,   176,     0,     0,     0,     0,    26,    27,     0,     0,
      31,   152,    39,    71,    84,     0,   178,     0,    11,    21,
      22,     0,     0,     0,     0,    28,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   158,   169,   166,    99,   100,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   207,   213,     2,     0,     0,   211,   120,   212,   198,
     199,     0,   209,   200,   201,   202,   203,     0,   193,   101,
     241,   238,   175,   163,   174,     0,   170,     0,   160,     0,
     129,   122,   127,     0,     0,   133,   136,   131,     0,   140,
       0,   148,   146,     0,   244,     0,     0,     0,     0,     0,
       0,     0,    86,    77,    78,    74,    75,    76,    82,    81,
      83,    79,    80,    73,     0,     8,     0,     0,     0,   180,
     179,   181,     0,    20,    18,     0,    23,     0,    19,    42,
      43,    44,    46,    47,    49,    50,    54,    55,    52,    53,
      57,    58,    60,    62,    64,    66,    68,     0,   230,     0,
     229,     0,     0,     0,     0,     0,   231,     0,     0,     0,
       0,   214,   208,   210,   196,     0,     0,   172,   180,   173,
     161,     0,   162,   159,   137,   135,   128,     0,   121,   142,
     147,     0,   139,     0,   177,    29,    30,     0,    32,    72,
      85,   188,     0,     0,   184,     0,   182,     0,     0,    40,
      17,     0,    16,     0,     0,   206,     0,     0,     0,   228,
       0,   232,     0,     0,   204,   194,     0,   171,   134,   138,
     149,   141,     0,   189,   183,   185,   190,     0,   186,     0,
      24,    70,   205,     0,     0,     0,     0,     0,     0,     0,
     195,   197,     9,   191,   187,     0,     0,     0,     0,     0,
       0,   215,   217,   218,     0,   220,     0,     0,     0,     0,
       0,     0,     0,   219,   224,   222,     0,   221,     0,     0,
       0,   216,   226,   225,   223,     0,   227
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -237,  -237,  -237,   -38,   -34,  -237,  -237,    52,  -237,   -87,
     141,   142,   144,   149,   180,   183,   184,   185,   189,  -237,
     -50,  -101,  -237,   -51,  -177,     3,     7,  -237,   245,  -237,
     -60,  -237,  -237,   225,  -120,   -74,  -237,    45,  -237,  -237,
     224,  -237,   252,   253,  -237,   -16,   -65,   -54,  -237,  -108,
      71,   264,   233,  -132,  -236,   -12,  -206,  -237,    42,  -237,
      -7,   139,  -199,  -237,  -237,  -237,  -237,  -237,   350,  -237,
    -237,  -237,  -237,  -237,  -237,  -237,  -237
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    98,    99,   100,   204,   101,   275,   152,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     153,   154,   264,   205,   116,   206,   126,    62,    63,    39,
      40,    41,    42,   133,   134,   135,   234,   235,    43,    44,
     138,   139,   330,    45,    46,   117,    65,    66,   120,   342,
     226,   142,   157,   343,   271,    47,   219,   315,   208,   209,
     210,   211,   212,   213,   214,   215,   216,    48,    49,    50,
     128,    51,    52,    53,    54,    55,    56
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      61,   131,   115,    37,    70,   119,    70,    38,    71,   132,
      74,   314,   313,   232,   299,    57,   165,   156,    57,   183,
      61,   225,    64,   218,   270,    57,    57,     2,    61,   229,
      72,   132,    72,   346,    79,    57,    57,    57,   173,   174,
     155,   141,    57,   121,   266,   143,    67,    68,   137,   266,
      57,    37,    75,    76,   140,    38,   324,   127,   230,   122,
     311,   276,   237,   353,    57,     2,   187,   125,   325,    57,
     266,   266,   132,   132,   239,   156,   132,   132,   240,   279,
     280,   281,   346,    61,   326,   203,   361,   386,    61,   132,
     184,   141,   319,   179,   155,   144,   329,    61,   155,   371,
     252,   241,   269,   316,   102,   189,    69,   268,   175,   176,
     189,   207,   277,    58,    58,    58,   218,   148,   232,   236,
      58,   221,    59,    70,    59,   158,    70,   273,   316,    59,
     278,   220,   268,   297,   233,   181,   224,    59,   313,   146,
     147,   115,   150,    60,    73,   266,   352,   307,   266,    72,
     369,   244,    72,   266,   370,   102,   203,   245,    70,   182,
     391,   398,   304,   339,     6,   340,   180,   345,   129,   411,
     318,     9,   130,   203,   166,   171,   172,   156,   132,   167,
     168,    16,    72,   115,    72,   349,    61,   265,   159,   160,
     161,   132,   320,   266,    26,   246,   247,   252,   321,   207,
     155,   267,   241,   322,   185,   268,   350,   334,   317,   245,
      59,   203,   351,   269,   249,    59,    57,     2,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   302,   207,   162,   377,
     373,   272,   163,   102,   164,   387,   321,   379,   124,   388,
     380,   266,   358,   131,   360,   266,   298,   362,   363,   300,
     389,   393,   318,   203,   404,   218,   266,   321,   301,   406,
     266,   303,   203,   416,   224,   266,   418,   115,    61,   266,
      61,   115,   266,   -13,   425,   102,   305,    61,   332,    72,
     266,   308,   333,   267,   310,   241,   347,   268,    72,   203,
     348,   365,   381,   177,   178,   366,   385,   169,   170,   236,
     282,   283,   118,   284,   285,   309,   203,   286,   287,   288,
     289,   -14,   136,   224,   102,   207,   290,   291,   224,   323,
     327,   335,   395,   397,   331,   400,   336,   338,   266,   354,
     356,   374,    72,   355,   136,   359,   375,   408,   410,   203,
     203,   203,   364,   383,   224,   392,   412,   394,   203,   292,
     420,   413,   238,   293,   243,   294,   188,   295,   203,   203,
     368,   203,   296,   186,   203,    72,    72,    72,   203,   102,
     203,   203,   250,   102,    72,   136,   136,   203,   337,   136,
     136,   242,   367,   227,    72,    72,   382,    72,    78,     0,
      72,     0,   136,     0,    72,     0,    72,    72,     0,     0,
       0,     0,     0,    72,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   401,
     402,   403,     0,     0,     0,     0,     0,     0,   405,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   414,   415,
       0,   417,     0,     0,   421,     0,     0,     0,   422,     0,
     423,   424,     0,     0,     0,     0,     0,   426,     0,     0,
       0,     0,     0,     0,    57,     2,    80,    81,    82,    83,
      84,    85,    86,     0,     0,     0,     0,     0,     0,     0,
       0,   136,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    87,    88,   136,     3,     4,   190,   191,     5,
       6,   192,   193,   194,     7,     0,     8,   151,    10,    11,
     195,   196,   197,    12,    13,    14,    15,    16,   198,    17,
      18,    90,    19,    20,   199,    21,    22,    23,    24,    25,
      26,   200,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    91,     0,   123,   201,     0,     0,     0,     0,
      92,    93,    94,    95,    96,    97,    57,     2,    80,    81,
      82,    83,    84,    85,    86,   202,     0,     0,   253,   254,
     255,   256,   257,   258,   259,   260,     0,     0,   261,   262,
       0,     0,     0,     0,    87,    88,     0,     3,     4,   190,
     191,     5,     6,   192,   193,   194,     7,     0,     8,   151,
      10,    11,   195,   196,   197,    12,    13,    14,    15,    16,
     198,    17,    18,    90,    19,    20,   199,    21,    22,    23,
      24,    25,    26,   200,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    91,     0,   123,   312,     0,     0,
       0,     0,    92,    93,    94,    95,    96,    97,    57,     2,
      80,    81,    82,    83,    84,    85,    86,   202,   263,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    87,    88,     0,     3,
       4,   190,   191,     5,     6,   192,   193,   194,     7,     0,
       8,   151,    10,    11,   195,   196,   197,    12,    13,    14,
      15,    16,   198,    17,    18,    90,    19,    20,   199,    21,
      22,    23,    24,    25,    26,   200,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    91,     0,   123,   372,
       0,     0,     0,     0,    92,    93,    94,    95,    96,    97,
      57,     2,    80,    81,    82,    83,    84,    85,    86,   202,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    87,    88,
       0,     3,     4,   190,   191,     5,     6,   192,   193,   194,
       7,     0,     8,   151,    10,    11,   195,   196,   197,    12,
      13,    14,    15,    16,   198,    17,    18,    90,    19,    20,
     199,    21,    22,    23,    24,    25,    26,   200,     0,    57,
       2,    80,    81,    82,    83,    84,    85,    86,    91,     0,
     123,     0,     0,     0,     0,     0,    92,    93,    94,    95,
      96,    97,     0,     0,     0,     0,     0,    87,    88,     0,
       0,   202,   190,   191,     0,     0,   192,   193,   194,     0,
       0,     0,   248,     0,     0,   195,   196,   197,     0,     0,
       0,     0,     0,   198,     0,     0,    90,     0,     0,   199,
       0,     0,     0,     0,     0,    57,   200,    80,    81,    82,
      83,    84,    85,    86,     0,     0,     0,    91,     0,   123,
       0,     0,     0,     0,     0,    92,    93,    94,    95,    96,
      97,     0,     0,    87,    88,     0,     0,     0,     0,     0,
     202,     0,     0,     0,     0,     0,     0,    57,   248,    80,
      81,    82,    83,    84,    85,    86,     0,     0,     0,     0,
       0,     0,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    87,    88,     0,     0,     0,
       0,     0,     0,    91,     0,     0,     0,     0,     0,     0,
     248,    92,    93,    94,    95,    96,    97,     0,     0,     0,
       0,     0,     0,     0,    90,     0,   306,     0,     0,     0,
       0,    57,     0,    80,    81,    82,    83,    84,    85,    86,
       0,     0,     0,     0,     0,    91,     0,     0,     0,     0,
       0,     0,     0,    92,    93,    94,    95,    96,    97,    87,
      88,     0,     0,     0,     0,     0,     0,     0,   357,     0,
       0,     0,     0,    57,   248,    80,    81,    82,    83,    84,
      85,    86,     0,     0,     0,     0,     0,     0,    90,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    87,    88,     0,     0,     0,     0,     0,     0,    91,
       0,     0,     0,     0,     0,     0,   248,    92,    93,    94,
      95,    96,    97,     0,     0,     0,     0,     0,     0,     0,
      90,     0,   384,     0,     0,     0,     2,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    91,     0,     0,     0,     0,     0,     0,     0,    92,
      93,    94,    95,    96,    97,     0,     3,     4,     0,     0,
       5,     6,     0,     0,   399,     7,     0,     8,     9,    10,
      11,     0,     0,     0,    12,    13,    14,    15,    16,     0,
      17,    18,     0,    19,    20,     0,    21,    22,    23,    24,
      25,    26,    57,     2,    80,    81,    82,    83,    84,    85,
      86,     0,     0,     0,     0,   123,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      87,    88,     0,     0,     4,   124,     0,     5,     6,     0,
       0,     0,     7,     0,     8,   151,     0,    11,     0,     0,
       0,     0,    13,    14,     0,    16,     0,    17,    18,    90,
       0,    20,     0,     0,     0,    23,    24,    25,    26,    57,
       2,    80,    81,    82,    83,    84,    85,    86,     0,     0,
      91,     0,   249,     0,     0,     0,     0,     0,    92,    93,
      94,    95,    96,    97,     0,     0,     0,    87,    88,     0,
       0,     4,     0,     0,     5,     6,     0,     0,     0,     7,
       0,     8,   151,     0,    11,     0,     0,     0,     0,    13,
      14,     0,    16,     0,    17,    18,    90,     0,    20,     0,
       0,     0,    23,    24,    25,    26,    57,     0,    80,    81,
      82,    83,    84,    85,    86,     0,     0,    91,     0,     0,
       0,     0,     0,     0,     0,    92,    93,    94,    95,    96,
      97,     0,     0,     0,    87,    88,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    57,    89,
      80,    81,    82,    83,    84,    85,    86,     0,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    87,    88,     0,     0,
       0,     0,     0,     0,    91,     0,   217,   390,     0,     0,
       0,    89,    92,    93,    94,    95,    96,    97,     0,     0,
       0,     0,     0,     0,    57,    90,    80,    81,    82,    83,
      84,    85,    86,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    91,     0,   217,     0,
       0,     0,    87,    88,    92,    93,    94,    95,    96,    97,
       0,     0,     0,     0,     0,     0,    57,    89,    80,    81,
      82,    83,    84,    85,    86,     0,     0,     0,     0,     0,
       0,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    87,    88,     0,     0,     0,     0,
       0,     0,    91,     0,     0,     0,     0,   228,     0,    89,
      92,    93,    94,    95,    96,    97,     0,     0,     0,     0,
       0,     0,    57,    90,    80,    81,    82,    83,    84,    85,
      86,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    91,   274,     0,     0,     0,     0,
      87,    88,    92,    93,    94,    95,    96,    97,     0,     0,
       0,     0,     0,     0,    57,    89,    80,    81,    82,    83,
      84,    85,    86,     0,     0,     0,     0,     0,     0,    90,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    87,    88,     0,     0,     0,     0,     0,     0,
      91,     0,     0,     0,     0,   344,     0,    89,    92,    93,
      94,    95,    96,    97,     0,     0,     0,     0,     0,     0,
      57,    90,    80,    81,    82,    83,    84,    85,    86,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    91,     0,     0,     0,     0,   378,    87,    88,
      92,    93,    94,    95,    96,    97,     0,     0,     0,     0,
       0,     0,    57,   248,    80,    81,    82,    83,    84,    85,
      86,     0,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      87,    88,     0,     0,     0,     0,     0,     0,    91,   396,
       0,     0,     0,     0,     0,   248,    92,    93,    94,    95,
      96,    97,     0,     0,     0,     0,     0,     0,    57,    90,
      80,    81,    82,    83,    84,    85,    86,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      91,   407,     0,     0,     0,     0,    87,    88,    92,    93,
      94,    95,    96,    97,     0,     0,     0,     0,     0,     0,
      57,   248,    80,    81,    82,    83,    84,    85,    86,     0,
       0,     0,     0,     0,     0,    90,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    87,    88,
       0,     0,     0,     0,     0,     0,    91,   409,     0,     0,
       0,     0,     0,   248,    92,    93,    94,    95,    96,    97,
       0,     0,     0,     0,     0,     0,    57,    90,    80,    81,
      82,    83,    84,    85,    86,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    91,   419,
       0,     0,     0,     0,    87,    88,    92,    93,    94,    95,
      96,    97,     0,     0,     0,     0,     0,     0,    57,    89,
      80,    81,    82,    83,    84,    85,    86,     0,     0,     0,
       0,     0,     0,    90,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    87,    88,     0,     0,
       0,     0,     0,     0,    91,     0,     0,     0,     0,     0,
       0,    89,    92,    93,    94,    95,    96,    97,     0,     0,
       0,     0,     0,     0,    57,    90,    80,    81,    82,    83,
      84,    85,    86,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   145,     0,     0,     0,
       0,     0,    87,    88,    92,    93,    94,    95,    96,    97,
       0,     0,     0,     0,     0,     0,    57,    89,    80,    81,
      82,    83,    84,    85,    86,     0,     0,     0,     0,     0,
       0,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    87,    88,     0,     0,     0,     0,
       0,     0,   149,     0,     0,     0,     0,     0,     0,   248,
      92,    93,    94,    95,    96,    97,     0,     0,     0,     0,
       0,     0,    57,    90,    80,    81,    82,    83,    84,    85,
      86,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      57,     2,     0,     0,    91,     0,     0,     0,     0,   222,
      87,    88,    92,    93,    94,    95,    96,    97,     0,     0,
       0,     0,     0,     0,     0,   248,     0,     0,     0,     0,
       0,     3,     4,     0,     0,     5,     6,     0,     0,    90,
       7,     0,     8,     9,    10,    11,     0,     0,     0,    12,
      13,    14,    15,    16,     0,    17,    18,     0,    19,    20,
     251,    21,    22,    23,    24,    25,    26,     2,    92,    93,
      94,    95,    96,    97,     0,   222,     0,     0,   316,   341,
       0,     0,   268,     0,     0,     0,     0,    59,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     3,     4,     0,
       0,     5,     6,     0,     0,     0,     7,     0,     8,     9,
      10,    11,     0,     0,     0,    12,    13,    14,    15,    16,
       0,    17,    18,     0,    19,    20,     0,    21,    22,    23,
      24,    25,    26,    77,     1,     0,     0,     2,     0,     0,
       0,     0,     0,     0,   267,   341,     0,     0,   268,     0,
       0,     0,     0,    59,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     3,     4,     0,
       0,     5,     6,     0,     0,     0,     7,     0,     8,     9,
      10,    11,     0,     2,     0,    12,    13,    14,    15,    16,
       0,    17,    18,     0,    19,    20,     0,    21,    22,    23,
      24,    25,    26,     0,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,     4,     0,     0,     5,     6,     0,
       2,     0,     7,     0,     8,     9,     0,    11,     0,     0,
       0,     0,    13,    14,     0,    16,     0,    17,    18,     0,
       0,    20,     0,     0,     0,    23,    24,    25,    26,     0,
       0,     4,     0,     0,     5,     6,     0,     1,     0,     7,
       2,     8,     9,   231,    11,     0,     0,     0,     0,    13,
      14,     0,    16,     0,    17,    18,     0,     0,    20,     0,
       0,     0,    23,    24,    25,    26,     0,     0,     0,     0,
       3,     4,     0,     0,     5,     6,     0,     0,     0,     7,
     328,     8,     9,    10,    11,     0,     0,     0,    12,    13,
      14,    15,    16,     0,    17,    18,     0,    19,    20,     0,
      21,    22,    23,    24,    25,    26,     0,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    57,     2,     0,
       0,     0,     0,     0,     0,     0,   222,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     3,     4,
       0,     0,     5,     6,     0,     0,     2,     7,     0,     8,
       9,    10,    11,     0,     0,     0,    12,    13,    14,    15,
      16,     0,    17,    18,     0,    19,    20,     0,    21,    22,
      23,    24,    25,    26,     0,     0,     3,     4,     0,     0,
       5,     6,     0,     0,     0,     7,   223,     8,     9,    10,
      11,     0,     0,     0,    12,    13,    14,    15,    16,     0,
      17,    18,     0,    19,    20,     2,    21,    22,    23,    24,
      25,    26,     0,   222,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   123,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     3,     4,     0,     0,     5,
       6,     0,     0,     2,     7,     0,     8,     9,    10,    11,
       0,   222,     0,    12,    13,    14,    15,    16,     0,    17,
      18,     0,    19,    20,     0,    21,    22,    23,    24,    25,
      26,     0,     0,     3,     4,     0,     0,     5,     6,     2,
       0,     0,     7,   376,     8,     9,    10,    11,     0,     0,
       0,    12,    13,    14,    15,    16,     0,    17,    18,     0,
      19,    20,     0,    21,    22,    23,    24,    25,    26,     3,
       4,     0,     0,     5,     6,     2,     0,     0,     7,     0,
       8,     9,    10,    11,     0,     0,     0,    12,    13,    14,
      15,    16,     0,    17,    18,     0,    19,    20,     0,    21,
      22,    23,    24,    25,    26,     0,     4,     0,     0,     5,
       6,     0,     0,     0,     7,     0,     8,     9,     0,    11,
       0,     0,     0,     0,    13,    14,     0,    16,     0,    17,
      18,     0,     0,    20,     0,     0,     0,    23,    24,    25,
      26
};

static const yytype_int16 yycheck[] =
{
      38,    66,    52,     0,    42,    59,    44,     0,    42,    69,
      44,   217,   211,   133,   191,     3,   103,    91,     3,    30,
      58,   129,    38,   124,   156,     3,     3,     4,    66,   130,
      42,    91,    44,   269,    81,     3,     3,     3,    27,    28,
      91,    79,     3,    88,    88,    81,    39,    40,    83,    88,
       3,    48,    45,    46,    83,    48,   233,    64,   132,   104,
     104,   162,   136,   102,     3,     4,   120,    64,    88,     3,
      88,    88,   132,   133,    84,   149,   136,   137,    88,   166,
     167,   168,   318,   121,   104,   123,   104,   104,   126,   149,
     101,   129,   224,    89,   145,    81,    84,   135,   149,    84,
     151,   139,   156,    81,    52,   121,    83,    85,    97,    98,
     126,   123,   163,    81,    81,    81,   217,    81,   238,   135,
      81,   128,    90,   161,    90,     9,   164,   161,    81,    90,
     164,   128,    85,   184,   102,   100,   129,    90,   337,    87,
      88,   191,    90,   104,    83,    88,    86,   198,    88,   161,
     327,    82,   164,    88,   331,   103,   194,    88,   196,    29,
     366,   104,   196,   264,    39,   266,    99,   268,    81,   104,
     224,    46,    85,   211,    90,    21,    22,   251,   238,    95,
      96,    56,   194,   233,   196,   272,   224,    82,    31,    32,
      33,   251,    82,    88,    69,   143,   144,   248,    88,   211,
     251,    81,   240,    82,    82,    85,    82,   245,   224,    88,
      90,   249,    88,   267,    83,    90,     3,     4,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   194,   249,    81,   347,
      82,    82,    85,   191,    87,    82,    88,   348,   103,    82,
     351,    88,   303,   318,   305,    88,   104,   308,   309,   104,
      82,    82,   316,   301,    82,   366,    88,    88,   102,    82,
      88,    81,   310,    82,   267,    88,    82,   327,   316,    88,
     318,   331,    88,   102,    82,   233,    81,   325,    84,   301,
      88,    81,    88,    81,   102,   333,    81,    85,   310,   337,
      85,    84,   353,    25,    26,    88,   357,    91,    92,   325,
     169,   170,    59,   171,   172,    81,   354,   173,   174,   175,
     176,   102,    69,   316,   272,   337,   177,   178,   321,    86,
     102,    82,   383,   384,   103,   386,    82,    82,    88,   102,
      70,    82,   354,   301,    91,   104,    86,   398,   399,   387,
     388,   389,   310,    81,   347,    82,    44,    86,   396,   179,
     411,   104,   137,   180,   140,   181,   121,   182,   406,   407,
     325,   409,   183,   120,   412,   387,   388,   389,   416,   327,
     418,   419,   149,   331,   396,   132,   133,   425,   249,   136,
     137,   139,   321,   129,   406,   407,   354,   409,    48,    -1,
     412,    -1,   149,    -1,   416,    -1,   418,   419,    -1,    -1,
      -1,    -1,    -1,   425,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   387,
     388,   389,    -1,    -1,    -1,    -1,    -1,    -1,   396,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   406,   407,
      -1,   409,    -1,    -1,   412,    -1,    -1,    -1,   416,    -1,
     418,   419,    -1,    -1,    -1,    -1,    -1,   425,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   238,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    32,   251,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    -1,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    83,    84,    -1,    -1,    -1,    -1,
      89,    90,    91,    92,    93,    94,     3,     4,     5,     6,
       7,     8,     9,    10,    11,   104,    -1,    -1,    13,    14,
      15,    16,    17,    18,    19,    20,    -1,    -1,    23,    24,
      -1,    -1,    -1,    -1,    31,    32,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,     3,     4,
       5,     6,     7,     8,     9,    10,    11,   104,   103,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    32,    -1,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    -1,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    83,    84,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
       3,     4,     5,     6,     7,     8,     9,    10,    11,   104,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    32,
      -1,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    -1,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    81,    -1,
      83,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    -1,    -1,    -1,    -1,    -1,    31,    32,    -1,
      -1,   104,    36,    37,    -1,    -1,    40,    41,    42,    -1,
      -1,    -1,    46,    -1,    -1,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    57,    -1,    -1,    60,    -1,    -1,    63,
      -1,    -1,    -1,    -1,    -1,     3,    70,     5,     6,     7,
       8,     9,    10,    11,    -1,    -1,    -1,    81,    -1,    83,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    -1,    -1,    31,    32,    -1,    -1,    -1,    -1,    -1,
     104,    -1,    -1,    -1,    -1,    -1,    -1,     3,    46,     5,
       6,     7,     8,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,
      46,    89,    90,    91,    92,    93,    94,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    60,    -1,   104,    -1,    -1,    -1,
      -1,     3,    -1,     5,     6,     7,     8,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    31,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,    -1,
      -1,    -1,    -1,     3,    46,     5,     6,     7,     8,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    60,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    32,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    -1,   104,    -1,    -1,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    94,    -1,    34,    35,    -1,    -1,
      38,    39,    -1,    -1,   104,    43,    -1,    45,    46,    47,
      48,    -1,    -1,    -1,    52,    53,    54,    55,    56,    -1,
      58,    59,    -1,    61,    62,    -1,    64,    65,    66,    67,
      68,    69,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    -1,    -1,    -1,    -1,    83,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    32,    -1,    -1,    35,   103,    -1,    38,    39,    -1,
      -1,    -1,    43,    -1,    45,    46,    -1,    48,    -1,    -1,
      -1,    -1,    53,    54,    -1,    56,    -1,    58,    59,    60,
      -1,    62,    -1,    -1,    -1,    66,    67,    68,    69,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    -1,    -1,
      81,    -1,    83,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    92,    93,    94,    -1,    -1,    -1,    31,    32,    -1,
      -1,    35,    -1,    -1,    38,    39,    -1,    -1,    -1,    43,
      -1,    45,    46,    -1,    48,    -1,    -1,    -1,    -1,    53,
      54,    -1,    56,    -1,    58,    59,    60,    -1,    62,    -1,
      -1,    -1,    66,    67,    68,    69,     3,    -1,     5,     6,
       7,     8,     9,    10,    11,    -1,    -1,    81,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,
      94,    -1,    -1,    -1,    31,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,    46,
       5,     6,     7,     8,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    83,    84,    -1,    -1,
      -1,    46,    89,    90,    91,    92,    93,    94,    -1,    -1,
      -1,    -1,    -1,    -1,     3,    60,     5,     6,     7,     8,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    83,    -1,
      -1,    -1,    31,    32,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    46,     5,     6,
       7,     8,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    86,    -1,    46,
      89,    90,    91,    92,    93,    94,    -1,    -1,    -1,    -1,
      -1,    -1,     3,    60,     5,     6,     7,     8,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    82,    -1,    -1,    -1,    -1,
      31,    32,    89,    90,    91,    92,    93,    94,    -1,    -1,
      -1,    -1,    -1,    -1,     3,    46,     5,     6,     7,     8,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    -1,    -1,    -1,    -1,    86,    -1,    46,    89,    90,
      91,    92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,
       3,    60,     5,     6,     7,     8,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    86,    31,    32,
      89,    90,    91,    92,    93,    94,    -1,    -1,    -1,    -1,
      -1,    -1,     3,    46,     5,     6,     7,     8,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    32,    -1,    -1,    -1,    -1,    -1,    -1,    81,    82,
      -1,    -1,    -1,    -1,    -1,    46,    89,    90,    91,    92,
      93,    94,    -1,    -1,    -1,    -1,    -1,    -1,     3,    60,
       5,     6,     7,     8,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    82,    -1,    -1,    -1,    -1,    31,    32,    89,    90,
      91,    92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,
       3,    46,     5,     6,     7,     8,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    82,    -1,    -1,
      -1,    -1,    -1,    46,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    60,     5,     6,
       7,     8,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    82,
      -1,    -1,    -1,    -1,    31,    32,    89,    90,    91,    92,
      93,    94,    -1,    -1,    -1,    -1,    -1,    -1,     3,    46,
       5,     6,     7,     8,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    89,    90,    91,    92,    93,    94,    -1,    -1,
      -1,    -1,    -1,    -1,     3,    60,     5,     6,     7,     8,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,    -1,    -1,
      -1,    -1,    31,    32,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    -1,    -1,    -1,     3,    46,     5,     6,
       7,     8,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    81,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      89,    90,    91,    92,    93,    94,    -1,    -1,    -1,    -1,
      -1,    -1,     3,    60,     5,     6,     7,     8,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,    -1,    81,    -1,    -1,    -1,    -1,    12,
      31,    32,    89,    90,    91,    92,    93,    94,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,    -1,    -1,    -1,    -1,
      -1,    34,    35,    -1,    -1,    38,    39,    -1,    -1,    60,
      43,    -1,    45,    46,    47,    48,    -1,    -1,    -1,    52,
      53,    54,    55,    56,    -1,    58,    59,    -1,    61,    62,
      81,    64,    65,    66,    67,    68,    69,     4,    89,    90,
      91,    92,    93,    94,    -1,    12,    -1,    -1,    81,    82,
      -1,    -1,    85,    -1,    -1,    -1,    -1,    90,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    35,    -1,
      -1,    38,    39,    -1,    -1,    -1,    43,    -1,    45,    46,
      47,    48,    -1,    -1,    -1,    52,    53,    54,    55,    56,
      -1,    58,    59,    -1,    61,    62,    -1,    64,    65,    66,
      67,    68,    69,     0,     1,    -1,    -1,     4,    -1,    -1,
      -1,    -1,    -1,    -1,    81,    82,    -1,    -1,    85,    -1,
      -1,    -1,    -1,    90,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    35,    -1,
      -1,    38,    39,    -1,    -1,    -1,    43,    -1,    45,    46,
      47,    48,    -1,     4,    -1,    52,    53,    54,    55,    56,
      -1,    58,    59,    -1,    61,    62,    -1,    64,    65,    66,
      67,    68,    69,    -1,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    35,    -1,    -1,    38,    39,    -1,
       4,    -1,    43,    -1,    45,    46,    -1,    48,    -1,    -1,
      -1,    -1,    53,    54,    -1,    56,    -1,    58,    59,    -1,
      -1,    62,    -1,    -1,    -1,    66,    67,    68,    69,    -1,
      -1,    35,    -1,    -1,    38,    39,    -1,     1,    -1,    43,
       4,    45,    46,    84,    48,    -1,    -1,    -1,    -1,    53,
      54,    -1,    56,    -1,    58,    59,    -1,    -1,    62,    -1,
      -1,    -1,    66,    67,    68,    69,    -1,    -1,    -1,    -1,
      34,    35,    -1,    -1,    38,    39,    -1,    -1,    -1,    43,
      84,    45,    46,    47,    48,    -1,    -1,    -1,    52,    53,
      54,    55,    56,    -1,    58,    59,    -1,    61,    62,    -1,
      64,    65,    66,    67,    68,    69,    -1,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,     3,     4,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    35,
      -1,    -1,    38,    39,    -1,    -1,     4,    43,    -1,    45,
      46,    47,    48,    -1,    -1,    -1,    52,    53,    54,    55,
      56,    -1,    58,    59,    -1,    61,    62,    -1,    64,    65,
      66,    67,    68,    69,    -1,    -1,    34,    35,    -1,    -1,
      38,    39,    -1,    -1,    -1,    43,    82,    45,    46,    47,
      48,    -1,    -1,    -1,    52,    53,    54,    55,    56,    -1,
      58,    59,    -1,    61,    62,     4,    64,    65,    66,    67,
      68,    69,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    83,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    34,    35,    -1,    -1,    38,
      39,    -1,    -1,     4,    43,    -1,    45,    46,    47,    48,
      -1,    12,    -1,    52,    53,    54,    55,    56,    -1,    58,
      59,    -1,    61,    62,    -1,    64,    65,    66,    67,    68,
      69,    -1,    -1,    34,    35,    -1,    -1,    38,    39,     4,
      -1,    -1,    43,    82,    45,    46,    47,    48,    -1,    -1,
      -1,    52,    53,    54,    55,    56,    -1,    58,    59,    -1,
      61,    62,    -1,    64,    65,    66,    67,    68,    69,    34,
      35,    -1,    -1,    38,    39,     4,    -1,    -1,    43,    -1,
      45,    46,    47,    48,    -1,    -1,    -1,    52,    53,    54,
      55,    56,    -1,    58,    59,    -1,    61,    62,    -1,    64,
      65,    66,    67,    68,    69,    -1,    35,    -1,    -1,    38,
      39,    -1,    -1,    -1,    43,    -1,    45,    46,    -1,    48,
      -1,    -1,    -1,    -1,    53,    54,    -1,    56,    -1,    58,
      59,    -1,    -1,    62,    -1,    -1,    -1,    66,    67,    68,
      69
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     4,    34,    35,    38,    39,    43,    45,    46,
      47,    48,    52,    53,    54,    55,    56,    58,    59,    61,
      62,    64,    65,    66,    67,    68,    69,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,   130,   131,   134,
     135,   136,   137,   143,   144,   148,   149,   160,   172,   173,
     174,   176,   177,   178,   179,   180,   181,     3,    81,    90,
     104,   108,   132,   133,   150,   151,   152,   131,   131,    83,
     108,   109,   160,    83,   109,   131,   131,     0,   173,    81,
       5,     6,     7,     8,     9,    10,    11,    31,    32,    46,
      60,    81,    89,    90,    91,    92,    93,    94,   106,   107,
     108,   110,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   129,   150,   148,   152,
     153,    88,   104,    83,   103,   130,   131,   165,   175,    81,
      85,   151,   135,   138,   139,   140,   148,    83,   145,   146,
      83,   108,   156,    81,    81,    81,   112,   112,    81,    81,
     112,    46,   112,   125,   126,   128,   140,   157,     9,    31,
      32,    33,    81,    85,    87,   114,    90,    95,    96,    91,
      92,    21,    22,    27,    28,    97,    98,    25,    26,    89,
      99,   100,    29,    30,   101,    82,   148,   152,   133,   150,
      36,    37,    40,    41,    42,    49,    50,    51,    57,    63,
      70,    84,   104,   108,   109,   128,   130,   160,   163,   164,
     165,   166,   167,   168,   169,   170,   171,    83,   126,   161,
     130,   165,    12,    82,   131,   154,   155,   156,    86,   126,
     140,    84,   139,   102,   141,   142,   150,   140,   138,    84,
      88,   108,   147,   145,    82,    88,   112,   112,    46,    83,
     157,    81,   128,    13,    14,    15,    16,    17,    18,    19,
      20,    23,    24,   103,   127,    82,    88,    81,    85,   152,
     158,   159,    82,   109,    82,   111,   126,   128,   109,   114,
     114,   114,   115,   115,   116,   116,   117,   117,   117,   117,
     118,   118,   119,   120,   121,   122,   123,   128,   104,   129,
     104,   102,   163,    81,   109,    81,   104,   128,    81,    81,
     102,   104,    84,   167,   161,   162,    81,   150,   152,   158,
      82,    88,    82,    86,   129,    88,   104,   102,    84,    84,
     147,   103,    84,    88,   108,    82,    82,   166,    82,   126,
     126,    82,   154,   158,    86,   126,   159,    81,    85,   114,
      82,    88,    86,   102,   102,   163,    70,   104,   128,   104,
     128,   104,   128,   128,   163,    84,    88,   155,   142,   129,
     129,    84,    84,    82,    82,    86,    82,   154,    86,   126,
     126,   128,   163,    81,   104,   128,   104,    82,    82,    82,
      84,   161,    82,    82,    86,   128,    82,   128,   104,   104,
     128,   163,   163,   163,    82,   163,    82,    82,   128,    82,
     128,   104,    44,   104,   163,   163,    82,   163,    82,    82,
     128,   163,   163,   163,   163,    82,   163
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   105,   106,   106,   106,   106,   106,   106,   106,   106,
     107,   107,   108,   109,   109,   110,   110,   110,   110,   110,
     110,   110,   110,   111,   111,   112,   112,   112,   112,   112,
     112,   112,   112,   113,   113,   113,   113,   113,   113,   114,
     114,   115,   115,   115,   115,   116,   116,   116,   117,   117,
     117,   118,   118,   118,   118,   118,   119,   119,   119,   120,
     120,   121,   121,   122,   122,   123,   123,   124,   124,   125,
     125,   126,   126,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   128,   128,   128,   129,   130,   130,
     131,   131,   131,   131,   131,   131,   131,   131,   132,   132,
     133,   133,   134,   134,   134,   134,   134,   134,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   136,   136,   136,   137,   137,   138,   138,   139,   140,
     140,   140,   140,   141,   141,   142,   142,   142,   142,   143,
     143,   143,   143,   143,   144,   146,   145,   145,   147,   147,
     148,   148,   148,   148,   149,   150,   150,   151,   151,   151,
     151,   151,   151,   151,   152,   152,   152,   152,   153,   153,
     154,   154,   155,   155,   155,   155,   156,   156,   157,   157,
     158,   158,   158,   159,   159,   159,   159,   159,   159,   159,
     159,   159,   160,   161,   161,   161,   162,   162,   163,   163,
     163,   163,   163,   163,   164,   164,   164,   165,   165,   166,
     166,   167,   167,   168,   168,   169,   169,   169,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   171,   171,
     171,   171,   171,   172,   172,   173,   173,   173,   174,   174,
     175,   175,   176,   177,   178,   179,   180,   180,   180,   180,
     180,   180,   180,   180,   181,   181,   181,   181
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     3,     6,
       1,     2,     1,     1,     1,     1,     4,     4,     3,     3,
       3,     2,     2,     1,     3,     1,     2,     2,     2,     4,
       4,     2,     4,     1,     1,     1,     1,     1,     1,     1,
       4,     1,     3,     3,     3,     1,     3,     3,     1,     3,
       3,     1,     3,     3,     3,     3,     1,     3,     3,     1,
       3,     1,     3,     1,     3,     1,     3,     1,     3,     1,
       5,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     3,     2,
       2,     1,     2,     1,     2,     1,     2,     1,     1,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     5,     4,     2,     1,     1,     1,     2,     3,     2,
       1,     2,     1,     1,     3,     0,     1,     2,     3,     5,
       4,     6,     5,     2,     1,     0,     2,     3,     1,     3,
       1,     1,     1,     1,     1,     2,     1,     1,     3,     4,
       3,     4,     4,     3,     2,     1,     3,     2,     1,     2,
       1,     3,     2,     2,     1,     1,     1,     3,     1,     2,
       1,     1,     2,     3,     2,     3,     3,     4,     2,     3,
       3,     4,     1,     1,     3,     4,     1,     3,     1,     1,
       1,     1,     1,     1,     3,     4,     3,     2,     3,     1,
       2,     1,     1,     1,     2,     5,     7,     5,     5,     7,
       6,     7,     7,     8,     7,     8,     8,     9,     3,     2,
       2,     2,     3,     1,     2,     1,     1,     1,     4,     3,
       1,     2,     1,     1,     4,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (scanner, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, GISourceScanner* scanner)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (scanner);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, GISourceScanner* scanner)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, scanner);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, GISourceScanner* scanner)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              , scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, GISourceScanner* scanner)
{
  YYUSE (yyvaluep);
  YYUSE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (GISourceScanner* scanner)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 327 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = g_hash_table_lookup (scanner->const_table, (yyvsp[0].str));
		if ((yyval.symbol) == NULL) {
			(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
		} else {
			(yyval.symbol) = gi_source_symbol_ref ((yyval.symbol));
		}
	  }
#line 2365 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 336 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		char *rest;
		guint64 value;
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		if (g_str_has_prefix (yytext, "0x") && strlen (yytext) > 2) {
			value = g_ascii_strtoull (yytext + 2, &rest, 16);
		} else if (g_str_has_prefix (yytext, "0") && strlen (yytext) > 1) {
			value = g_ascii_strtoull (yytext + 1, &rest, 8);
		} else {
			value = g_ascii_strtoull (yytext, &rest, 10);
		}
		(yyval.symbol)->const_int = value;
		(yyval.symbol)->const_int_is_unsigned = (rest && (rest[0] == 'U'));
	  }
#line 2385 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 352 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_boolean_set = TRUE;
		(yyval.symbol)->const_boolean = g_ascii_strcasecmp (yytext, "true") == 0 ? TRUE : FALSE;
	  }
#line 2395 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 358 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = g_utf8_get_char(yytext + 1);
	  }
#line 2405 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 364 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_double_set = TRUE;
		(yyval.symbol)->const_double = 0.0;
        sscanf (yytext, "%lf", &((yyval.symbol)->const_double));
	  }
#line 2416 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 372 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[-1].symbol);
	  }
#line 2424 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 376 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2432 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 384 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		yytext[strlen (yytext) - 1] = '\0';
		(yyval.symbol)->const_string = parse_c_string_literal (yytext + 1);
                if (!g_utf8_validate ((yyval.symbol)->const_string, -1, NULL))
                  {
#if 0
                    g_warning ("Ignoring non-UTF-8 constant string \"%s\"", yytext + 1);
#endif
                    g_free((yyval.symbol)->const_string);
                    (yyval.symbol)->const_string = NULL;
                  }

	  }
#line 2451 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 399 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		char *strings, *string2;
		(yyval.symbol) = (yyvsp[-1].symbol);
		yytext[strlen (yytext) - 1] = '\0';
		string2 = parse_c_string_literal (yytext + 1);
		strings = g_strconcat ((yyval.symbol)->const_string, string2, NULL);
		g_free ((yyval.symbol)->const_string);
		g_free (string2);
		(yyval.symbol)->const_string = strings;
	  }
#line 2466 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 413 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.str) = g_strdup (yytext);
	  }
#line 2474 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 426 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2482 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 430 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2490 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 434 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2498 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 438 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2506 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 442 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2514 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 446 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2522 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 450 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2530 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 463 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2538 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 467 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2546 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 471 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		switch ((yyvsp[-1].unary_operator)) {
		case UNARY_PLUS:
			(yyval.symbol) = (yyvsp[0].symbol);
			break;
		case UNARY_MINUS:
			(yyval.symbol) = gi_source_symbol_copy ((yyvsp[0].symbol));
			(yyval.symbol)->const_int = -(yyvsp[0].symbol)->const_int;
			break;
		case UNARY_BITWISE_COMPLEMENT:
			(yyval.symbol) = gi_source_symbol_copy ((yyvsp[0].symbol));
			(yyval.symbol)->const_int = ~(yyvsp[0].symbol)->const_int;
			break;
		case UNARY_LOGICAL_NEGATION:
			(yyval.symbol) = gi_source_symbol_copy ((yyvsp[0].symbol));
			(yyval.symbol)->const_int = !gi_source_symbol_get_const_boolean ((yyvsp[0].symbol));
			break;
		default:
			(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
			break;
		}
	  }
#line 2573 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 494 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[-1].symbol);
		if ((yyval.symbol)->const_int_set) {
			(yyval.symbol)->base_type = gi_source_basic_type_new ((yyval.symbol)->const_int_is_unsigned ? "guint64" : "gint64");
		}
	  }
#line 2584 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 501 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[-1].symbol);
		if ((yyval.symbol)->const_int_set) {
			(yyval.symbol)->base_type = gi_source_basic_type_new ("guint64");
		}
	  }
#line 2595 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 508 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2603 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 512 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		ctype_free ((yyvsp[-1].ctype));
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2612 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 520 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.unary_operator) = UNARY_ADDRESS_OF;
	  }
#line 2620 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 524 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.unary_operator) = UNARY_POINTER_INDIRECTION;
	  }
#line 2628 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 528 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.unary_operator) = UNARY_PLUS;
	  }
#line 2636 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 532 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.unary_operator) = UNARY_MINUS;
	  }
#line 2644 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 536 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.unary_operator) = UNARY_BITWISE_COMPLEMENT;
	  }
#line 2652 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 540 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.unary_operator) = UNARY_LOGICAL_NEGATION;
	  }
#line 2660 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 548 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[0].symbol);
		if ((yyval.symbol)->const_int_set || (yyval.symbol)->const_double_set || (yyval.symbol)->const_string != NULL) {
			(yyval.symbol)->base_type = (yyvsp[-2].ctype);
		} else {
			ctype_free ((yyvsp[-2].ctype));
		}
	  }
#line 2673 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 561 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int * (yyvsp[0].symbol)->const_int;
	  }
#line 2683 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 567 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		if ((yyvsp[0].symbol)->const_int != 0) {
			(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int / (yyvsp[0].symbol)->const_int;
		}
	  }
#line 2695 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 575 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		if ((yyvsp[0].symbol)->const_int != 0) {
			(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int % (yyvsp[0].symbol)->const_int;
		}
	  }
#line 2707 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 587 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int + (yyvsp[0].symbol)->const_int;
	  }
#line 2717 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 593 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int - (yyvsp[0].symbol)->const_int;
	  }
#line 2727 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 603 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int << (yyvsp[0].symbol)->const_int;

		/* assume this is a bitfield/flags declaration
		 * if a left shift operator is sued in an enum value
                 * This mimics the glib-mkenum behavior.
		 */
		is_bitfield = TRUE;
	  }
#line 2743 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 615 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int >> (yyvsp[0].symbol)->const_int;
	  }
#line 2753 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 625 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int < (yyvsp[0].symbol)->const_int;
	  }
#line 2763 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 631 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int > (yyvsp[0].symbol)->const_int;
	  }
#line 2773 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 637 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int <= (yyvsp[0].symbol)->const_int;
	  }
#line 2783 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 643 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int >= (yyvsp[0].symbol)->const_int;
	  }
#line 2793 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 653 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int == (yyvsp[0].symbol)->const_int;
	  }
#line 2803 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 659 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int != (yyvsp[0].symbol)->const_int;
	  }
#line 2813 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 669 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int & (yyvsp[0].symbol)->const_int;
	  }
#line 2823 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 62:
#line 679 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int ^ (yyvsp[0].symbol)->const_int;
	  }
#line 2833 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 689 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[-2].symbol)->const_int | (yyvsp[0].symbol)->const_int;
	  }
#line 2843 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 699 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int =
		  gi_source_symbol_get_const_boolean ((yyvsp[-2].symbol)) &&
		  gi_source_symbol_get_const_boolean ((yyvsp[0].symbol));
	  }
#line 2855 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 711 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_CONST, scanner->current_file, lineno);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int =
		  gi_source_symbol_get_const_boolean ((yyvsp[-2].symbol)) ||
		  gi_source_symbol_get_const_boolean ((yyvsp[0].symbol));
	  }
#line 2867 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 723 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_get_const_boolean ((yyvsp[-4].symbol)) ? (yyvsp[-2].symbol) : (yyvsp[0].symbol);
	  }
#line 2875 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 731 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2883 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 754 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 2891 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 767 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		GList *l;
		for (l = (yyvsp[-1].list); l != NULL; l = l->next) {
			GISourceSymbol *sym = l->data;
			gi_source_symbol_merge_type (sym, gi_source_type_copy ((yyvsp[-2].ctype)));
			if ((yyvsp[-2].ctype)->storage_class_specifier & STORAGE_CLASS_TYPEDEF) {
				sym->type = CSYMBOL_TYPE_TYPEDEF;
			} else if (sym->base_type->type == CTYPE_FUNCTION) {
				sym->type = CSYMBOL_TYPE_FUNCTION;
			} else {
				sym->type = CSYMBOL_TYPE_OBJECT;
			}
			gi_source_scanner_add_symbol (scanner, sym);
			gi_source_symbol_unref (sym);
		}
		ctype_free ((yyvsp[-2].ctype));
	  }
#line 2913 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 785 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		ctype_free ((yyvsp[-1].ctype));
	  }
#line 2921 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 792 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = (yyvsp[0].ctype);
		(yyval.ctype)->storage_class_specifier |= (yyvsp[-1].storage_class_specifier);
	  }
#line 2930 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 797 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_type_new (CTYPE_INVALID);
		(yyval.ctype)->storage_class_specifier |= (yyvsp[0].storage_class_specifier);
	  }
#line 2939 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 802 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = (yyvsp[-1].ctype);
		/* combine basic types like unsigned int and long long */
		if ((yyval.ctype)->type == CTYPE_BASIC_TYPE && (yyvsp[0].ctype)->type == CTYPE_BASIC_TYPE) {
			char *name = g_strdup_printf ("%s %s", (yyval.ctype)->name, (yyvsp[0].ctype)->name);
			g_free ((yyval.ctype)->name);
			(yyval.ctype)->name = name;
			ctype_free ((yyvsp[0].ctype));
		} else {
			set_or_merge_base_type ((yyvsp[-1].ctype), (yyvsp[0].ctype));
		}
	  }
#line 2956 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 816 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = (yyvsp[0].ctype);
		(yyval.ctype)->type_qualifier |= (yyvsp[-1].type_qualifier);
	  }
#line 2965 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 821 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_type_new (CTYPE_INVALID);
		(yyval.ctype)->type_qualifier |= (yyvsp[0].type_qualifier);
	  }
#line 2974 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 826 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = (yyvsp[0].ctype);
		(yyval.ctype)->function_specifier |= (yyvsp[-1].function_specifier);
	  }
#line 2983 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 831 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_type_new (CTYPE_INVALID);
		(yyval.ctype)->function_specifier |= (yyvsp[0].function_specifier);
	  }
#line 2992 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 839 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.list) = g_list_append (NULL, (yyvsp[0].symbol));
	  }
#line 3000 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 843 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.list) = g_list_append ((yyvsp[-2].list), (yyvsp[0].symbol));
	  }
#line 3008 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 855 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.storage_class_specifier) = STORAGE_CLASS_TYPEDEF;
	  }
#line 3016 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 103:
#line 859 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.storage_class_specifier) = STORAGE_CLASS_EXTERN;
	  }
#line 3024 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 104:
#line 863 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.storage_class_specifier) = STORAGE_CLASS_STATIC;
	  }
#line 3032 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 105:
#line 867 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.storage_class_specifier) = STORAGE_CLASS_AUTO;
	  }
#line 3040 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 106:
#line 871 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.storage_class_specifier) = STORAGE_CLASS_REGISTER;
	  }
#line 3048 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 107:
#line 875 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.storage_class_specifier) = STORAGE_CLASS_THREAD_LOCAL;
	  }
#line 3056 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 108:
#line 882 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_type_new (CTYPE_VOID);
	  }
#line 3064 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 109:
#line 886 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_basic_type_new ("char");
	  }
#line 3072 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 110:
#line 890 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_basic_type_new ("short");
	  }
#line 3080 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 111:
#line 894 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_basic_type_new ("int");
	  }
#line 3088 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 112:
#line 898 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_basic_type_new ("long");
	  }
#line 3096 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 113:
#line 902 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_basic_type_new ("float");
	  }
#line 3104 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 114:
#line 906 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_basic_type_new ("double");
	  }
#line 3112 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 115:
#line 910 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_basic_type_new ("signed");
	  }
#line 3120 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 116:
#line 914 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_basic_type_new ("unsigned");
	  }
#line 3128 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 117:
#line 918 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_basic_type_new ("bool");
	  }
#line 3136 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 120:
#line 924 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_typedef_new ((yyvsp[0].str));
		g_free ((yyvsp[0].str));
	  }
#line 3145 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 121:
#line 932 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		GISourceSymbol *sym;
		(yyval.ctype) = (yyvsp[-4].ctype);
		(yyval.ctype)->name = (yyvsp[-3].str);
		(yyval.ctype)->child_list = (yyvsp[-1].list);

		sym = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
		if ((yyval.ctype)->type == CTYPE_STRUCT) {
			sym->type = CSYMBOL_TYPE_STRUCT;
		} else if ((yyval.ctype)->type == CTYPE_UNION) {
			sym->type = CSYMBOL_TYPE_UNION;
		} else {
			g_assert_not_reached ();
		}
		sym->ident = g_strdup ((yyval.ctype)->name);
		sym->base_type = gi_source_type_copy ((yyval.ctype));
		gi_source_scanner_add_symbol (scanner, sym);
		gi_source_symbol_unref (sym);
	  }
#line 3169 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 122:
#line 952 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = (yyvsp[-3].ctype);
		(yyval.ctype)->child_list = (yyvsp[-1].list);
	  }
#line 3178 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 123:
#line 957 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = (yyvsp[-1].ctype);
		(yyval.ctype)->name = (yyvsp[0].str);
	  }
#line 3187 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 124:
#line 965 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
                scanner->private = FALSE;
		(yyval.ctype) = gi_source_struct_new (NULL);
	  }
#line 3196 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 125:
#line 970 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
                scanner->private = FALSE;
		(yyval.ctype) = gi_source_union_new (NULL);
	  }
#line 3205 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 127:
#line 979 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.list) = g_list_concat ((yyvsp[-1].list), (yyvsp[0].list));
	  }
#line 3213 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 128:
#line 986 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
	    GList *l;
	    (yyval.list) = NULL;
	    for (l = (yyvsp[-1].list); l != NULL; l = l->next)
	      {
		GISourceSymbol *sym = l->data;
		if ((yyvsp[-2].ctype)->storage_class_specifier & STORAGE_CLASS_TYPEDEF)
		    sym->type = CSYMBOL_TYPE_TYPEDEF;
		else
		    sym->type = CSYMBOL_TYPE_MEMBER;
		gi_source_symbol_merge_type (sym, gi_source_type_copy ((yyvsp[-2].ctype)));
                sym->private = scanner->private;
                (yyval.list) = g_list_append ((yyval.list), sym);
	      }
	    ctype_free ((yyvsp[-2].ctype));
	  }
#line 3234 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 129:
#line 1006 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = (yyvsp[-1].ctype);
		set_or_merge_base_type ((yyvsp[-1].ctype), (yyvsp[0].ctype));
	  }
#line 3243 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 131:
#line 1012 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = (yyvsp[0].ctype);
		(yyval.ctype)->type_qualifier |= (yyvsp[-1].type_qualifier);
	  }
#line 3252 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1017 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_type_new (CTYPE_INVALID);
		(yyval.ctype)->type_qualifier |= (yyvsp[0].type_qualifier);
	  }
#line 3261 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1025 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.list) = g_list_append (NULL, (yyvsp[0].symbol));
	  }
#line 3269 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1029 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.list) = g_list_append ((yyvsp[-2].list), (yyvsp[0].symbol));
	  }
#line 3277 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1036 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 3285 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1041 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
	  }
#line 3293 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1045 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[-2].symbol);
		if ((yyvsp[0].symbol)->const_int_set) {
		  (yyval.symbol)->const_int_set = TRUE;
		  (yyval.symbol)->const_int = (yyvsp[0].symbol)->const_int;
		}
	  }
#line 3305 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1056 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_enum_new ((yyvsp[-3].str));
		(yyval.ctype)->child_list = (yyvsp[-1].list);
		(yyval.ctype)->is_bitfield = is_bitfield || scanner->flags;
		last_enum_value = -1;
	  }
#line 3316 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1063 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_enum_new (NULL);
		(yyval.ctype)->child_list = (yyvsp[-1].list);
		(yyval.ctype)->is_bitfield = is_bitfield || scanner->flags;
		last_enum_value = -1;
	  }
#line 3327 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1070 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_enum_new ((yyvsp[-4].str));
		(yyval.ctype)->child_list = (yyvsp[-2].list);
		(yyval.ctype)->is_bitfield = is_bitfield || scanner->flags;
		last_enum_value = -1;
	  }
#line 3338 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1077 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_enum_new (NULL);
		(yyval.ctype)->child_list = (yyvsp[-2].list);
		(yyval.ctype)->is_bitfield = is_bitfield || scanner->flags;
		last_enum_value = -1;
	  }
#line 3349 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1084 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_enum_new ((yyvsp[0].str));
	  }
#line 3357 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1091 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
                scanner->flags = FALSE;
                scanner->private = FALSE;
          }
#line 3366 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1099 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		/* reset flag before the first enum value */
		is_bitfield = FALSE;
	  }
#line 3375 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1104 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
            (yyvsp[0].symbol)->private = scanner->private;
            (yyval.list) = g_list_append (NULL, (yyvsp[0].symbol));
	  }
#line 3384 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1109 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
            (yyvsp[0].symbol)->private = scanner->private;
            (yyval.list) = g_list_append ((yyvsp[-2].list), (yyvsp[0].symbol));
	  }
#line 3393 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1117 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_OBJECT, scanner->current_file, lineno);
		(yyval.symbol)->ident = (yyvsp[0].str);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = ++last_enum_value;
		g_hash_table_insert (scanner->const_table, g_strdup ((yyval.symbol)->ident), gi_source_symbol_ref ((yyval.symbol)));
	  }
#line 3405 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1125 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_OBJECT, scanner->current_file, lineno);
		(yyval.symbol)->ident = (yyvsp[-2].str);
		(yyval.symbol)->const_int_set = TRUE;
		(yyval.symbol)->const_int = (yyvsp[0].symbol)->const_int;
		last_enum_value = (yyval.symbol)->const_int;
		g_hash_table_insert (scanner->const_table, g_strdup ((yyval.symbol)->ident), gi_source_symbol_ref ((yyval.symbol)));
	  }
#line 3418 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1137 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.type_qualifier) = TYPE_QUALIFIER_CONST;
	  }
#line 3426 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1141 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.type_qualifier) = TYPE_QUALIFIER_RESTRICT;
	  }
#line 3434 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1145 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.type_qualifier) = TYPE_QUALIFIER_EXTENSION;
	  }
#line 3442 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1149 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.type_qualifier) = TYPE_QUALIFIER_VOLATILE;
	  }
#line 3450 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1156 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.function_specifier) = FUNCTION_INLINE;
	  }
#line 3458 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1163 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[0].symbol);
		gi_source_symbol_merge_type ((yyval.symbol), (yyvsp[-1].ctype));
	  }
#line 3467 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1172 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
		(yyval.symbol)->ident = (yyvsp[0].str);
	  }
#line 3476 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1177 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[-1].symbol);
	  }
#line 3484 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1181 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[-3].symbol);
		gi_source_symbol_merge_type ((yyval.symbol), gi_source_array_new ((yyvsp[-1].symbol)));
	  }
#line 3493 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1186 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[-2].symbol);
		gi_source_symbol_merge_type ((yyval.symbol), gi_source_array_new (NULL));
	  }
#line 3502 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1191 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		GISourceType *func = gi_source_function_new ();
		// ignore (void) parameter list
		if ((yyvsp[-1].list) != NULL && ((yyvsp[-1].list)->next != NULL || ((GISourceSymbol *) (yyvsp[-1].list)->data)->base_type->type != CTYPE_VOID)) {
			func->child_list = (yyvsp[-1].list);
		}
		(yyval.symbol) = (yyvsp[-3].symbol);
		gi_source_symbol_merge_type ((yyval.symbol), func);
	  }
#line 3516 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1201 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		GISourceType *func = gi_source_function_new ();
		func->child_list = (yyvsp[-1].list);
		(yyval.symbol) = (yyvsp[-3].symbol);
		gi_source_symbol_merge_type ((yyval.symbol), func);
	  }
#line 3527 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1208 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		GISourceType *func = gi_source_function_new ();
		(yyval.symbol) = (yyvsp[-2].symbol);
		gi_source_symbol_merge_type ((yyval.symbol), func);
	  }
#line 3537 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1217 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_pointer_new (NULL);
		(yyval.ctype)->type_qualifier = (yyvsp[0].type_qualifier);
	  }
#line 3546 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1222 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.ctype) = gi_source_pointer_new (NULL);
	  }
#line 3554 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1226 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		GISourceType **base = &((yyvsp[0].ctype)->base_type);

		while (*base != NULL) {
			base = &((*base)->base_type);
		}
		*base = gi_source_pointer_new (NULL);
		(*base)->type_qualifier = (yyvsp[-1].type_qualifier);
		(yyval.ctype) = (yyvsp[0].ctype);
	  }
#line 3569 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1237 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		GISourceType **base = &((yyvsp[0].ctype)->base_type);

		while (*base != NULL) {
			base = &((*base)->base_type);
		}
		*base = gi_source_pointer_new (NULL);
		(yyval.ctype) = (yyvsp[0].ctype);
	  }
#line 3583 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1251 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.type_qualifier) = (yyvsp[-1].type_qualifier) | (yyvsp[0].type_qualifier);
	  }
#line 3591 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1258 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.list) = g_list_append (NULL, (yyvsp[0].symbol));
	  }
#line 3599 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1262 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.list) = g_list_append ((yyvsp[-2].list), (yyvsp[0].symbol));
	  }
#line 3607 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1269 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[0].symbol);
		gi_source_symbol_merge_type ((yyval.symbol), (yyvsp[-1].ctype));
	  }
#line 3616 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1274 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[0].symbol);
		gi_source_symbol_merge_type ((yyval.symbol), (yyvsp[-1].ctype));
	  }
#line 3625 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1279 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
		(yyval.symbol)->base_type = (yyvsp[0].ctype);
	  }
#line 3634 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1284 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_ELLIPSIS, scanner->current_file, lineno);
	  }
#line 3642 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1291 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		GISourceSymbol *sym = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
		sym->ident = (yyvsp[0].str);
		(yyval.list) = g_list_append (NULL, sym);
	  }
#line 3652 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1297 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		GISourceSymbol *sym = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
		sym->ident = (yyvsp[0].str);
		(yyval.list) = g_list_append ((yyvsp[-2].list), sym);
	  }
#line 3662 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1311 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
		gi_source_symbol_merge_type ((yyval.symbol), (yyvsp[0].ctype));
	  }
#line 3671 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1317 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[0].symbol);
		gi_source_symbol_merge_type ((yyval.symbol), (yyvsp[-1].ctype));
	  }
#line 3680 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1325 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[-1].symbol);
	  }
#line 3688 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1329 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
		gi_source_symbol_merge_type ((yyval.symbol), gi_source_array_new (NULL));
	  }
#line 3697 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1334 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
		gi_source_symbol_merge_type ((yyval.symbol), gi_source_array_new ((yyvsp[-1].symbol)));
	  }
#line 3706 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1339 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[-2].symbol);
		gi_source_symbol_merge_type ((yyval.symbol), gi_source_array_new (NULL));
	  }
#line 3715 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1344 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.symbol) = (yyvsp[-3].symbol);
		gi_source_symbol_merge_type ((yyval.symbol), gi_source_array_new ((yyvsp[-1].symbol)));
	  }
#line 3724 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 188:
#line 1349 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		GISourceType *func = gi_source_function_new ();
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
		gi_source_symbol_merge_type ((yyval.symbol), func);
	  }
#line 3734 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 189:
#line 1355 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		GISourceType *func = gi_source_function_new ();
		// ignore (void) parameter list
		if ((yyvsp[-1].list) != NULL && ((yyvsp[-1].list)->next != NULL || ((GISourceSymbol *) (yyvsp[-1].list)->data)->base_type->type != CTYPE_VOID)) {
			func->child_list = (yyvsp[-1].list);
		}
		(yyval.symbol) = gi_source_symbol_new (CSYMBOL_TYPE_INVALID, scanner->current_file, lineno);
		gi_source_symbol_merge_type ((yyval.symbol), func);
	  }
#line 3748 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 190:
#line 1365 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		GISourceType *func = gi_source_function_new ();
		(yyval.symbol) = (yyvsp[-2].symbol);
		gi_source_symbol_merge_type ((yyval.symbol), func);
	  }
#line 3758 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 191:
#line 1371 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		GISourceType *func = gi_source_function_new ();
		// ignore (void) parameter list
		if ((yyvsp[-1].list) != NULL && ((yyvsp[-1].list)->next != NULL || ((GISourceSymbol *) (yyvsp[-1].list)->data)->base_type->type != CTYPE_VOID)) {
			func->child_list = (yyvsp[-1].list);
		}
		(yyval.symbol) = (yyvsp[-3].symbol);
		gi_source_symbol_merge_type ((yyval.symbol), func);
	  }
#line 3772 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 192:
#line 1384 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.str) = g_strdup (yytext);
	  }
#line 3780 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 242:
#line 1491 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.str) = g_strdup (yytext + strlen ("#define "));
	  }
#line 3788 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 243:
#line 1498 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		(yyval.str) = g_strdup (yytext + strlen ("#define "));
	  }
#line 3796 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 245:
#line 1509 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		if ((yyvsp[0].symbol)->const_int_set || (yyvsp[0].symbol)->const_boolean_set || (yyvsp[0].symbol)->const_double_set || (yyvsp[0].symbol)->const_string != NULL) {
			GISourceSymbol *macro = gi_source_symbol_copy ((yyvsp[0].symbol));
			g_free (macro->ident);
			macro->ident = (yyvsp[-1].str);
			gi_source_scanner_add_symbol (scanner, macro);
			gi_source_symbol_unref (macro);
			gi_source_symbol_unref ((yyvsp[0].symbol));
		} else {
			g_free ((yyvsp[-1].str));
			gi_source_symbol_unref ((yyvsp[0].symbol));
		}
	  }
#line 3814 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 246:
#line 1526 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		push_conditional (scanner, FOR_GI_SCANNER);
		update_skipping (scanner);
	  }
#line 3823 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 247:
#line 1531 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		push_conditional (scanner, NOT_GI_SCANNER);
		update_skipping (scanner);
	  }
#line 3832 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 248:
#line 1536 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
	 	warn_if_cond_has_gi_scanner (scanner, yytext);
		push_conditional (scanner, IRRELEVANT);
	  }
#line 3841 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 249:
#line 1541 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		warn_if_cond_has_gi_scanner (scanner, yytext);
		push_conditional (scanner, IRRELEVANT);
	  }
#line 3850 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 250:
#line 1546 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		warn_if_cond_has_gi_scanner (scanner, yytext);
		push_conditional (scanner, IRRELEVANT);
	  }
#line 3859 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 251:
#line 1551 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		warn_if_cond_has_gi_scanner (scanner, yytext);
		pop_conditional (scanner);
		push_conditional (scanner, IRRELEVANT);
		update_skipping (scanner);
	  }
#line 3870 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 252:
#line 1558 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		toggle_conditional (scanner);
		update_skipping (scanner);
	  }
#line 3879 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;

  case 253:
#line 1563 "giscanner/scannerparser.y" /* yacc.c:1646  */
    {
		pop_conditional (scanner);
		update_skipping (scanner);
	  }
#line 3888 "giscanner/scannerparser.c" /* yacc.c:1646  */
    break;


#line 3892 "giscanner/scannerparser.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (scanner, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (scanner, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, scanner);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1576 "giscanner/scannerparser.y" /* yacc.c:1906  */

static void
yyerror (GISourceScanner *scanner, const char *s)
{
  /* ignore errors while doing a macro scan as not all object macros
   * have valid expressions */
  if (!scanner->macro_scan)
    {
      fprintf(stderr, "%s:%d: %s in '%s' at '%s'\n",
	      g_file_get_parse_name (scanner->current_file), lineno, s, linebuf, yytext);
    }
}

static int
eat_hspace (FILE * f)
{
  int c;
  do
    {
      c = fgetc (f);
    }
  while (c == ' ' || c == '\t');
  return c;
}

static int
pass_line (FILE * f, int c,
           FILE *out)
{
  while (c != EOF && c != '\n')
    {
      if (out)
        fputc (c, out);
      c = fgetc (f);
    }
  if (c == '\n')
    {
      if (out)
        fputc (c, out);
      c = fgetc (f);
      if (c == ' ' || c == '\t')
        {
          c = eat_hspace (f);
        }
    }
  return c;
}

static int
eat_line (FILE * f, int c)
{
  return pass_line (f, c, NULL);
}

static int
read_identifier (FILE * f, int c, char **identifier)
{
  GString *id = g_string_new ("");
  while (g_ascii_isalnum (c) || c == '_')
    {
      g_string_append_c (id, c);
      c = fgetc (f);
    }
  *identifier = g_string_free (id, FALSE);
  return c;
}

void
gi_source_scanner_parse_macros (GISourceScanner *scanner, GList *filenames)
{
  GError *error = NULL;
  char *tmp_name = NULL;
  FILE *fmacros =
    fdopen (g_file_open_tmp ("gen-introspect-XXXXXX.h", &tmp_name, &error),
            "w+");
  GList *l;

  for (l = filenames; l != NULL; l = l->next)
    {
      FILE *f = fopen (l->data, "r");
      int line = 1;

      GString *define_line;
      char *str;
      gboolean error_line = FALSE;
      gboolean end_of_word;
      int c = eat_hspace (f);
      while (c != EOF)
        {
          if (c != '#')
            {
              /* ignore line */
              c = eat_line (f, c);
              line++;
              continue;
            }

          /* print current location */
          str = g_strescape (l->data, "");
          fprintf (fmacros, "# %d \"%s\"\n", line, str);
          g_free (str);

          c = eat_hspace (f);
          c = read_identifier (f, c, &str);
          end_of_word = (c == ' ' || c == '\t' || c == '\n' || c == EOF);
          if (end_of_word &&
              (g_str_equal (str, "if") ||
               g_str_equal (str, "endif") ||
               g_str_equal (str, "ifndef") ||
               g_str_equal (str, "ifdef") ||
               g_str_equal (str, "else") ||
               g_str_equal (str, "elif")))
            {
              fprintf (fmacros, "#%s ", str);
              g_free (str);
              c = pass_line (f, c, fmacros);
              line++;
              continue;
            }
          else if (strcmp (str, "define") != 0 || !end_of_word)
            {
              g_free (str);
              /* ignore line */
              c = eat_line (f, c);
              line++;
              continue;
            }
          g_free (str);
          c = eat_hspace (f);
          c = read_identifier (f, c, &str);
          if (strlen (str) == 0 || (c != ' ' && c != '\t' && c != '('))
            {
              g_free (str);
              /* ignore line */
              c = eat_line (f, c);
              line++;
              continue;
            }
          define_line = g_string_new ("#define ");
          g_string_append (define_line, str);
          g_free (str);
          if (c == '(')
            {
              while (c != ')')
                {
                  g_string_append_c (define_line, c);
                  c = fgetc (f);
                  if (c == EOF || c == '\n')
                    {
                      error_line = TRUE;
                      break;
                    }
                }
              if (error_line)
                {
                  g_string_free (define_line, TRUE);
                  /* ignore line */
                  c = eat_line (f, c);
                  line++;
                  continue;
                }

              g_assert (c == ')');
              g_string_append_c (define_line, c);
              c = fgetc (f);

              /* found function-like macro */
              fprintf (fmacros, "%s\n", define_line->str);

              g_string_free (define_line, TRUE);
              /* ignore rest of line */
              c = eat_line (f, c);
              line++;
              continue;
            }
          if (c != ' ' && c != '\t')
            {
              g_string_free (define_line, TRUE);
              /* ignore line */
              c = eat_line (f, c);
              line++;
              continue;
            }
          while (c != EOF && c != '\n')
            {
              g_string_append_c (define_line, c);
              c = fgetc (f);
              if (c == '\\')
                {
                  c = fgetc (f);
                  if (c == '\n')
                    {
                      /* fold lines when seeing backslash new-line sequence */
                      c = fgetc (f);
                    }
                  else
                    {
                      g_string_append_c (define_line, '\\');
                    }
                }
            }

          /* found object-like macro */
          fprintf (fmacros, "%s\n", define_line->str);

          c = eat_line (f, c);
          line++;
        }

      fclose (f);
    }

  rewind (fmacros);
  gi_source_scanner_parse_file (scanner, fmacros);
  fclose (fmacros);
  g_unlink (tmp_name);
}

gboolean
gi_source_scanner_parse_file (GISourceScanner *scanner, FILE *file)
{
  g_return_val_if_fail (file != NULL, FALSE);

  lineno = 1;
  yyin = file;
  yyparse (scanner);
  yyin = NULL;

  return TRUE;
}

gboolean
gi_source_scanner_lex_filename (GISourceScanner *scanner, const gchar *filename)
{
  lineno = 1;
  yyin = fopen (filename, "r");

  while (yylex (scanner) != YYEOF)
    ;

  fclose (yyin);

  return TRUE;
}
