/*
UNICODE.C

symbols in this file:
0018CCD0 0070:
	_umemchr (0000)
0018CD40 00a0:
	_umemcpy (0000)
0018CDE0 0070:
	_umemcmp (0000)
0018CE50 0070:
	_umemmove (0000)
0018CEC0 0070:
	_umemset (0000)
0018CF30 0080:
	_ustrcpy (0000)
0018CFB0 00b0:
	_ustrcat (0000)
0018D060 00b0:
	_ustrcmp (0000)
0018D110 0070:
	_ustrlen (0000)
0018D180 0080:
	_ustrnlen (0000)
0018D200 0070:
	_ustrchr (0000)
0018D270 00b0:
	_ustrcoll (0000)
0018D320 00b0:
	_ustrcspn (0000)
0018D3D0 00a0:
	_ustrncat (0000)
0018D470 0070:
	_ustrncmp (0000)
0018D4E0 0070:
	_ustrncpy (0000)
0018D550 00b0:
	_ustrpbrk (0000)
0018D600 0070:
	_ustrrchr (0000)
0018D670 00b0:
	_ustrspn (0000)
0018D720 00b0:
	_ustrstr (0000)
0018D7D0 0070:
	_ustrtok (0000)
0018D840 00d0:
	_ustrxfrm (0000)
0018D910 0070:
	_ustrlwr (0000)
0018D980 0070:
	_ustrupr (0000)
0018D9F0 00b0:
	_ustrnlwr (0000)
0018DAA0 00b0:
	_ustrnupr (0000)
0018DB50 00b0:
	_ustrcasecmp (0000)
0018DC00 00b0:
	_ustrncasecmp (0000)
0018DCB0 0020:
	_uisalpha (0000)
0018DCD0 0020:
	_uisupper (0000)
0018DCF0 0020:
	_uislower (0000)
0018DD10 0020:
	_uisdigit (0000)
0018DD30 0020:
	_uisxdigit (0000)
0018DD50 0020:
	_uisspace (0000)
0018DD70 0020:
	_uispunct (0000)
0018DD90 0020:
	_uisalnum (0000)
0018DDB0 0020:
	_uisprint (0000)
0018DDD0 0020:
	_uisgraph (0000)
0018DDF0 0020:
	_uiscntrl (0000)
0018DE10 0020:
	_utoupper (0000)
0018DE30 0020:
	_utolower (0000)
0018DE50 0040:
	_ufgetc (0000)
0018DE90 0040:
	_ufputc (0000)
0018DED0 0040:
	_uungetc (0000)
0018DF10 00a0:
	_ufgets (0000)
0018DFB0 00a0:
	_ufputs (0000)
0018E050 0070:
	_ugets (0000)
0018E0C0 0070:
	_uputs (0000)
0018E130 00a0:
	_ufprintf (0000)
0018E1D0 0070:
	_uprintf (0000)
0018E240 00b0:
	_usnprintf (0000)
0018E2F0 00b0:
	_usprintf (0000)
0018E3A0 0080:
	_uvfprintf (0000)
0018E420 0070:
	_uvprintf (0000)
0018E490 00b0:
	_uvsnprintf (0000)
0018E540 00b0:
	_uvsprintf (0000)
0018E5F0 00a0:
	_ufdopen (0000)
0018E690 00a0:
	_ufopen (0000)
0018E730 0040:
	_ufclose (0000)
0018E770 00b0:
	_ufreopen (0000)
0018E820 0070:
	_uperror (0000)
0018E890 00a0:
	_upopen (0000)
0018E930 0070:
	_uremove (0000)
0018E9A0 0010:
	_utmpnam (0000)
0018E9B0 0070:
	_ustrtol (0000)
0018EA20 0070:
	_ustrtoul (0000)
0018EA90 0070:
	_ustrtod (0000)
0018EB00 0070:
	_uatoi (0000)
0018EB70 0040:
	_uctime (0000)
0018EBB0 0040:
	_uasctime (0000)
0018EBF0 00b0:
	_wide_to_ascii (0000)
0018ECA0 00a0:
	_ascii_to_wide (0000)
0018ED40 0040:
	_ustrerror (0000)
002A2AC8 001c:
	??_C@_0BM@KPIDJFLK@count?5?$DM?5MAXIMUM_MEMCMP_SIZE?$AA@ (0000)
002A2AE4 001e:
	??_C@_0BO@BBLLNEKE@c?3?2halo?2SOURCE?2text?2unicode?4c?$AA@ (0000)
002A2B08 004f:
	??_C@_0EP@IPHIOENI@?$CI?$CI?$CIchar?5?$CK?$CJsrc?$CLcount?$CJ?5?$DM?$DN?5?$CIchar?5?$CK?$CJ@ (0000)
002A2B58 0036:
	??_C@_0DG@FFFMCJHE@?$CIcount?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIcount?5?$DM?5MAXIMUM@ (0000)
002A2B90 000c:
	??_C@_0M@BLMMNECI@dest?5?$CG?$CG?5src?$AA@ (0000)
002A2B9C 002f:
	??_C@_0CP@GDKCICIK@?$CIcount?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIcount?5?$DM?$DN?5MAXIMU@ (0000)
002A2BCC 0013:
	??_C@_0BD@ONBNFFHL@buffer1?5?$CG?$CG?5buffer2?$AA@ (0000)
002A2BE0 0037:
	??_C@_0DH@FKHPLIFK@?$CIcount?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIcount?5?$DM?$DN?5MAXIMU@ (0000)
002A2C18 002f:
	??_C@_0CP@HLJOKPIA@?$CIcount?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIcount?5?$DM?$DN?5MAXIMU@ (0000)
002A2C48 003b:
	??_C@_0DL@ILIANHOG@?$CI?$CIsrc?$CLsource_size?$CJ?5?$DM?5dest?$CJ?5?$HM?$HM?5?$CI?$CI@ (0000)
002A2C84 003a:
	??_C@_0DK@DPCFBKKM@?$CIsource_size?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIsource_si@ (0000)
002A2CC0 0022:
	??_C@_0CC@EMHAMOOL@wcslen?$CIsrc?$CJ?5?$DM?5MAXIMUM_STRING_SIZ@ (0000)
002A2CE4 0023:
	??_C@_0CD@MCFPOAN@wcslen?$CIdest?$CJ?5?$DM?5MAXIMUM_STRING_SI@ (0000)
002A2D08 0026:
	??_C@_0CG@JFACIFPH@wcslen?$CIstring2?$CJ?5?$DM?5MAXIMUM_STRING@ (0000)
002A2D30 0026:
	??_C@_0CG@KAOPDDKE@wcslen?$CIstring1?$CJ?5?$DM?5MAXIMUM_STRING@ (0000)
002A2D58 0013:
	??_C@_0BD@NBCEHOHI@string1?5?$CG?$CG?5string2?$AA@ (0000)
002A2D6C 001b:
	??_C@_0BL@PMAHACJJ@size?5?$DM?5MAXIMUM_STRING_SIZE?$AA@ (0000)
002A2D88 0025:
	??_C@_0CF@FCJMMMOO@wcslen?$CIstring?$CJ?5?$DM?5MAXIMUM_STRING_@ (0000)
002A2DB0 002c:
	??_C@_0CM@OEHOFCJH@wcslen?$CIcharacter_set?$CJ?5?$DM?5MAXIMUM_@ (0000)
002A2DDC 0018:
	??_C@_0BI@CNAFLGLG@string?5?$CG?$CG?5character_set?$AA@ (0000)
002A2DF4 002e:
	??_C@_0CO@JBLCAJEF@?$CIcount?5?$DO?$DN?50?$CJ?5?$CG?$CG?5?$CIcount?5?$DM?5MAXIMUM@ (0000)
002A2E24 0029:
	??_C@_0CJ@CFPGNFCG@wcslen?$CIdelimiters?$CJ?5?$DM?5MAXIMUM_STR@ (0000)
002A2E50 000b:
	??_C@_0L@KABPAJDP@delimiters?$AA@ (0000)
002A2E5C 001c:
	??_C@_0BM@EKKPEFAL@count?5?$DM?5MAXIMUM_STRING_SIZE?$AA@ (0000)
002A2E78 0007:
	??_C@_06DJAMKFID@stream?$AA@ (0000)
002A2E80 0025:
	??_C@_0CF@ODNOBAON@wcslen?$CIformat?$CJ?5?$DM?5MAXIMUM_STRING_@ (0000)
002A2EA8 002c:
	??_C@_0CM@INOIICAC@?$CIsize?5?$DO?50?$CJ?5?$CG?$CG?5?$CIsize?5?$DM?$DN?5MAXIMUM_S@ (0000)
002A2ED4 0011:
	??_C@_0BB@LLHJAMON@string?5?$CG?$CG?5format?$AA@ (0000)
002A2EE8 0011:
	??_C@_0BB@FMMLKAJE@stream?5?$CG?$CG?5format?$AA@ (0000)
002A2EFC 0007:
	??_C@_06CPKOMEJH@fd?5?$DO?50?$AA@ (0000)
002A2F04 0023:
	??_C@_0CD@CHIODFOI@wcslen?$CIpath?$CJ?5?$DM?5MAXIMUM_STRING_SI@ (0000)
002A2F28 0005:
	??_C@_04LNEJFJGI@path?$AA@ (0000)
002A2F30 0028:
	??_C@_0CI@OMCNEDMK@wcslen?$CImode?$CJ?5?$DM?5MAXIMUM_MODE_STRI@ (0000)
002A2F58 000d:
	??_C@_0N@DGNJMIDO@path?5?$CG?$CG?5mode?$AA@ (0000)
002A2F68 0026:
	??_C@_0CG@IBGAHBIE@wcslen?$CIcommand?$CJ?5?$DM?5MAXIMUM_STRING@ (0000)
002A2F90 0010:
	??_C@_0BA@ENEADPPF@command?5?$CG?$CG?5mode?$AA@ (0000)
002A2FA0 0023:
	??_C@_0CD@OAEACAAO@wcslen?$CInptr?$CJ?5?$DM?5MAXIMUM_STRING_SI@ (0000)
002A2FC4 0005:
	??_C@_04DKBBLKKK@nptr?$AA@ (0000)
002A2FCC 0006:
	??_C@_05NAPFDJNG@timer?$AA@ (0000)
002A2FD4 0008:
	??_C@_07GMJPCJBH@timeptr?$AA@ (0000)
002A2FDC 001d:
	??_C@_0BN@GMFKDIGL@length?5?$DM?5MAXIMUM_STRING_SIZE?$AA@ (0000)
002A2FFC 0011:
	??_C@_0BB@LBAPFJHB@unicode?5?$CG?$CG?5ascii?$AA@ (0000)
002A3010 0011:
	??_C@_0BB@IALIJFPB@ascii?5?$CG?$CG?5unicode?$AA@ (0000)
002A3024 0008:
	??_C@_17LGGGMOOB@?$AA?$CF?$AAh?$AAs?$AA?$AA@ (0000)
004C1A08 0200:
	_bss_004c1a08 (0000)
*/

