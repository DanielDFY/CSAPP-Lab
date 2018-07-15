#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 8.0.0.  Version 8.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2014, plus Amendment 1 (published
   2015-05-15).  */
/* We do not support C11 <threads.h>.  */
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  /* Get digits which is 0 in both x and y. Then use ~ to get the result */
  int both_zero = ~x | ~y;
  int result = ~both_zero;
  return result;
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  /* Exploit ability of shifts to compute 4*n */
  int n_hex = n << 3;
  int x_trim = x >> n_hex;
  int result = 0xff & x_trim;
  return result;
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
 /*
  * Exploit !!n to determine whether to shift. 
  * Then exploit ability of shifts to get binary mask.
  */
  int head = (!!n) & 1;
  int mask = ~(head << 31 >> (n + (~1 + 1)));
  int result = (x >> n) & mask;
  return result;
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  /* Exploit divide-and-conquer and calculate (x&1)+((x>>1)&1) in every step (regard x as 2-bit integer) */
  int mask1_tmp = (0x55 << 8) | 0x55;
  int mask1 = (mask1_tmp << 16) | mask1_tmp;

  int mask2_tmp = (0x33 << 8) | 0x33;
  int mask2 = (mask2_tmp << 16) | mask2_tmp;

  int mask3_tmp = (0x0f << 8) | 0x0f;
  int mask3 = (mask3_tmp << 16) | mask3_tmp;

  int mask4 = (0xff << 16) | 0xff;

  int mask5 = (0xff << 8) | 0xff;

  x=(x & mask1) + ((x >> 1) & mask1);
  x=(x & mask2) + ((x >> 2) & mask2);
  x=(x & mask3) + ((x >> 4) & mask3);
  x=(x & mask4) + ((x >> 8) & mask4);
  x=(x & mask5) + ((x >> 16) & mask5);
  return x;
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  /* ~0+1=0, ~x+x=0xffffffff if x!=0 */
  int if_zero = (x | (~x + 1)) >> 31;
  return (~if_zero) & 1;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 1 << 31;
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
 /*
  * Exploit ability of shift to transfer 32-n digits from the big endian into the value of the nth bit.
  * Use ^ and ! to know if transfered x and x are equal.
  * Equal means x is an n-bit.
  */
  int shift = 32 + (~n + 1);
  int change_big_endian = (x << shift) >> shift;
  int result = x ^ change_big_endian;
  return !result;
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
    /* Use bias to determine whether round up or round down */
    int sign = x >> 31;
    int mask = (1 << n) + (~0);
    int bias = sign & mask;
    int result = (x + bias) >> n;
	  return result;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
  /* (x>>31) >0 if x<0, !x=1 if x=0 */ 
  return !((x >> 31) | (!x));
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
/* 
 * Classify the discusstion according to whether the sign bits are same.
 * If same, compute x-y immediately.
 * If differ, check the sign bit of y.
 */
	int sign_x = (x >> 31) & 1;
	int sign_y = (y >> 31) & 1;
	int sign_differ = (sign_x ^ sign_y) & 1;
	int sign_same = !sign_differ;
	int sub_sign = (x + (~y)) >> 31;
	int result = (sub_sign & sign_same) | (~sign_y & sign_differ);
	return result;
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
/* 
 * Use dichotomy to find the maximum effective position of x.
 * Sum in every step. 
 */
  int divide_1, divide_2, divide_3, divide_4, divide_5;
  int result = 0;

  divide_1 = (!!(x >> 16)) << 4;
  x >>= divide_1;
  result += divide_1;

  divide_2 = (!!(x >> 8)) << 3;
  x >>= divide_2;
  result += divide_2;

  divide_3 = (!!(x >> 4)) << 2;
  x >>= divide_3;
  result += divide_3;

  divide_4 = (!!(x >> 2)) << 1;
  x >>= divide_4;
  result += divide_4;

  divide_5 = (!!(x >> 1));
  x >>= divide_5;
  result += divide_5;

  return result;
}
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
  /* Use abs to check NaN. If uf is NaN, return itself, else change the sign bit */
  unsigned abs = uf & 0x7fffffff;
  unsigned reverse = uf ^ 0x80000000;
  if(abs > 0x7f800000)
    return uf;
  return reverse;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
/*
 * First check if x is 0.
 * Then get ths absolute value and sign bit of x
 * Normalize x and count the shifting
 * Use the formula to get the float
 * Finally round x;
 */
  unsigned sign = 0;
  unsigned abs = x;
  unsigned shifting = 0;
  unsigned tmp = 0;
  unsigned count = 0;
  unsigned shifted = 0;
  unsigned result = 0;

  if(!x) 
    return 0;
  else if(x >> 31){
    sign = 0x80000000;
    abs = -x;
  }

  shifting = abs;

  while(1){
    tmp = shifting;
    shifting <<= 1;
    ++count;
    if(tmp & 0x80000000)
      break;
  }

  shifted = shifting;

  result = sign + (shifted >> 9) + ((159 - count) << 23);
  
  if(((shifted & 0x1ff) > 0x100 )|| !((shifted & 0x3ff) ^ 0x300))
    result += 1;
  return result;
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  /* Classify the discussion into normalized value, denoramlized value, NaN and 0 */
  if(!(uf & 0x7f800000))
    return ((uf & 0x007fffff) << 1) | (uf & 0x80000000);
  else if((uf & 0x7f800000) ^ 0x7f800000)
    return uf + 0x00800000;
  return uf;
}