/*
CSERIES.H
*/

#ifndef __CSERIES_H
#define __CSERIES_H
#pragma once

#define _USE_MATH_DEFINES

#include <StdDef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

/* ---------- constants */

#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

enum
{
	MILLISECONDS_PER_SECOND = 1000,
	SECONDS_PER_MINUTE = 60,
	MINUTES_PER_HOUR = 60,
	HOURS_PER_DAY = 24,
	VBLANKS_PER_SECOND = 60,
	TICKS_PER_SECOND = 30,
	ACTUAL_TICKS_PER_SECOND = 30,
	TICKS_PER_MINUTE = (SECONDS_PER_MINUTE*TICKS_PER_SECOND),
	TICKS_PER_HOUR = (MINUTES_PER_HOUR*TICKS_PER_MINUTE),
	TICKS_PER_DAY = (HOURS_PER_DAY*TICKS_PER_HOUR)
};

enum
{
	UNSIGNED_LONG_MAX = 4294967295,
	LONG_MAX = 2147483647L,
	LONG_MIN = (-2147483648L),
	LONG_BITS = 32,
	LONG_BITS_BITS = 5,

	UNSIGNED_SHORT_MAX = 65535,
	SHORT_MAX = 32767,
	SHORT_MIN = (-32768),
	SHORT_BITS = 16,
	SHORT_BITS_BITS = 4,

	UNSIGNED_CHAR_MAX = 255,
	CHAR_MAX = 127,
	CHAR_MIN = (-128),
	CHAR_BITS = 8,
	CHAR_BITS_BITS = 3
};

enum
{
	_x = 0,
	_y,
	_z,
	NUMBER_OF_RECTANGLE2D_COMPONENTS = 4,
	NUMBER_OF_RECTANGLE3D_COMPONENTS = 6,
	NUMBER_OF_VERTICES_PER_LINE = 2,
	NUMBER_OF_VERTICES_PER_TRIANGLE = 3,
	NUMBER_OF_VERTICES_PER_QUADRALATERAL = 4,
	NUMBER_OF_VERTICES_PER_QUADRILATERAL = 4,
	NUMBER_OF_VERTICES_PER_HEXAGON = 6,
	NUMBER_OF_VERTICES_PER_PYRAMID = 5,
	NUMBER_OF_VERTICES_PER_CUBE = 8,
	NUMBER_OF_TRIANGLES_PER_QUADRILATERAL = 2,
	NUMBER_OF_EDGES_PER_TRIANGLE = 3,
	NUMBER_OF_EDGES_PER_QUADRALATERAL = 4,
	NUMBER_OF_EDGES_PER_HEXAGON = 6,
	NUMBER_OF_FACES_PER_CUBE = 6,
	_rectangle_top_left = 0,
	_rectangle_bottom_left = 1,
	_rectangle_top_right = 2,
	_rectangle_bottom_right = 3,
	NUMBER_OF_POINTS_PER_RECTANGLE = 4,
};


#define NONE -1

/* ---------- macros */

#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)

/* on non-matching builds give actual source line info for assertions */
#ifdef NON_MATCHING
	#define MATCH_FILE(file) __FILE__
	#define MATCH_LINE(line) __LINE__
#else
	#define MATCH_FILE(file) file
	#define MATCH_LINE(line) line
#endif

#define match_halt(file, line) do { display_assert(NULL, MATCH_FILE(file), MATCH_LINE(line), TRUE); halt_and_catch_fire(); } while (FALSE);
#define match_vhalt(file, line, string) do { display_assert(string, MATCH_FILE(file), MATCH_LINE(line), TRUE); halt_and_catch_fire(); } while (FALSE);
#ifdef HALO_RELEASE
/* release builds of the native ports (configure.py --release): like the
retail game, a failed assertion does not stop the game, but it is noted in
debug.txt (release_assert_failed, cseries.c). The expressions are evaluated
as in the checked form, since a few do work the game relies on (heap_insert
in path_obstacle_avoidance.c, hs_parse_variable in hs_compile.c). A message
is written only when it is a string literal: one built from the failing
data (csprintf of a bad index's name) was never evaluated in a release
build, and could fault where the game would carry on. Each is a statement
ending in a brace, like the checked form, which some uses rely on (no
semicolon). */
#define RELEASE_ASSERT_MESSAGE(string) \
	(__builtin_constant_p(string) ? (char const *)(string) : "<message not formatted in release builds>")