/* ---------- headers */

#include <wchar.h>
#include <wctype.h>
#include <stdarg.h>

#include "cseries/cseries.h"
#include "unicode.h"

/* ---------- constants */

enum
{
	MAXIMUM_MEMCMP_SIZE = 0x10000000,
	MAXIMUM_MEMCPY_MEMMOVE_SIZE = 0x10000000,
	MAXIMUM_MEMSET_SIZE = 0x10000000,
	MAXIMUM_MODE_STRING_SIZE = 4,
	MAXIMUM_STRING_SIZE = 0x8000,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static wchar_t bss_004c1a08[0x100];

/* ---------- public code */

int uisalpha(
	wchar_t character)
{
	return iswalpha(character);
}

int uisupper(
	wchar_t character)
{
	return iswupper(character);
}

int uislower(
	wchar_t character)
{
	return iswlower(character);
}

int uisdigit(
	wchar_t character)
{
	return iswdigit(character);
}

int uisxdigit(
	wchar_t character)
{
	return iswxdigit(character);
}

int uisspace(
	wchar_t character)
{
	return iswspace(character);
}

int uispunct(
	wchar_t character)
{
	return iswpunct(character);
}

int uisalnum(
	wchar_t character)
{
	return iswalnum(character);
}

int uisprint(
	wchar_t character)
{
	return iswprint(character);
}

int uisgraph(
	wchar_t character)
{
	return iswgraph(character);
}

int uiscntrl(
	wchar_t character)
{
	return iswcntrl(character);
}

int utoupper(
	wchar_t character)
{
	return towupper(character);
}

int utolower(
	wchar_t character)
{
	return towlower(character);
}

void *umemchr(
	void const *buffer,
	int value,
	unsigned long count)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		84,
		buffer);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		85,
		count < MAXIMUM_MEMCMP_SIZE);

	return memchr(buffer, value, count);
}

