#include "harness.h"

#include <math.h>
#include <stdint.h>
#include "under_test.inc"

static uint32_t bits(float value)
{
	uint32_t result;

	memcpy(&result, &value, sizeof(result));
	return result;
}

static float from_bits(uint32_t value)
{
	float result;

	memcpy(&result, &value, sizeof(result));
	return result;
}

/* square_root against the root the game took before, of the float as a
double rounded back, to the bit (a zero's sign too; any NaN for a NaN) */
static void compare(uint32_t x_bits)
{
	float x = from_bits(x_bits);
	float expected = (float)sqrt((double)x), result = square_root(x);

	if (isnan(expected))
		CHECK(isnan(result), "square_root(%.9g) [%08x]: %.9g, not NaN", x, (unsigned)x_bits, result);
	else
		CHECK(bits(result) == bits(expected), "square_root(%.9g) [%08x]: %.9g [%08x], not %.9g [%08x]",
			x, (unsigned)x_bits, result, (unsigned)bits(result), expected, (unsigned)bits(expected));
}

int main(int argc, char **argv)
{
	const char *case_name = argv[1];
	uint32_t seed = 12345;
	uint32_t sign, exponent, i;

	CASE("every exponent")
	{
		/* both signs, every exponent (zeros and subnormals, infinities and
		NaNs too): the mantissa's ends and 4096 others */
		for (sign = 0; sign < 2; sign++)
		{
			for (exponent = 0; exponent < 256; exponent++)
			{
				uint32_t base = (sign << 31) | (exponent << 23);

				compare(base);
				compare(base | 1);
				compare(base | 0x7FFFFF);
				compare(base | 0x400000);
				for (i = 0; i < 4096; i++)
				{
					seed = seed * 1664525u + 1013904223u;
					compare(base | (seed >> 9));
				}
			}
		}
	}
	CASE("squares")
	{
		/* exact roots, and the floats either side of a square, where a
		root rounds closest to halfway */
		for (i = 1; i < 4096; i++)
		{
			uint32_t square = bits((float)i * (float)i);

			compare(square - 1);
			compare(square);
			compare(square + 1);
		}
	}
	return 0;
}