#define match_assert(file, line, expr) if (!(expr)) { release_assert_failed(#expr, MATCH_FILE(file), MATCH_LINE(line), TRUE); }
#define match_vassert(file, line, expr, string) if (!(expr)) { release_assert_failed(RELEASE_ASSERT_MESSAGE(string), MATCH_FILE(file), MATCH_LINE(line), TRUE); }
#define match_warn(file, line, expr) if (!(expr)) { release_assert_failed(#expr, MATCH_FILE(file), MATCH_LINE(line), FALSE); }
#define match_vwarn(file, line, expr, string) if (!(expr)) { release_assert_failed(RELEASE_ASSERT_MESSAGE(string), MATCH_FILE(file), MATCH_LINE(line), FALSE); }
#else
#define match_assert(file, line, expr) if (!(expr)) { display_assert(#expr, MATCH_FILE(file), MATCH_LINE(line), TRUE); system_exit(-1); }
#define match_vassert(file, line, expr, string) if (!(expr)) { display_assert(string, MATCH_FILE(file), MATCH_LINE(line), TRUE); system_exit(-1); }
#define match_warn(file, line, expr) if (!(expr)) { display_assert(#expr, MATCH_FILE(file), MATCH_LINE(line), FALSE); }
#define match_vwarn(file, line, expr, string) if (!(expr)) { display_assert(string, MATCH_FILE(file), MATCH_LINE(line), FALSE); }
#endif
#define match_dassert(file, line, expr, diagnostic) do { match_vassert(file, line, expr, diagnostic); } while (FALSE)
#define match_dwarn(file, line, expr, diagnostic) do { match_vwarn(file, line, expr, diagnostic); } while (FALSE)
#define match_dhalt(file, line, diagnostic) do { match_vhalt(file, line, diagnostic); } while (FALSE)

#define halt() match_halt(__FILE__, __LINE__)
#define vhalt(string) match_vhalt(__FILE__, __LINE__, string)
#define dhalt(diagnostic) match_dhalt(__FILE__, __LINE__, diagnostic)
#define assert(expr) match_assert(__FILE__, __LINE__, expr)
#define dassert(expr, diagnostic) match_dassert(__FILE__, __LINE__, expr, diagnostic)
/* VC7 has no variadic macros: format vassert messages with csprintf(temporary, ...). */
#define vassert(expr, string) match_vassert(__FILE__, __LINE__, expr, string)
#define warn(expr) match_warn(__FILE__, __LINE__, expr)
#define dwarn(expr, diagnostic) match_dwarn(__FILE__, __LINE__, expr, diagnostic)
#define vwarn(expr, string) match_vwarn(__FILE__, __LINE__, expr, string)

#define ABS(x) ((x>=0) ? (x) : -(x))

#define MIN(a,b) ((a)>(b)?(b):(a))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define FLOOR(n,floor) ((n)<(floor)?(floor):(n))
#define CEILING(n,ceiling) ((n)>(ceiling)?(ceiling):(n))
#define PIN(n,floor,ceiling) ((n)<(floor) ? (floor) : CEILING((n),(ceiling)))

#define FLAG(b) (1<<(b))
#define TEST_FLAG(flags, bit) (((flags)&(unsigned)FLAG(bit))!=0)
#define SET_FLAG(f, b, v) ((v) ? ((f)|=(unsigned)FLAG(b)) : ((f)&=(unsigned)~FLAG(b)))

