# Data Lab

### bitAnd

> x & y using only | and ˜

```
int bitAnd(int x, int y) {
	int both_zero = ~x | ~y;
	int result = ~both_zero;
	return result;
}

/* Get digits which is 0 in both x and y. Then use ~ to get the result */
```

### getByte

>Extract byte n from word x
>
>Bytes numbered from 0 (LSB) to 3 (MSB)
>
>Examples: getByte(0x12345678,1) = 0x56
>
>Legal ops: ! ~ & ^ | + << >>
>
>Max ops: 6

```c
int getByte(int x, int n) {
	int n_bit = n << 3;
	int x_trim = x >> n_bit;
	int result = 0xff & x_trim;
	return result;
}

/* Exploit ability of shifts to compute 4*n */
```

### logicalShift

>shift x to the right by n, using a logical shift
>
>Can assume that 0 <= n <= 31
>
>Examples: logicalShift(0x87654321,4) = 0x08765432
>
>Legal ops: ! ~ & ^ | + << >>
>
>Max ops: 20

```c
int logicalShift(int x, int n) {
	int head = (!!n) & 1;
	int mask = ~(head << 31 >> (n + (~1 + 1)));
	int result = (x >> n) & mask;
	return result;
}

/*
 * Exploit !!n to determine whether to shift. 
 * Then exploit ability of shifts to get binary mask.
 */

```

### bitCount

>returns count of number of 1's in word
>
>Examples: bitCount(5) = 2, bitCount(7) = 3
>
>Legal ops: ! ~ & ^ | + << >>
>
>Max ops: 40

```c
int bitCount(int x) {
	int mask1_tmp = (0x55 << 8) | 0x55;
	int mask1 = (mask1_tmp << 16) | mask1_tmp
	
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
 * Exploit divide-and-conquer and calculate (x&1)+((x>>1)&1) 
 * in every step (regard x as 2-bit integer) 
 */
```

### bang

>Compute !x without using !
>
>Examples: bang(3) = 0, bang(0) = 1
>
>Legal ops: ~ & ^ | + << >>
>
>Max ops: 12

```c
int bang(int x) {
	int if_zero = (x | (~x + 1)) >> 31;
	return (~if_zero) & 1;
}

/* ~0 + 1 = 0, ~x + x = 0xffffffff if x! = 0 */
```

### tmin

>return minimum two's complement integer 
>
>Legal ops: ! ~ & ^ | + << >>
>
>Max ops: 4

```
int tmin(void) {
	return 1 << 31;
}
```

### fitsBits

>return 1 if x can be represented as an
>
>n-bit, two's complement integer.
>
>1 <= n <= 32
>
>Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
>
>Legal ops: ! ~ & ^ | + << >>
>
>Max ops: 15

```c
int fitsBits(int x, int n) {
	int shift = 32 + (~n + 1);
	int change_big_endian = (x << shift) >> shift;
	int result = x ^ change_big_endian;
	return !result;
}

/* 
 * Exploit ability of shift to transfer the first 32-n digits into the value of the nth bit.
 * Use ^ and ! to know if transfered x and x are equal.
 * Equal means x is an n-bit.
 */
```

### divpwr2

>divpwr2 - Compute x/(2^n), for 0 <= n <= 30
>
>Round toward zero
>
>Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
>
>Legal ops: ! ~ & ^ | + << >>
>
>Max ops: 15

```c
int divpwr2(int x, int n) {
    int sign = x >> 31;
    int mask = (1 << n) + (~0);
    int bias = sign & mask;
    int result = (x + bias) >> n;
    return result;
}

/* Use bias to determine whether round up or round down */
```

### negate

> return -x 
>
> Example: negate(1) = -1.
>
> Legal ops: ! ~ & ^ | + << >>
>
> Max ops: 5

```c
int negate(int x) {
    return ~x + 1;
}
```

### isPositive

>return 1 if x > 0, return 0 otherwise 
>
>Example: isPositive(-1) = 0.
>
>Legal ops: ! ~ & ^ | + << >>
>
>Max ops: 8

```c
int isPositive(int x) {
    return !((x >> 31) | (!x));
}

 /* (x>>31) >0 if x<0, !x=1 if x=0 */ 
```

### isLessOrEqual

>if x <= y  then return 1, else return 
>
>Example: isLessOrEqual(4,5) = 1.
>
>Legal ops: ! ~ & ^ | + << >>
>
>Max ops: 24

```c
int isLessOrEqual(int x, int y) {
    int sign_x = (x >> 31) & 1;
    int sign_y = (y >> 31) & 1;
    int sign_differ = (sign_x ^ sign_y) & 1;
    int sign_same = !sign_differ;
    int sub_sign = (x + (~y)) >> 31;
    int result = (sub_sign & sign_same) | (~sign_y & sign_differ);
    return result;
}

/* 
 * Classify the discusstion according to whether the sign bits are same.
 * If same, compute x-y immediately.
 * If differ, check the sign bit of y.
 */
```

### ilog2

>return floor(log base 2 of x), where x > 0
>
>Example: ilog2(16) = 4
>
>Legal ops: ! ~ & ^ | + << >>
>
>Max ops: 90

```c
int ilog2(int x) {
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
 * Use dichotomy to find the maximum effective position of x.
 * Sum in every step. 
 */
```

### float_neg

>float_neg - Return bit-level equivalent of expression -f for
>
>floating point argument f.
>
>Both the argument and result are passed as unsigned int's, but
>
>they are to be interpreted as the bit-level representations of
>
>single-precision floating point values.
>
>When argument is NaN, return argument.
>
>Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
>
>Max ops: 10

```c
unsigned float_neg(unsigned uf) {
    unsigned abs = uf & 0x7fffffff;
    unsigned reverse = uf ^ 0x80000000;
    if(abs > 0x7f800000)
        return uf;
    return reverse;
}

/* Use abs to check NaN. If uf is NaN, return itself, else change the sign bit */
```

### float_i2f

>Return bit-level equivalent of expression (float) x
>
>Result is returned as unsigned int, but
>
>it is to be interpreted as the bit-level representation of a
>
>single-precision floating point values.
>
>Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
>
>Max ops: 30

```c
unsigned float_i2f(int x) {
    unsigned sign = 0;
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
 * First check if x is 0.
 * Then get ths absolute value and sign bit of x.
 * Normalize x and count the shifting.
 * Use the formula to get the float.
 * Finally round x;
 */
```

### float_twice

>Return bit-level equivalent of expression 2*f for
>
>floating point argument f.
>
>Both the argument and result are passed as unsigned int's, but
>
>they are to be interpreted as the bit-level representation of
>
>single-precision floating point values.
>
>When argument is NaN, return argument
>
>Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
>
>Max ops: 30

```c
unsigned float_twice(unsigned uf) {
    if(!(uf & 0x7f800000))
        return ((uf & 0x007fffff) << 1) | (uf & 0x80000000);
    else if((uf & 0x7f800000) ^ 0x7f800000)
        return uf + 0x00800000;
    return uf;
}

/* Classify the discussion into normalized value, denoramlized value, NaN and 0 */
```