long umemcmp(
	void const *buffer1,
	void const *buffer2,
	unsigned long count)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		109,
		buffer1 && buffer2);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		110,
		(count >= 0) && (count <= MAXIMUM_MEMCMP_SIZE));

	return csmemcmp(buffer1, buffer2, count);
}

void *umemmove(
	void *dest,
	void const *src,
	unsigned long count)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		121,
		dest && src);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		122,
		(count >= 0) && (count <= MAXIMUM_MEMCPY_MEMMOVE_SIZE));

	return csmemmove(dest, src, count);
}

wint_t ufgetc(
	FILE *stream)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		589,
		stream);

	return fgetwc(stream);
}

wint_t ufputc(
	wchar_t character,
	FILE *stream)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		599,
		stream);

	return fputwc(character, stream);
}

wint_t uungetc(
	wchar_t character,
	FILE *stream)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		609,
		stream);

	return ungetwc(character, stream);
}

int ufclose(
	FILE *stream)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		900,
		stream);

	return fclose(stream);
}

wchar_t *uctime(
	time_t const *timer)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		1014,
		timer);

	return _wctime(timer);
}

wchar_t *uasctime(
	struct tm const *timeptr)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		1023,
		timeptr);

	return _wasctime(timeptr);
}

wchar_t *utmpnam(
	wchar_t *string)
{
	return _wtmpnam(string);
}