#define BIT_VECTOR_SIZE_IN_LONGS(bit_count) (((bit_count) + (LONG_BITS - 1)) >> LONG_BITS_BITS)
#define BIT_VECTOR_SIZE_IN_BYTES(bit_count) (sizeof(long) * BIT_VECTOR_SIZE_IN_LONGS(bit_count))
#define BIT_VECTOR_TEST_FLAG(bit_vector, bit) (TEST_FLAG((bit_vector)[(bit) >> LONG_BITS_BITS], ((bit) & (LONG_BITS - 1))))
#define BIT_VECTOR_SET_FLAG(bit_vector, bit, enable) (SET_FLAG((bit_vector)[(bit) >> LONG_BITS_BITS], ((bit) & (LONG_BITS - 1)), enable))

#define VALID_FLAGS(flags, bits) (!((flags)>>bits))

#define SIZEOF_BITS(value) (CHAR_BITS*sizeof(value))
#define NUMBEROF(array) (sizeof(array) / sizeof(array[0]))

#define VALID_INDEX(index, count) (index>=0 && index<count)

#define DATUM_INDEX_NEW(absolute_index, salt) ((absolute_index) | ((salt)<<SHORT_BITS))
/* port: cast to long, 4 bytes as on the Xbox.
UNSIGNED_LONG_MAX does not fit in an int,
so clang and gcc make all constants of the limits enum long long.
Without the cast, the index shifts the arguments after it in a printf-style call. */
#define DATUM_INDEX_TO_ABSOLUTE_INDEX(datum_index) ((long)((datum_index)&UNSIGNED_SHORT_MAX))
#define DATUM_INDEX_TO_IDENTIFIER(datum_index) ((datum_index)>>SHORT_BITS)

/* ---------- fixed math */

#define SHORT_FIXED_TO_LONG(f) ((f)>>CHAR_BITS)

/* ---------- types */

typedef unsigned char byte;
typedef unsigned short word;
typedef float real;

typedef byte boolean;

typedef unsigned long tag;

/* ---------- prototypes/CSERIES.C */

void cseries_initialize(void);
void cseries_dispose(void);
tag string_to_tag(const char *s);
char *tag_to_string(tag t, char *s);
long strnlen(const char *string, long n);
char *strnupr(char *string, long n);
char *strnlwr(char *string, long n);
char *strupr(char *string);
char *strlwr(char *string);
char *csprintf(char *buffer, char *format, ...);
void display_assert(char *information, char *file, long line, boolean fatal);
#ifdef HALO_RELEASE
void release_assert_failed(char const *information, char const *file, long line, boolean fatal);
#endif
long csmemcmp(const void *p1, const void *p2, unsigned long size);
void *csmemmove(void *destination, const void *source, unsigned long size);
void *csmemset(void *buffer, long c, unsigned long size);
char *csstrcat(char *s1, const char *s2);
long csstrcmp(const char *s1, const char *s2);
char *csstrncat(char *s1, const char *s2, unsigned long size);
long csstrncmp(const char *s1, const char *s2, unsigned long size);
char *csstrncpy(char *s1, const char *s2, unsigned long size);
char *csstrtok(char *s1, const char *s2);
unsigned long csstrlen(const char *s1);
char *csstrcpy(char *destination, const char *source);
void *csmemcpy(void *destination, const void *source, unsigned long size);
long csstrcasecmp(const char *s1, const char *s2);
char *stristr(char const *haystack, char const *needle);
unsigned long string_hash(char const *string);

/* ---------- prototypes/CSERIES_WINDOWS.C */

void system_exit(long code);
#ifdef HALO_RELEASE
/* set by display_assert when a release build skips a fatal assertion */
extern __thread boolean display_assert_skipped;
#endif

/* ---------- prototypes/MAIN.C */

void halt_and_catch_fire(void);

/* ---------- prototypes/DEBUG_MEMORY.C */

void debug_memory_manager_initialize(
	void);
/*void debug_memory_manager_dispose(void);*/
void check_memory_status(
	struct memory_status *memory_status,
	const char *location);
