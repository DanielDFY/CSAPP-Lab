# Architecture Lab

This lab is based on Y86-64 instruction set. Before we begin out tasks, build up our environment with the help of the given documnet.

* [Part A](#partA)
* [Part B](#partB)
* [Part C](#partC)

There are three parts in this lab. Part A is about how to translate C code into Y86-64 assembly codes. Part B is about how to add an instruction into `SEQ`. And in part C we will try to modify both the `PIPE` and a given Y86-64 program to improve their performance.

<h2 id = "partA">Part A</h2>

In this part we need to translate three C functions based on linked list into assembly codes under directory `sim/misc` . Here is the definition of the element struct `ELE` :

```c
/* linked list element */
typedef struct ELE {
    long val;
    struct ELE *next;
} *list_ptr;
```

* **sum_list**

  The first function calculates the sum of a given linked list.

  ```c
  /* sum_list - Sum the elements of a linked list */
  long sum_list(list_ptr ls)
  {
      long val = 0;
      while (ls) {
  				val += ls->val;
  				ls = ls->next;
      }
      return val;
  }
  ```

  To begin with, create a framework for the program:

  ```
  # Execution begins at address 0
          .pos    0
          irmovq  stack, %rsp     # Set up stack pointer
          call    main            # Execute main program
          halt                    # Terminate program
          
          ...			# Todo
          
  # Stack starts here and grows to lower addresses
          .pos 0x200
  stack: 
  ```

  Then add the given list:

  ```
  # Sample linked list
          .align  8
  ele1:
          .quad   0x00a
          .quad   ele2
  ele2:
          .quad   0x0b0
          .quad   ele3
  ele3:
          .quad   0xc00
          .quad   0
  ```

  Before writing the sum function, consider how to pass the list to the function. We can write the main program like this:

  ```
  main:
          pushq   %rdi
          irmovq  ele1, %rdi
          call    sumList
          popq    %rdi
          ret
  ```

  Because the function needs only one argument, we use register `rdi` to store the start adress of the list.

  Now let's deal with out target. As a callee called by the main function, first push the data in the registers we decide to use, and pop them back before we return.

  ```
  # sum_list - Sum the elements of a linked list
  # long sum_list(list_ptr ls)
  # ls in %rdi
  sumList:
          pushq   %rcx            # push data into stack for caller
          pushq   %r8
          irmovq  $8 , %r8        # constant 8, size of type 'long'
          xorq    %rax, %rax      # sum = 0
          jmp     test
  loop:
          mrmovq  (%rdi), %rcx    # ls -> val
          addq    %rcx, %rax      # add to sum
          addq    %r8, %rdi       # move to member after val
          mrmovq  (%rdi),  %rdi   # ls -> next
  test:
          andq    %rdi, %rdi      # set CC
          jne     loop            # stop when ls equals 0
          popq    %r8             # pop data from stack for caller
          popq    %rcx
          ret 
  ```

  Because Y86-64 doesn't support the addition between immediate value and register, the value has to be stored in a register first. In `ELE` , `val` is a `long` type member, so if we want to get the member `next` , the pointer should move 8 bytes

  , which is the space occupied by `val` in the struct.

  To compile our code, type instruction `./yas sum.ys` into the terminal. Run `./yis sum.yo` to get the result:

  ```
  Stopped in 36 steps at PC = 0x13.  Status 'HLT', CC Z=1 S=0 O=0
  Changes to registers:
  %rax:	0x0000000000000000	0x0000000000000cba
  %rsp:	0x0000000000000000	0x0000000000000200
  
  Changes to memory:
  0x01e8:	0x0000000000000000	0x000000000000005d
  0x01f8:	0x0000000000000000	0x0000000000000013
  ```

* **rsum_list**

  This is just the recursive version of `sum_list` . 

  ```c
  /* rsum_list - Recursive version of sum_list */
  long rsum_list(list_ptr ls)
  {
      if (!ls)
  	return 0;
      else {
  	long val = ls->val;
  	long rest = rsum_list(ls->next);
  	return val + rest;
      }
  }
  ```

  The only difference is that we have to modify the loop and test part:

  ```
  # rsum_list - Recursive version of sum_list
  # long rsum_list(list_ptr ls)
  # ls in %rdi
  rsumList:
          pushq   %rcx            # push data into stack for caller
          pushq   %r8
          irmovq  $8 , %r8        # constant 8, size of type 'long'
          xorq    %rax, %rax      # set return value to 0
          andq    %rdi, %rdi      # set CC
          je      end             # return when ls = 0
          mrmovq  (%rdi), %rcx    # ls -> val
          addq    %r8, %rdi       # move to member after val
          mrmovq  (%rdi), %rdi    # ls -> next
          call    rsumList        # recursion
          addq    %rcx, %rax      # return val + rest
  end:
          popq    %r8             # pop data from stack for caller
          popq    %rcx
          ret                     # return
  ```

  Then check the result with the same instructions:

  ```
  Stopped in 59 steps at PC = 0x13.  Status 'HLT', CC Z=0 S=0 O=0
  Changes to registers:
  %rax:	0x0000000000000000	0x0000000000000cba
  %rsp:	0x0000000000000000	0x0000000000000200
  
  Changes to memory:
  0x0190:	0x0000000000000000	0x0000000000000008
  0x0198:	0x0000000000000000	0x0000000000000c00
  0x01a0:	0x0000000000000000	0x000000000000009a
  0x01a8:	0x0000000000000000	0x0000000000000008
  0x01b0:	0x0000000000000000	0x00000000000000b0
  0x01b8:	0x0000000000000000	0x000000000000009a
  0x01c0:	0x0000000000000000	0x0000000000000008
  0x01c8:	0x0000000000000000	0x000000000000000a
  0x01d0:	0x0000000000000000	0x000000000000009a
  0x01e8:	0x0000000000000000	0x000000000000005d
  0x01f8:	0x0000000000000000	0x0000000000000013
  ```

* **copy_block**

  In this function, data in the source block will be copied into the destination block, and the return value is the xor checksum of all the data in the source block.

  ```c
  /* copy_block - Copy src to dest and return xor checksum of src */
  long copy_block(long *src, long *dest, long len)
  {
      long result = 0;
      while (len > 0) {
  	long val = *src++;
  	*dest++ = val;
  	result ^= val;
  	len--;
      }
      return result;
  }
  ```

  Now we modify the data part:

  ```
          .align 8
  # Source block
  src:
          .quad   0x00a
          .quad   0x0b0
          .quad   0xc00
  # Destination block
  dest:
          .quad   0x111
          .quad   0x222
          .quad   0x333
  ```

  Because  `copy_block` has three parameters, now we need three registers to pass the arguments.

  ```
  main:
          pushq   %rdi
          pushq   %rsi
          pushq   %rbp
          irmovq  src, %rdi
          irmovq  dest, %rsi
          irmovq  $3, %rbp
          call    copyBlock
          popq    %rbp
          popq    %rsi
          popq    %rdi
          ret
  ```

  And we can write the function like this:

  ```
  # copy_block - Copy src to dest and return xor checksum of src
  # long copy_block(long *src, long *dest, long len)
  # src in %rdi, dest in %rsi, len in %rbp
  copyBlock:
          pushq   %rcx            # push data into stack for caller
          pushq   %r8
          pushq   %r9
          irmovq  $8 , %r8        # constant 8, size of type 'long'
          irmovq  $1 , %r9        # constant 1
          xorq    %rax, %rax      # result = 0
          andq    %rbp, %rbp      # set CC
          jmp     test            # check len
  
  loop:
          mrmovq  (%rdi), %rcx    # val = *src
          addq    %r8, %rdi       # ++src
          rmmovq  %rcx, (%rsi)    # *dest = val
          addq    %r8, %rsi       # ++dest
          xorq    %rcx, %rax      # result ^= val
          subq    %r9, %rbp       # --len, set CC
  
  test:
          jg      loop            # stop when len <= 0
          popq    %r9             # pop data from stack for caller
          popq    %r8
          popq    %rcx
          ret   
  ```

  Finally check out result:

  ```
  Stopped in 48 steps at PC = 0x13.  Status 'HLT', CC Z=1 S=0 O=0
  Changes to registers:
  %rax:	0x0000000000000000	0x0000000000000cba
  %rsp:	0x0000000000000000	0x0000000000000200
  
  Changes to memory:
  0x0030:	0x0000000000000111	0x000000000000000a
  0x0038:	0x0000000000000222	0x00000000000000b0
  0x0040:	0x0000000000000333	0x0000000000000c00
  0x01d8:	0x0000000000000000	0x0000000000000075
  0x01f8:	0x0000000000000000	0x0000000000000013
  ```

<h2 id = "partB">Part B</h2>

In this part we will add instruction `iaddq` to the SEQ under directory `sim/seq` .

First consider the description of `iaddq` :

```
iaddq Description:
+------------+----------------------+
| Stage      | iaddq V, rB          |
+------------+----------------------+
| Fetch      | icode:ifun <- M1[PC] |
|            | rA:rB <- M1[PC + 1]  |
|            | valC <- M8[PC + 2]   |
|            | valP <- PC + 10      |
+------------+----------------------+
| Decode     | valB <- R[rB]        |
+------------+----------------------+
| Execute    | valE <- valB + valC  |
|            | Set CC               |
+------------+----------------------+
| Memory     | 	                    |
+------------+----------------------+
| Write back | R[rB] <- valE        |
+------------+----------------------+
| PC update  | PC <- valP           |
+-----------------------------------+
```

According to this description, we only need to add `IIADDQ` into proper set for the signals.

```
bool instr_valid = icode in 
	{ INOP, IHALT, IRRMOVQ, IIRMOVQ, IRMMOVQ, IMRMOVQ,
	       IOPQ, IJXX, ICALL, IRET, IPUSHQ, IPOPQ, IIADDQ };
	       
# Does fetched instruction require a regid byte?
bool need_regids =
	icode in { IRRMOVQ, IOPQ, IPUSHQ, IPOPQ, 
		     IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ };
		   
# Does fetched instruction require a constant word?
bool need_valC =
	icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL, IIADDQ };
	
...

## What register should be used as the B source?
word srcB = [
	icode in { IOPQ, IRMMOVQ, IMRMOVQ, IIADDQ } : rB;
	icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't need register
];

## What register should be used as the E destination?
word dstE = [
	icode in { IRRMOVQ } && Cnd : rB;
	icode in { IIRMOVQ, IOPQ, IIADDQ} : rB;
	icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't write any register
];

...

## Select input A to ALU
word aluA = [
	icode in { IRRMOVQ, IOPQ } : valA;
	icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ } : valC;
	icode in { ICALL, IPUSHQ } : -8;
	icode in { IRET, IPOPQ } : 8;
	# Other instructions don't need ALU
];

## Select input B to ALU
word aluB = [
	icode in { IRMMOVQ, IMRMOVQ, IOPQ, ICALL, 
		      IPUSHQ, IRET, IPOPQ, IIADDQ } : valB;
	icode in { IRRMOVQ, IIRMOVQ } : 0;
	# Other instructions don't need ALU
];

## Should the condition codes be updated?
bool set_cc = icode in { IOPQ, IIADDQ };
```

To check our answer, type `./ssim -t ../y86-code/asumi.yo` , `(cd ../y86-code; make testssim)` and `(cd ../ptest; make SIM=../seq/ssim)` . All three tests should give `ISA Check Succeeds`.

<h2 id = "partC">Part C</h2>

We will be working in directory `sim/pipe`  to modify `ncopy.ys` and `pipe-full.hcl` with the goal of making `ncopy.ys` run as fast as possible.

According to the document, each time we modify the ncopy.ys program, we can rebuild the driver programs by typing `make drivers`. Each time we modify the `pipe-full.hcl` file, we can rebuild the simulator by typing `make psim VERSION=full`. We can also use `make VERSION=full` under `sim` to rebuild the whole environment.

To test our correctness, use `./psim -t sdriver.yo` and `./psim -t ldriver.yo` to check `pipe-full.hcl` . Use `./correctness.pl` to check `ncopy.ys` and use `./benchmark.pl` to test the performance.

Note that `ncopy.yo` which we can get as in Part A is limited to be no more than 1000 bytes long. `./check-len.pl < ncopy.yo` can help us to check the length.

In this implementation, the modification to `pipe-full.hcl` is nearly the same as what we did in Part B. Instruction `iaddq` is added to the pipeline. Our focus is on `ncopy.ys`.

```
# You can modify this portion
		# Loop header
		xorq		%rax,%rax		# count = 0;
		andq 		%rdx,%rdx		# len <= 0?
		jle 		Done			# if so, goto Done:
Loop:	
		mrmovq 		(%rdi), %r10		# read val from src...
		rmmovq 		%r10, (%rsi)		# ...and store it to dst
		andq 		%r10, %r10		# val <= 0?
		jle 		Npos			# if so, goto Npos:
		irmovq 		$1, %r10
		addq 		%r10, %rax		# count++
Npos:	
		irmovq 		$1, %r10
		subq 		%r10, %rdx		# len--
		irmovq 		$8, %r10
		addq 		%r10, %rdi		# src++
		addq 		%r10, %rsi		# dst++
		andq 		%rdx,%rdx		# len > 0?
		jg 		Loop			# if so, goto Loop:
```

It's easy to know that we don't need `xorq %rax,%rax` because the register is initialized to store 0. And we can use `iaddq` instruction we added into `PIPE` to save instructions. Solving hazards by reordering also helps. Then we may get following code:

```
# You can modify this portion
		# Loop header
		andq 		%rdx,%rdx		# len > 0 ?
		jmp 		Test			# goto Test:
Loop:	
		mrmovq 		(%rdi), %r10		# read val from src...
		iaddq		$8, %rdi		# src++
		rmmovq 		%r10, (%rsi)		# ...and store it to dst
		andq 		%r10, %r10		# val <= 0 ?
		jle 		Npos			# if so, goto Npos:
		iaddq 		$1, %rax		# count++
Npos:	
		iaddq		$8, %rsi		# drc++
		iaddq		$-1, %rdx		# len--, len > 0 ?
Test:
		jg 		Loop			# if so, goto Loop:
```

But all these effors are too powerless to get us out of 0 points.

To improve the performance, we can try to expand the loop to reduce the cost on branch prediction. For example:

```
Loop1:
		mrmovq		(%rdi), %r8		# read val from src...
		mrmovq		8(%rdi), %r9		# read next val from src...
		rmmovq		%r8, (%rsi)		# store val to dst
		andq		%r8, %r8		# val <= 0 ?
		jle		Loop2			# if so, goto Loop2:
		iaddq		$1, %rax		# count++
Loop2:
		mrmovq		16(%rdi), %r8		# read val from src...
		rmmovq		%r9, 8(%rsi)		# store val to dst
		andq		%r9, %r9		# val <= 0 ?
		jle		Loop3			# if so, goto Loop3:
		iaddq		$1, %rax		# count++

...

Loop:	
		iaddq		$8 * x, %rdi		# src += 8 * x for x loop expanded
		iaddq		$8 * x, %rsi		# dst += 8 * x for x loop expanded
		iaddq		$-x, %rdx		# len >= x ?
		jge		Loop1			# if so, goto Loop1
```

The more we expand, we may expect higher performance. But the degree we can expand the loop is limited by the 1000-byte requirement.

Before we determine the degree x, consider the case when the remain values is less than x. We can simply keep using loop to solve it, but it's still expensive. To figure out the remainder faster, we can use binary search. And we can organize the expanded loop for remainder in a descending order. Every time we get the remainder, the program will jump to the proper position and go through the instructions to the end.

However, if we simply use descending order, the bubbles created by hazard can not be removed like what we did in the loop expanded above. Something still need to be inserted between `mrmovq` and `rmmovq` instruction.

Reconsider the process to determine the remainder. If we can make use of the CC got from the last comparision in the binary tree, the two instruction used to increase `count` may be useful.

When the program jumps from the binary tree to the expanded loop, if the comparision always less or equal than 0, then `jle` can be used to simultaneously check both `val` and whether this loop part is just the position program jumps to. With this idea, the binary tree should be carefully constructed.

Finally the structure of the binary tree can be designed as follows:

```
	   3
         /   \
        /     \
       2       6
      / \     / \
     1   0   /   \
            5     8
           /     / \
          4     7   9
# binary tree for remainder check
Root:
		iaddq		$7, %rdx		# len - 3 = ?
		jl		L			# len < 3, goto left child
		jg		R			# len > 3, goto right child
		je		R3			# len = 3, goto remainder 3
L:								
		iaddq		$2, %rdx		# len - 1 = ?
		je		R1			# len = 1, goto remainder 1
		iaddq		$-1, %rdx		# len - 2 = ？
		je		R2			# len = 2, goto remainder 2
		ret					# len = 0, return
R:
		iaddq		$-3, %rdx		# len - 6 = ?
		jg		RR			# len > 6, goto right child
		je		R6			# len = 6, goto remainder 6
RL:
		iaddq		$1, %rdx		# len - 5 = ?
		je		R5			# len = 5, goto remainder 5
		jmp		R4			# len = 4, goto remainder 4
RR:
		iaddq		$-2, %rdx		# len - 8 = ?
		je		R8			# len = 8, goto remainder 8
		jl		R7			# len = 7, goto remainder 7
R9:
	...
```

We can have degree of 10 under the 1000-byte limitation. Notice that when get 0, we can return immediately, and when get 9 we can enter the expanded loop immediately. Therefore, every time the program jumps out of the tree, `jle` will be guaranteed to be true.

Thus the descending expanded loop can remove nearly all the bubbles created by hazard:

```
R9:
		mrmovq 		64(%rdi), %r10		# read val from src...
		rmmovq 		%r10, 64(%rsi)		# can not remove bubble here
		andq 		%r10, %r10		# val <= 0 ?
R8:
		mrmovq 		56(%rdi), %r10		# read val from src...
		jle		R8Npos			# if val <= 0, gotp R8Npos:
		iaddq		$1, %rax		# count++
R8Npos:
		rmmovq 		%r10, 56(%rsi)
		andq 		%r10, %r10		# val <= 0 ?
R7:
		mrmovq 		48(%rdi), %r10		# read val from src...
		jle		R7Npos			# if val <= 0, gotp R7Npos:
		iaddq		$1, %rax		# count++
R7Npos:
		rmmovq 		%r10, 48(%rsi)
		andq 		%r10, %r10		# val <= 0 ?
R6:
		mrmovq 		40(%rdi), %r10		# read val from src...
		jle		R6Npos			# if val <= 0, gotp R6Npos:
		iaddq		$1, %rax		# count++
R6Npos:
		rmmovq 		%r10, 40(%rsi)
		andq 		%r10, %r10		# val <= 0 ?
R5:
		mrmovq 		32(%rdi), %r10		# read val from src...
		jle		R5Npos			# if val <= 0, gotp R5Npos:
		iaddq		$1, %rax		# count++
R5Npos:
		rmmovq 		%r10, 32(%rsi)
		andq 		%r10, %r10		# val <= 0 ?
R4:
		mrmovq 		24(%rdi), %r10		# read val from src...
		jle		R4Npos			# if val <= 0, gotp R4Npos:
		iaddq		$1, %rax		# count++
R4Npos:
		rmmovq 		%r10, 24(%rsi)
		andq 		%r10, %r10		# val <= 0 ?
R3:
		mrmovq 		16(%rdi), %r10		# read val from src...
		jle		R3Npos			# if val <= 0, gotp R3Npos:
		iaddq		$1, %rax		# count++
R3Npos:
		rmmovq 		%r10, 16(%rsi)
		andq 		%r10, %r10		# val <= 0 ?
R2:
		mrmovq 		8(%rdi), %r10		# read val from src...
		jle		R2Npos			# if val <= 0, gotp R2Npos:
		iaddq		$1, %rax		# count++
R2Npos:
		rmmovq 		%r10, 8(%rsi)
		andq 		%r10, %r10		# val <= 0 ?
R1:
		mrmovq 		(%rdi), %r10		# read val from src...
		jle		R1Npos			# if val <= 0, gotp R1Npos:
		iaddq		$1, %rax		# count++
R1Npos:
		rmmovq 		%r10, (%rsi)
		andq 		%r10, %r10		# val <= 0 ?
		jle		Done			# if so, return
		iaddq		$1, %rax		# count++	
```

Now use the instructions to rebuild and test our program:

```
ncopy.yo length = 998 bytes

68/68 pass correctness test

Average CPE	7.49
Score		60.0/60.0
```