unsigned long ustrlen(
	wchar_t const *string)
{
	unsigned long size;

	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		194,
		string);

	size = wcslen(string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		196,
		size < MAXIMUM_STRING_SIZE);

	return size;
}

wchar_t *ustrchr(
	wchar_t const *string,
	wchar_t character)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		224,
		string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		225,
		wcslen(string) < MAXIMUM_STRING_SIZE);

	return wcschr(string, character);
}

wchar_t *ustrrchr(
	wchar_t const *string,
	wchar_t character)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		333,
		string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		334,
		wcslen(string) < MAXIMUM_STRING_SIZE);

	return wcsrchr(string, character);
}

wchar_t *ustrlwr(
	wchar_t *string)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		392,
		string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		393,
		wcslen(string) < MAXIMUM_STRING_SIZE);

	return _wcslwr(string);
}

wchar_t *ustrupr(
	wchar_t *string)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		402,
		string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		403,
		wcslen(string) < MAXIMUM_STRING_SIZE);

	return _wcsupr(string);
}

wchar_t *ugets(
	wchar_t *string)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		643,
		string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		644,
		wcslen(string) < MAXIMUM_STRING_SIZE);

	return _getws(string);
}

int uputs(
	wchar_t const *string)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		665,
		string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		666,
		wcslen(string) < MAXIMUM_STRING_SIZE);

	return _putws(string);
}

int uremove(
	wchar_t const *path)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		946,
		path);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		947,
		wcslen(path) < MAXIMUM_STRING_SIZE);

	return _wremove(path);
}

long ustrtol(
	wchar_t const *nptr,
	wchar_t **endptr,
	int base)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		967,
		nptr);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		968,
		wcslen(nptr) < MAXIMUM_STRING_SIZE);

	return wcstol(nptr, endptr, base);
}