void debug_check_memory(
	const char *file,
	long line);
void debug_dump_memory_for_file(
	const char *file);
void debug_dump_memory_by_file(
	void);
void *debug_malloc(
	unsigned int size,
	boolean clear,
	const char *file,
	long line);
void debug_free(
	void *pointer,
	const char *file,
	long line);
void *debug_realloc(
	void *pointer,
	unsigned int size,
	const char *file,
	long line);
void debug_dump_memory(
	void);

/* ---------- prototypes/STACK_WALK_WINDOWS.C */

void stack_walk_disregard_symbol_names(boolean disregard);

/* ---------- macros */

#ifndef BUILDING_CSERIES
#define memcmp csmemcmp
#define memmove csmemmove
#define memset csmemset
#define strcat csstrcat
#define strcmp csstrcmp
#define strncat csstrncat
#define strncmp csstrncmp
#define strncpy csstrncpy
#define strtok csstrtok
#define strlen csstrlen
#define strcpy csstrcpy
#define memcpy csmemcpy

#define match_malloc(file, line, size) debug_malloc(size, FALSE, MATCH_FILE(file), MATCH_LINE(line))
#define match_free(file, line, ptr) debug_free(ptr, MATCH_FILE(file), MATCH_LINE(line))
#define match_realloc(file, line, ptr, size) debug_realloc(ptr, size, MATCH_FILE(file), MATCH_LINE(line))

#define malloc(size) match_malloc(__FILE__, __LINE__, size)
#define free(ptr) match_free(__FILE__, __LINE__, ptr)
#define realloc(ptr, size) match_realloc(__FILE__, __LINE__, ptr, size)
#endif

/* ---------- globals */

extern char temporary[256];

extern const union real_argb_color *global_real_argb_white;
extern const union real_argb_color *global_real_argb_grey;
extern const union real_argb_color *global_real_argb_black;
extern const union real_argb_color *global_real_argb_red;
extern const union real_argb_color *global_real_argb_green;
extern const union real_argb_color *global_real_argb_blue;
extern const union real_argb_color *global_real_argb_cyan;
extern const union real_argb_color *global_real_argb_yellow;
extern const union real_argb_color *global_real_argb_magenta;
extern const union real_argb_color *global_real_argb_pink;
extern const union real_argb_color *global_real_argb_lightblue;
extern const union real_argb_color *global_real_argb_orange;
extern const union real_argb_color *global_real_argb_purple;
extern const union real_argb_color *global_real_argb_aqua;
extern const union real_argb_color *global_real_argb_darkgreen;
extern const union real_argb_color *global_real_argb_salmon;
extern const union real_argb_color *global_real_argb_violet;

extern const union real_rgb_color *global_real_rgb_white;
extern const union real_rgb_color *global_real_rgb_grey;
extern const union real_rgb_color *global_real_rgb_black;
extern const union real_rgb_color *global_real_rgb_red;
extern const union real_rgb_color *global_real_rgb_green;
extern const union real_rgb_color *global_real_rgb_blue;
extern const union real_rgb_color *global_real_rgb_cyan;
extern const union real_rgb_color *global_real_rgb_yellow;
extern const union real_rgb_color *global_real_rgb_magenta;
extern const union real_rgb_color *global_real_rgb_pink;
extern const union real_rgb_color *global_real_rgb_lightblue;
extern const union real_rgb_color *global_real_rgb_orange;
extern const union real_rgb_color *global_real_rgb_purple;
extern const union real_rgb_color *global_real_rgb_aqua;
extern const union real_rgb_color *global_real_rgb_darkgreen;
extern const union real_rgb_color *global_real_rgb_salmon;
extern const union real_rgb_color *global_real_rgb_violet;

/* ---------- public code */

__inline long fast_ftol(
	real value)
{
	long result;

	/* FISTP: round to nearest under the default control word */
	result = (long)__builtin_rint((double)value);

	return result;
}

#endif // __CSERIES_H
