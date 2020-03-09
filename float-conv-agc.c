#include <stdio.h>
#include <stdlib.h>		/* strtod */
#include <math.h>		/* pow */

//Converts from floating point to AGC native octal representation
//Inputs are floating point numbers

//--- 2DEC: (original) operand="3.141592653" mod1="B-4" mod2="" extra=""
//--- 2DEC: (modified) operand="3.141593" mod1="B-4" mod2=""
//060455,000703: 27,3355           06220 37553  PI/16              2DEC     3.141592653 B-4         

//-------------------------------------------------------------------------
// Converts a string like "E+-n" or "B+-n" to a scale factor.

double ScaleFactor(char *s)
{
	int n;
	printf("Scalefactor %s\n", s);
	if (*s == 0)
		return (1.0);

	if (*s == 'E') {
		n = atoi(s + 1);
		return (pow(10.0, n));
	}
	if (*s == 'B') {
		n = atoi(s + 1);
		return (pow(2.0, n));
	}

	return (1.0);
}

//--- 2DEC: (original) operand="3.141592653" mod1="B-4" mod2="" extra=""
//--- 2DEC: (modified) operand="3.141593" mod1="B-4" mod2=""

int main(int argc, char **argv)
{
	char default_operand[] = "3.141592653";
	char *operand = NULL;
	char mod1[] = "B-4";
	double floatpoint;
	double x;
	double tmpval;
	int Sign, Value, i;


	if(2>=argc)
	{
		operand = argv[1];
	} else {
		operand = default_operand;
	}
			
	printf("Input operand %s\n", operand);
	floatpoint = strtod(operand, NULL);
	printf("In Floating point %f\n", floatpoint);

	x = floatpoint * ScaleFactor(mod1) * ScaleFactor(" ");
	// Convert to 1's complement format.
	Sign = 0;
	if (operand[0] == '-') {
		// x < 0
		Sign = 1;
		x = -x;
	}

	if (fmod(x, 1.0) == 0.0) {
		// Integer: just convert directly to octal.
		Value = (int)x;
	} else {
		// Floating point: scale. FP numbers > 1.0 are an error.
		if (x >= 1.0)
			return (1);

		for (Value = 0, i = 0; i < 28; i++) {
			Value = Value << 1;
			if (x >= 0.5) {
				Value++;
				x -= 0.5;
			}
			x *= 2;
		}

		if (x >= 0.5 && Value < 0x0fffffff)
			Value++;
	}

	i = Value & 0x00003fff;
	Value = (Value >> 14) & 0x00003fff;
	if (Sign) {
		Value = ~Value;
		i = ~i;
		i &= 0x00007fff;
		Value &= 0x00007fff;
	}
	printf("Value %o %o\n", Value, i);
	return 0;
}