int uatoi(
	wchar_t const *string)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		1002,
		string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		1003,
		wcslen(string) < MAXIMUM_STRING_SIZE);

	return _wtoi(string);
}

void *umemset(
	void *buffer,
	int value,
	unsigned long count)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		133,
		buffer);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		134,
		(count >= 0) && (count <= MAXIMUM_MEMSET_SIZE));

	return csmemset(buffer, value, count);
}

long ustrncmp(
	wchar_t const *string1,
	wchar_t const *string2,
	unsigned long count)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		298,
		string1 && string2);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		299,
		(count >= 0) && (count < MAXIMUM_STRING_SIZE));

	return wcsncmp(string1, string2, count);
}

wchar_t *ustrncpy(
	wchar_t *dest,
	wchar_t const *src,
	unsigned long count)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		310,
		dest && src);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		311,
		(count >= 0) && (count < MAXIMUM_STRING_SIZE));

	return wcsncpy(dest, src, count);
}

wchar_t *ustrtok(
	wchar_t *string,
	wchar_t const *delimiters)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		368,
		delimiters);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		369,
		wcslen(delimiters) < MAXIMUM_STRING_SIZE);

	return wcstok(string, delimiters);
}

int uprintf(
	wchar_t const *format,
	...)
{
	int result;
	va_list arglist;

	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		699,
		format);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		700,
		wcslen(format) < MAXIMUM_STRING_SIZE);

	va_start(arglist, format);
	result = vwprintf(format, arglist);
	va_end(arglist);

	return result;
}

int uvprintf(
	wchar_t const *format,
	va_list arglist)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		803,
		format);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		804,
		wcslen(format) < MAXIMUM_STRING_SIZE);

	return vwprintf(format, arglist);
}

void uperror(
	wchar_t const *string)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		922,
		string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		923,
		wcslen(string) < MAXIMUM_STRING_SIZE);

	_wperror(string);

	return;
}

unsigned long ustrtoul(
	wchar_t const *nptr,
	wchar_t **endptr,
	int base)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		979,
		nptr);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		980,
		wcslen(nptr) < MAXIMUM_STRING_SIZE);

	return wcstoul(nptr, endptr, base);
}

double ustrtod(
	wchar_t const *nptr,
	wchar_t **endptr)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		990,
		nptr);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		991,
		wcslen(nptr) < MAXIMUM_STRING_SIZE);

	return wcstod(nptr, endptr);
}

unsigned long ustrnlen(
	wchar_t const *string,
	unsigned long maximum_length)
{
	unsigned long size;

	size = 0;
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		208,
		string);

	while ((size < maximum_length) && *string++)
		size++;

	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		214,
		size < MAXIMUM_STRING_SIZE);

	return size;
}

wchar_t *ustrcpy(
	wchar_t *dest,
	wchar_t const *src)
{
	unsigned long source_size;

	source_size = wcslen(src);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		146,
		(source_size >= 0) && (source_size < MAXIMUM_STRING_SIZE));
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		147,
		((src+source_size) < dest) || ((dest + source_size) < src));

	return wcscpy(dest, src);
}

int uvfprintf(
	FILE *stream,
	wchar_t const *format,
	va_list arglist)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		792,
		stream && format);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		793,
		wcslen(format) < MAXIMUM_STRING_SIZE);

	return vfwprintf(stream, format, arglist);
}

void *umemcpy(
	void *dest,
	void const *src,
	unsigned long count)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		96,
		dest && src);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		97,
		(count >= 0) && (count < MAXIMUM_MEMCPY_MEMMOVE_SIZE));
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		98,
		(((char *)src+count) <= (char *)dest) || (((char *)dest+count) <= (char *)src));

	return csmemcpy(dest, src, count);
}

wchar_t *ustrncat(
	wchar_t *dest,
	wchar_t const *src,
	unsigned long count)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		273,
		dest && src);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		274,
		wcslen(dest) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		275,
		(count >= 0) && (count < MAXIMUM_STRING_SIZE));

	return wcsncat(dest, src, count);
}

