#ifndef NASM_WARNINGS_H
#define NASM_WARNINGS_H

#ifndef WARN_SHR
# error "warnings.h should only be included from within error.h"
#endif

enum warn_index {
	WARN_IDX_NONE                    =   0, /* not suppressible */
	WARN_IDX_BND                     =   1, /* invalid BND prefixes */
	WARN_IDX_ENVIRONMENT             =   2, /* nonexistent environment variable */
	WARN_IDX_FLOAT_DENORM            =   3, /* floating point denormal */
	WARN_IDX_FLOAT_OVERFLOW          =   4, /* floating point overflow */
	WARN_IDX_FLOAT_TOOLONG           =   5, /* too many digits in floating-point number */
	WARN_IDX_FLOAT_UNDERFLOW         =   6, /* floating point underflow */
	WARN_IDX_HLE                     =   7, /* invalid HLE prefixes */
	WARN_IDX_LABEL_ORPHAN            =   8, /* labels alone on lines without trailing `:' */
	WARN_IDX_LABEL_REDEF             =   9, /* label redefined to an identical value */
	WARN_IDX_LABEL_REDEF_LATE        =  10, /* label (re)defined during code generation */
	WARN_IDX_LOCK                    =  11, /* LOCK prefix on unlockable instructions */
	WARN_IDX_MACRO_DEFAULTS          =  12, /* macros with more default than optional parameters */
	WARN_IDX_MACRO_PARAMS_MULTI      =  13, /* multi-line macro calls with wrong parameter count */
	WARN_IDX_MACRO_PARAMS_SINGLE     =  14, /* single-line macro calls with wrong parameter count */
	WARN_IDX_NEGATIVE_REP            =  15, /* regative %rep count */
	WARN_IDX_NUMBER_OVERFLOW         =  16, /* numeric constant does not fit */
	WARN_IDX_OBSOLETE_NOP            =  17, /* instruction obsolete and is a noop on the target CPU */
	WARN_IDX_OBSOLETE_REMOVED        =  18, /* instruction obsolete and removed on the target CPU */
	WARN_IDX_OBSOLETE_VALID          =  19, /* instruction obsolete but valid on the target CPU */
	WARN_IDX_PHASE                   =  20, /* phase error during stabilization */
	WARN_IDX_PRAGMA_BAD              =  21, /* malformed %pragma */
	WARN_IDX_PRAGMA_EMPTY            =  22, /* empty %pragma directive */
	WARN_IDX_PRAGMA_NA               =  23, /* %pragma not applicable to this compilation */
	WARN_IDX_PRAGMA_UNKNOWN          =  24, /* unknown %pragma facility or directive */
	WARN_IDX_PTR                     =  25, /* non-NASM keyword used in other assemblers */
	WARN_IDX_REGSIZE                 =  26, /* register size specification ignored */
	WARN_IDX_UNKNOWN_WARNING         =  27, /* unknown warning in -W/-w or warning directive */
	WARN_IDX_USER                    =  28, /* %warning directives */
	WARN_IDX_WARN_STACK_EMPTY        =  29, /* warning stack empty */
	WARN_IDX_ZEROING                 =  30, /* RESx in initialized section becomes zero */
	WARN_IDX_ZEXT_RELOC              =  31, /* relocation zero-extended to match output format */
	WARN_IDX_OTHER                   =  32, /* any warning not specifially mentioned above */
	WARN_IDX_ALL                     =  33  /* all possible warnings */
};

enum warn_const {
	WARN_NONE                        =   0 << WARN_SHR,
	WARN_BND                         =   1 << WARN_SHR,
	WARN_ENVIRONMENT                 =   2 << WARN_SHR,
	WARN_FLOAT_DENORM                =   3 << WARN_SHR,
	WARN_FLOAT_OVERFLOW              =   4 << WARN_SHR,
	WARN_FLOAT_TOOLONG               =   5 << WARN_SHR,
	WARN_FLOAT_UNDERFLOW             =   6 << WARN_SHR,
	WARN_HLE                         =   7 << WARN_SHR,
	WARN_LABEL_ORPHAN                =   8 << WARN_SHR,
	WARN_LABEL_REDEF                 =   9 << WARN_SHR,
	WARN_LABEL_REDEF_LATE            =  10 << WARN_SHR,
	WARN_LOCK                        =  11 << WARN_SHR,
	WARN_MACRO_DEFAULTS              =  12 << WARN_SHR,
	WARN_MACRO_PARAMS_MULTI          =  13 << WARN_SHR,
	WARN_MACRO_PARAMS_SINGLE         =  14 << WARN_SHR,
	WARN_NEGATIVE_REP                =  15 << WARN_SHR,
	WARN_NUMBER_OVERFLOW             =  16 << WARN_SHR,
	WARN_OBSOLETE_NOP                =  17 << WARN_SHR,
	WARN_OBSOLETE_REMOVED            =  18 << WARN_SHR,
	WARN_OBSOLETE_VALID              =  19 << WARN_SHR,
	WARN_PHASE                       =  20 << WARN_SHR,
	WARN_PRAGMA_BAD                  =  21 << WARN_SHR,
	WARN_PRAGMA_EMPTY                =  22 << WARN_SHR,
	WARN_PRAGMA_NA                   =  23 << WARN_SHR,
	WARN_PRAGMA_UNKNOWN              =  24 << WARN_SHR,
	WARN_PTR                         =  25 << WARN_SHR,
	WARN_REGSIZE                     =  26 << WARN_SHR,
	WARN_UNKNOWN_WARNING             =  27 << WARN_SHR,
	WARN_USER                        =  28 << WARN_SHR,
	WARN_WARN_STACK_EMPTY            =  29 << WARN_SHR,
	WARN_ZEROING                     =  30 << WARN_SHR,
	WARN_ZEXT_RELOC                  =  31 << WARN_SHR,
	WARN_OTHER                       =  32 << WARN_SHR
};

struct warning_alias {
	const char *name;
	enum warn_index warning;
};

#define NUM_WARNING_ALIAS 37
extern const char * const warning_name[34];
extern const char * const warning_help[34];
extern const struct warning_alias warning_alias[NUM_WARNING_ALIAS];
extern const uint8_t warning_default[33];
extern uint8_t warning_state[33];

#endif /* NASM_WARNINGS_H */
