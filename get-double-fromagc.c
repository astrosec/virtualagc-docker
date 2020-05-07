#include <stdio.h>
#include <stdlib.h>


#define SP (unsigned)1
#define DP (unsigned)2

typedef int GdbmiComplement_t;

typedef struct {
  char     type[3];
  unsigned precision;
  double   scalar;
} GdmiScalar_t;

const GdmiScalar_t GdbmiScalarMap[] = {
  {"FF",SP, 85.41},            // TRIM DEGREES: seconds of arc
  {"GG",SP,  1.0/16384},       // INERTIA: Frac. of 1,048,576 kg m²
  {"II",SP,  1.0/16384},       // THRUST MOMENT: Frac. of 1,048,576 Nm
  {"JJ",DP,  2.0},             // POSITION5: m
  {"KK",SP,  1.0/16384},       // WEIGHT2: kg
  {"LL",DP,  0.00008055},      // POSITION6: Nautical Miles.
  {"MM",DP, 25.0/268435456},   // DRAG ACCELERATION: G
  {"PP",DP,  1.0},             // 2 INTEGERS
  {"UU",DP,  1.0/268435456},   // VELOCITY/2VS: Frac. of 51,532.3946 feet/sec
  {"VV",DP,  0.00005128},      // POSITION8: Nautical miles
  {"XX",DP,  1.0/512},         // POSITION9: Meters
  {"YY",DP,  1.0/268435456},   // VELOCITY4: Meters/Centisec
  {"SP",SP,  1.0/16384},       // GENERIC: Single Precision Fraction.
  {"DP",DP,  1.0/268435456},   // GENERIC: Double Precision Fraction.
/*  {"TP",DP,  1.0/4398046511104}// GENERIC: Triple Precision Fraction.  */
  { "A",SP,  1.0},             // OCTAL: Octal
  { "B",SP,  1.0/16384},       // FRACTIONAL: Fraction (Default)
  { "C",SP,  1.0},             // WHOLE: 1 Unit
  { "D",SP,360.0/32768},       // CDU DEGREES: Degrees (15 Bit 2s Complement)
  { "E",SP, 90.0/16384},       // ELEVATION DEGREES: Degrees
  { "F",SP,180.0/16384},       // DEGREES      : Degrees
  { "G",DP,360.0/268435456},   // DEGREES  (90): Degrees
  { "H",DP,360.0/268435456},   // DEGREES (360): Degrees
  { "J",SP, 90.0/32768},       // DEGREES: Degrees (15 bit 2s Complement)
  { "K",DP,  1.0/100},         // TIME (HR, MIN, SEC): seconds
  { "L",DP,  1.0/100},         // TIME (MIN SEC): minutes | seconds
  { "M",SP,  1.0/100},         // TIME (SEC): seconds
  { "N",DP,  1.0/100},         // TIME (SEC): seconds
  { "P",DP,  1.0/2097152},     // VELOCITY 2: meters/centi-sec
  { "Q",DP,  2.0},             // POSITION 4: meters
  { "S",DP,  1.0/2097152},     // VELOCITY 3: meters/centi-sec
  { "T",SP,  1.0/100},         // G
};


//agc_value = (agc_value << 16) | DbgGetValueByAddress (gdbmi_addr + 1);

static int GdbmiScalar = 13;

int GdbmiIntFromAgc(unsigned short value, GdbmiComplement_t format)
{
  int result = value; /* Assume 15 bit 2s Comp positive*/
  
  if (format == 1 )
  {  /* 14 bit 1s Comp */
     if (value & 0x4000) result = ((value ^ 0x3fff) & 0x3fff) * -1;
     else result = value & 0x3fff;
  }
  
  return result;
}

double GdbmiDoubleFromAgc(unsigned value)
{
  int precision = GdbmiScalarMap[GdbmiScalar].precision;
  GdbmiComplement_t c = 2;
  double scalar = GdbmiScalarMap[GdbmiScalar].scalar;
  
  unsigned lower = value & 0x0000ffff;
  unsigned upper = (value & 0xffff0000) >> 16;
  double result = 0;
  
  if (GdbmiScalarMap[GdbmiScalar].type[0] == 'D' || 
      GdbmiScalarMap[GdbmiScalar].type[0] == 'J') c = 2; /* Use Twos Complement */
  
  if (precision == SP){
    result = GdbmiIntFromAgc(upper,c) * scalar;
  }
  else { /* Double Precision */
    result = (GdbmiIntFromAgc(upper,c) * 16384 + GdbmiIntFromAgc(lower,c)) * scalar;
  }
  return result;
}


int main(int argc, char **argv)
{

	if (argc<3)
	{
		printf("./a.out 06220 037553\n");
                printf("result is 3.141592681\n");
		printf("Usage: %s octalval octalval \n", argv[0]);
	}

     unsigned agc_value = ( (strtol(argv[1],NULL,8) << 16) | strtol(argv[2],NULL,8));


     double result = GdbmiDoubleFromAgc(agc_value);
     printf("result is %.*f\n",9,result*16);
     return 0;
}