wchar_t *ufgets(
	wchar_t *string,
	int size,
	FILE *stream)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		620,
		string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		621,
		wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		622,
		size < MAXIMUM_STRING_SIZE);

	return fgetws(string, size, stream);
}

int ufputs(
	wchar_t const *string,
	FILE *stream)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		632,
		string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		633,
		wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		634,
		stream);

	return fputws(string, stream);
}

int ufprintf(
	FILE *stream,
	wchar_t const *format,
	...)
{
	int result;
	va_list arglist;

	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		680,
		stream);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		681,
		format);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		682,
		wcslen(format) < MAXIMUM_STRING_SIZE);

	va_start(arglist, format);
	result = vfwprintf(stream, format, arglist);
	va_end(arglist);

	return result;
}

FILE *ufdopen(
	int fd,
	wchar_t const *path)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		877,
		path);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		878,
		wcslen(path) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		879,
		fd > 0);

	return _wfdopen(fd, path);
}

FILE *ufopen(
	wchar_t const *path,
	wchar_t const *mode)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		889,
		path && mode);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		890,
		wcslen(path) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		891,
		wcslen(mode) < MAXIMUM_MODE_STRING_SIZE);

	return _wfopen(path, mode);
}

FILE *upopen(
	wchar_t const *command,
	wchar_t const *mode)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		935,
		command && mode);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		936,
		wcslen(command) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		937,
		wcslen(mode) < MAXIMUM_MODE_STRING_SIZE);

	return NULL;
}

wchar_t *ascii_to_wide(
	char const *ascii,
	wchar_t *unicode,
	unsigned long size)
{
	unsigned long length;
	long i;

	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		1087,
		ascii && unicode);

	length = csstrlen(ascii);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		1089,
		length < MAXIMUM_STRING_SIZE);

	if (size >= 2 * length + 2)
	{
		unicode[length] = 0;
		for (i = length - 1; i >= 0; i--)
			unicode[i] = (short)ascii[i];

		return unicode;
	}

	return NULL;
}

wchar_t *ustrcat(
	wchar_t *dest,
	wchar_t const *src)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		157,
		dest && src);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		158,
		wcslen(dest) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		159,
		wcslen(src) < MAXIMUM_STRING_SIZE);

	return wcscat(dest, src);
}

long ustrcmp(
	wchar_t const *string1,
	wchar_t const *string2)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		181,
		string1 && string2);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		182,
		wcslen(string1) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		183,
		wcslen(string2) < MAXIMUM_STRING_SIZE);

	return wcscmp(string1, string2);
}

long ustrcoll(
	wchar_t const *string1,
	wchar_t const *string2)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		235,
		string1 && string2);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		236,
		wcslen(string1) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		237,
		wcslen(string2) < MAXIMUM_STRING_SIZE);

	return wcscoll(string1, string2);
}

unsigned long ustrcspn(
	wchar_t const *string,
	wchar_t const *character_set)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		247,
		string && character_set);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		248,
		wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		249,
		wcslen(character_set) < MAXIMUM_STRING_SIZE);

	return wcscspn(string, character_set);
}

wchar_t *ustrpbrk(
	wchar_t const *string,
	wchar_t const *character_set)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		321,
		string && character_set);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		322,
		wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		323,
		wcslen(character_set) < MAXIMUM_STRING_SIZE);

	return wcspbrk(string, character_set);
}

unsigned long ustrspn(
	wchar_t const *string,
	wchar_t const *character_set)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		344,
		string && character_set);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		345,
		wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		346,
		wcslen(character_set) < MAXIMUM_STRING_SIZE);

	return wcsspn(string, character_set);
}

wchar_t *ustrstr(
	wchar_t const *string,
	wchar_t const *character_set)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		356,
		string && character_set);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		357,
		wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		358,
		wcslen(character_set) < MAXIMUM_STRING_SIZE);

	return wcsstr(string, character_set);
}

wchar_t *ustrnlwr(
	wchar_t *string,
	unsigned long count)
{
	wchar_t *position;

	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		415,
		string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		416,
		wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		417,
		count < MAXIMUM_STRING_SIZE);

	for (position = string; *position; position++)
		*position = towupper(*position);

	return string;
}

