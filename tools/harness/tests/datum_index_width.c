#include <stdarg.h>
#include "harness.h"
#undef UNSIGNED_LONG_MAX
#undef DATUM_INDEX_TO_ABSOLUTE_INDEX
#include "under_test.inc"

/* a printf-style callee: a long, then a string */
static char const *string_after_long(int count, ...)
{
	va_list arguments;
	long first;
	char const *second;

	va_start(arguments, count);
	first = va_arg(arguments, long);
	second = va_arg(arguments, char const *);
	va_end(arguments);
	return first == 0x1234 ? second : NULL;
}

int main(int argc, char **argv)
{
	char const *case_name = argc == 2 ? argv[1] : "";
	long datum_index = (long)0x80011234UL;

	CASE("size")
		CHECK(sizeof(DATUM_INDEX_TO_ABSOLUTE_INDEX(datum_index)) == sizeof(long), "%u bytes",
			(unsigned)sizeof(DATUM_INDEX_TO_ABSOLUTE_INDEX(datum_index)));
	else CASE("varargs")
	{
		char const *name = string_after_long(2, DATUM_INDEX_TO_ABSOLUTE_INDEX(datum_index), "name");

		CHECK(name && !strcmp(name, "name"), "the argument after the index shifted");
	}
	else return 2;
	return 0;
}