wchar_t *ustrnupr(
	wchar_t *string,
	unsigned long count)
{
	wchar_t *position;

	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		435,
		string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		436,
		wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		437,
		count < MAXIMUM_STRING_SIZE);

	for (position = string; *position; position++)
		*position = towlower(*position);

	return string;
}

unsigned long ustrxfrm(
	wchar_t *dest,
	wchar_t const *src,
	unsigned long count)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		380,
		dest && src);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		381,
		wcslen(dest) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		382,
		wcslen(src) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		383,
		count < MAXIMUM_STRING_SIZE);

	return wcsxfrm(dest, src, count);
}

long ustrcasecmp(
	wchar_t const *string1,
	wchar_t const *string2)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		455,
		string1 && string2);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		456,
		wcslen(string1) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		457,
		wcslen(string2) < MAXIMUM_STRING_SIZE);

	return _wcsicmp(string1, string2);
}

long ustrncasecmp(
	wchar_t const *string1,
	wchar_t const *string2,
	unsigned long count)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		472,
		string1 && string2);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		473,
		wcslen(string1) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		474,
		wcslen(string2) < MAXIMUM_STRING_SIZE);

	return _wcsnicmp(string1, string2, count);
}

int usnprintf(
	wchar_t *string,
	unsigned long size,
	wchar_t const *format,
	...)
{
	int result;
	va_list arglist;

	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		719,
		string);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		720,
		(size > 0) && (size <= MAXIMUM_STRING_SIZE));
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		721,
		wcslen(format) < MAXIMUM_STRING_SIZE);

	va_start(arglist, format);
	result = _vsnwprintf(string, size, format, arglist);
	va_end(arglist);

	return result;
}

int usprintf(
	wchar_t *string,
	wchar_t const *format,
	...)
{
	int result;
	va_list arglist;

	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		751,
		string && format);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		752,
		wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		753,
		wcslen(format) < MAXIMUM_STRING_SIZE);

	va_start(arglist, format);
	result = vswprintf(string, format, arglist);
	va_end(arglist);

	return result;
}

int uvsnprintf(
	wchar_t *string,
	unsigned long size,
	wchar_t const *format,
	va_list arglist)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		816,
		string && format);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		817,
		wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		818,
		wcslen(format) < MAXIMUM_STRING_SIZE);

	return _vsnwprintf(string, size, format, arglist);
}

int uvsprintf(
	wchar_t *string,
	wchar_t const *format,
	va_list arglist)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		841,
		string && format);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		842,
		wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		843,
		wcslen(format) < MAXIMUM_STRING_SIZE);

	return vswprintf(string, format, arglist);
}

FILE *ufreopen(
	wchar_t const *path,
	wchar_t const *mode,
	FILE *stream)
{
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		911,
		path && mode);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		912,
		wcslen(path) < MAXIMUM_STRING_SIZE);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		913,
		wcslen(mode) < MAXIMUM_MODE_STRING_SIZE);

	return _wfreopen(path, mode, stream);
}

char *wide_to_ascii(
	wchar_t const *unicode,
	char *ascii,
	unsigned long size)
{
	unsigned long length;
	unsigned long i;

	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		1040,
		unicode && ascii);

	length = wcslen(unicode);
	match_assert(
		"c:\\halo\\SOURCE\\text\\unicode.c",
		1042,
		length < MAXIMUM_STRING_SIZE);

	if (length > size - 1)
		return NULL;

	for (i = 0; i < length; i++)
	{
		if (unicode[i] & 0xFF80)
			return NULL;
	}

	for (i = 0; i < length; i++)
		ascii[i] = (char)unicode[i];

	ascii[i] = 0;

	return ascii;
}

wchar_t *ustrerror(
	int error_number)
{
	bss_004c1a08[0] = 0;
	usnprintf(bss_004c1a08, 0x100, L"%hs", strerror(error_number));

	return bss_004c1a08;
}

/* ---------- private code */
