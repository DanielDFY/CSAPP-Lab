# Bomb Lab

## Table of contents

* [File Introduction](#file)
* [Commands](#commands)
* [Part I: Normal Phases](#part1)
    * [phase 1](#phase1)
    * [phase 2](#phase2)
    * [phase 3](#phase3)
    * [phase 4](#phase4)
    * [phase 5](#phase5)
    * [phase 6](#phase6)

<h2 id = "file">File introduction</h2>

README.txt: A descriptive file

bomb: An executable program. If input strings are incorrect, the program will "explode!"

bomb.c: The source code of the bomb.


<h2 id = "commands">Commands</h2>

#### Using GDB

```
unix> gdb bomb                    # use gdb to run bomb
(gdb) break *<addr>               # set break point
(gdb) run                         # run the program 
(gdb) run < input.txt             # run the program with parameters in the file
(gdb) disas                       # check the current assembly code
(gdb) print <variable>            # print the value of the variable
(gdb) info registers              # check the value of the registers
(gdb) stepi                       # run the next assembly code
(gdb) x/s <addr>                  # print the value stored at the address as string
(gdb) set args <file>             # read arguments from the file
```

More usages can be found in [gdb-ref](./gdb-ref.md)

#### Generating byte codes

```shell
unix> gcc -c example.s

unix> objdump -d example.o
```

#### Decompile

```
objdump -d bomb > bomb.txt
```


<h2 id = "part1">Part I: Normal Phases</h2>

First we can decompile the executable program.

```shell
objdump -d bomb > bomb.txt
```

In `bomb.txt` we can find `<main>`. We can see there are 6 phases in the main code. Each phase requires us to input  something to solve. If the input string is incorrect, the program will jump to `<explode_bomb>` and explode the bomb. Now lets solve these phases respectively.

<h3 id = "phase1">Phase 1</h3>

The first phase is about string comparision. We are supposed to enter the GDB mode first.

```shell
unix> gdb bomb
```

To find phase 1 and prevent the bomb explotion, we need to set break points before we run the file.

```shell
(gdb) break explode_bomb
(gdb) break phase_1
```

Then run the program

```shell
(gdb) run
```

Now the program comes to phase_1 and require us to input the string. Because we have set the break points, we can type some random words and use gdb instructions to find clues.

```
abc

Breakpoint 2, 0x0000000000400ee0 in phase_1()
```

Now we are at phase 1 (The breakpoint we set has paused the program).  We can use `disas` to check the assembly code.

```
(gdb) disas
Dump of assembler code for function phase_1:
=> 0x0000000000400ee0 <+0>:		sub    $0x8,%rsp
   0x0000000000400ee4 <+4>:		mov    $0x402400,%esi
   0x0000000000400ee9 <+9>:		callq  0x401338 <strings_not_equal>
   0x0000000000400eee <+14>:	        test   %eax,%eax
   0x0000000000400ef0 <+16>:	        je     0x400ef7 <phase_1+23>
   0x0000000000400ef2 <+18>:	        callq  0x40143a <explode_bomb>
   0x0000000000400ef7 <+23>:	        add    $0x8,%rsp
   0x0000000000400efb <+27>:	        retq   
End of assembler dump.
```

 (Also you can look up for the related code in `bomb.txt` we created just now)

The program is at the line  `<+0>`. According to the codes, we can know that `callq` will check if the value stored at the first and second parameter are identical. The second parameter is stored in %esi, and the address is `0x402400`. The first parameter is stored in %eax. We can use `x/s` to dereference this address.

```
(gdb) x/s $eax
0x603780 <input_strings>:		"abc"
```

This is what we just input. Therefore, we know that if the string we input is the same as the string stored at address `0x402400`, the bomb will not explode. Let's find out the answer.

```
(gdb) x/s 0x402400
0x402400:						"Border relations with Canada have never been better."
```

Now we get the answer to phase 1. Type `quit` to exit GDB, then creat a file `touch input.txt` and put our first answer at the first line. Check our answer by these commands:

```
(gdb) break explode_bomb
(gdb) break phase_2
(gdb) set args ./input.txt
(gdb) run
```

If you see words as follows, congratuations! The first phase has been solved!

```
Phase 1 defused. How about the next one?
```

<h3 id = "phase2">Phase 2</h3>

The second phase is about loops. As phase 1, we first type some random words. Remember to set proper break points before `run`.

```
abc

Breakpoint 2, 0x0000000000400efc in phase_2 ()
(gdb) disas
Dump of assembler code for function phase_2:
=> 0x0000000000400efc <+0>:		push   %rbp
   0x0000000000400efd <+1>:		push   %rbx
   0x0000000000400efe <+2>:		sub    $0x28,%rsp
   0x0000000000400f02 <+6>:		mov    %rsp,%rsi
   0x0000000000400f05 <+9>:		callq  0x40145c <read_six_numbers>
   0x0000000000400f0a <+14>:	        cmpl   $0x1,(%rsp)
   0x0000000000400f0e <+18>:	        je     0x400f30 <phase_2+52>
   0x0000000000400f10 <+20>:	        callq  0x40143a <explode_bomb>
   0x0000000000400f15 <+25>:	        jmp    0x400f30 <phase_2+52>
   0x0000000000400f17 <+27>:	        mov    -0x4(%rbx),%eax
   0x0000000000400f1a <+30>:	        add    %eax,%eax
   0x0000000000400f1c <+32>:	        cmp    %eax,(%rbx)
   0x0000000000400f1e <+34>:	        je     0x400f25 <phase_2+41>
   0x0000000000400f20 <+36>:	        callq  0x40143a <explode_bomb>
   0x0000000000400f25 <+41>:	        add    $0x4,%rbx
   0x0000000000400f29 <+45>:	        cmp    %rbp,%rbx
   0x0000000000400f2c <+48>:	        jne    0x400f17 <phase_2+27>
   0x0000000000400f2e <+50>:	        jmp    0x400f3c <phase_2+64>
   0x0000000000400f30 <+52>:	        lea    0x4(%rsp),%rbx
   0x0000000000400f35 <+57>:	        lea    0x18(%rsp),%rbp
   0x0000000000400f3a <+62>:	        jmp    0x400f17 <phase_2+27>
   0x0000000000400f3c <+64>:	        add    $0x28,%rsp
   0x0000000000400f40 <+68>:	        pop    %rbx
   0x0000000000400f41 <+69>:	        pop    %rbp
   0x0000000000400f42 <+70>:	        retq   
End of assembler dump.
```

Line `<+9>` indicates that we need to input 6 numbers. From line `<+14>`  and `<+18>` we know that the first numbe is 1. Then the program jumps to line `<+52>` , it puts the second number into %rbx and the last number into %rbp. Line `<+27>` to line `<+50>` tell us the program checks if the current number is twice as the last number. After checking the last number, the function returns. So the answer should be 1 2 4 8 16 32. Let's check:

```
(gdb) break explode_bomb
(gdb) break phase_3
(gdb) set args ./input.txt             # The second answer has been added
(gdb) run
```

```
That's number 2. Keep going!
```

Now we can go to phase 3.


<h3 id = "phase3">Phase 3</h3>

The third phase is about conditionals/switches. Type some words and use `disas`. Remember to set the break points before `run`.

```
abc

Breakpoint 2, 0x0000000000400f43 in phase_3 ()
(gdb) disas
Dump of assembler code for function phase_3:
=> 0x0000000000400f43 <+0>:		sub    $0x18,%rsp
   0x0000000000400f47 <+4>:		lea    0xc(%rsp),%rcx
   0x0000000000400f4c <+9>:		lea    0x8(%rsp),%rdx
   0x0000000000400f51 <+14>:	        mov    $0x4025cf,%esi
   0x0000000000400f56 <+19>:	        mov    $0x0,%eax
   0x0000000000400f5b <+24>:	        callq  0x400bf0 <__isoc99_sscanf@plt>
   0x0000000000400f60 <+29>:	        cmp    $0x1,%eax
   0x0000000000400f63 <+32>:	        jg     0x400f6a <phase_3+39>
   0x0000000000400f65 <+34>:	        callq  0x40143a <explode_bomb>
   0x0000000000400f6a <+39>:	        cmpl   $0x7,0x8(%rsp)
   0x0000000000400f6f <+44>:	        ja     0x400fad <phase_3+106>
   0x0000000000400f71 <+46>:	        mov    0x8(%rsp),%eax
   0x0000000000400f75 <+50>:	        jmpq   *0x402470(,%rax,8)
   0x0000000000400f7c <+57>:	        mov    $0xcf,%eax
   0x0000000000400f81 <+62>:	        jmp    0x400fbe <phase_3+123>
   0x0000000000400f83 <+64>:	        mov    $0x2c3,%eax
   0x0000000000400f88 <+69>:	        jmp    0x400fbe <phase_3+123>
   0x0000000000400f8a <+71>:	        mov    $0x100,%eax
   0x0000000000400f8f <+76>:	        jmp    0x400fbe <phase_3+123>
   0x0000000000400f91 <+78>:	        mov    $0x185,%eax
   0x0000000000400f96 <+83>:	        jmp    0x400fbe <phase_3+123>
   0x0000000000400f98 <+85>:	        mov    $0xce,%eax
   0x0000000000400f9d <+90>:	        jmp    0x400fbe <phase_3+123>
   0x0000000000400f9f <+92>:	        mov    $0x2aa,%eax
   0x0000000000400fa4 <+97>:	        jmp    0x400fbe <phase_3+123>
   0x0000000000400fa6 <+99>:	        mov    $0x147,%eax
   0x0000000000400fab <+104>:	        jmp    0x400fbe <phase_3+123>
   0x0000000000400fad <+106>:	        callq  0x40143a <explode_bomb>
   0x0000000000400fb2 <+111>:	        mov    $0x0,%eax
   0x0000000000400fb7 <+116>:	        jmp    0x400fbe <phase_3+123>
   0x0000000000400fb9 <+118>:	        mov    $0x137,%eax
   0x0000000000400fbe <+123>:	        cmp    0xc(%rsp),%eax
   0x0000000000400fc2 <+127>:	        je     0x400fc9 <phase_3+134>
   0x0000000000400fc4 <+129>:	        callq  0x40143a <explode_bomb>
   0x0000000000400fc9 <+134>:	        add    $0x18,%rsp
   0x0000000000400fcd <+138>:	        retq   
End of assembler dump.
```

We notice that in line `<+14>` there is a abrupt address 0x4025cf. Dereference this address:

```
(gdb) x/s 0x4025cf
0x4025cf: 						"%d %d"
```

So propably in this phase we should input 2 intergers. The first number is stored at (%rsp + 0x8), the second is at (%rsp + 0xc). From line `<+29>`  to line `<+50>` we know that this construction is a swith. The first number is used to decide the case and it must between 0 to 7. `jmpq   *0x402470(,%rax,8)` means jump to address `0x402470 + %rax × 8`, so `0x402470` must be where the jump table is. This switch has 8 cases, so there are 8 addresses in the table. Use `x/8x <address>` to print 8 addresses in hexadecimal format.

```
(gdb) x/8x 0x402470
0x402470:	0x0000000000400f7c	0x0000000000400fb9
0x402480:	0x0000000000400f83	0x0000000000400f8a
0x402490:	0x0000000000400f91	0x0000000000400f98
0x4024a0:	0x0000000000400f9f	0x0000000000400fa6
```

Compared to the code at these address, under different cases, different values are stored in %eax and compared with 0xc(%rsp), which is the second argument, in line `<+123>`. If they are identical, the bomb will not explode. So the answer can be one of these 8 pairs:

`0 207` `1 311` `2 707` `3 256` `4 389` `5 206` `6 682` `7 327`.

Check our answers:

```
Halfway there!
```

Now we can go to phase 4.




<h3 id = "phase4">Phase 4</h3>

The forth phase is about recursive calls and the stack discipline. Remember to set the break points.

```
abc

Breakpoint 1, 0x000000000040100c in phase_4 ()
(gdb) disas
Dump of assembler code for function phase_4:
=> 0x000000000040100c <+0>:	sub    $0x18,%rsp
   0x0000000000401010 <+4>:	lea    0xc(%rsp),%rcx
   0x0000000000401015 <+9>:	lea    0x8(%rsp),%rdx
   0x000000000040101a <+14>:	mov    $0x4025cf,%esi
   0x000000000040101f <+19>:	mov    $0x0,%eax
   0x0000000000401024 <+24>:	callq  0x400bf0 <__isoc99_sscanf@plt>
   0x0000000000401029 <+29>:	cmp    $0x2,%eax
   0x000000000040102c <+32>:	jne    0x401035 <phase_4+41>
   0x000000000040102e <+34>:	cmpl   $0xe,0x8(%rsp)
   0x0000000000401033 <+39>:	jbe    0x40103a <phase_4+46>
   0x0000000000401035 <+41>:	callq  0x40143a <explode_bomb>
   0x000000000040103a <+46>:	mov    $0xe,%edx
   0x000000000040103f <+51>:	mov    $0x0,%esi
   0x0000000000401044 <+56>:	mov    0x8(%rsp),%edi
   0x0000000000401048 <+60>:	callq  0x400fce <func4>
   0x000000000040104d <+65>:	test   %eax,%eax
   0x000000000040104f <+67>:	jne    0x401058 <phase_4+76>
   0x0000000000401051 <+69>:	cmpl   $0x0,0xc(%rsp)
   0x0000000000401056 <+74>:	je     0x40105d <phase_4+81>
   0x0000000000401058 <+76>:	callq  0x40143a <explode_bomb>
   0x000000000040105d <+81>:	add    $0x18,%rsp
   0x0000000000401061 <+85>:	retq   
End of assembler dump.
```

Similar to phase 3, first we can check the address 0x4025cf at line `<+14>`.

```
(gdb) x/s 0x4025cf
0x4025cf:						"%d %d"
```

Together with line `<+29>` and line `<+32>` we know the program requires 2 integers. Then the program set the first number and 0 as two arguments and call func4. Let's look up for func4 in the file we created, which is bomb.txt.

```
0000000000400fce <func4>:
  400fce:	48 83 ec 08          	sub    $0x8,%rsp
  400fd2:	89 d0                	mov    %edx,%eax
  400fd4:	29 f0                	sub    %esi,%eax
  400fd6:	89 c1                	mov    %eax,%ecx
  400fd8:	c1 e9 1f             	shr    $0x1f,%ecx
  400fdb:	01 c8                	add    %ecx,%eax
  400fdd:	d1 f8                	sar    %eax
  400fdf:	8d 0c 30             	lea    (%rax,%rsi,1),%ecx
  400fe2:	39 f9                	cmp    %edi,%ecx
  400fe4:	7e 0c                	jle    400ff2 <func4+0x24>
  400fe6:	8d 51 ff             	lea    -0x1(%rcx),%edx
  400fe9:	e8 e0 ff ff ff       	callq  400fce <func4>
  400fee:	01 c0                	add    %eax,%eax
  400ff0:	eb 15                	jmp    401007 <func4+0x39>
  400ff2:	b8 00 00 00 00       	mov    $0x0,%eax
  400ff7:	39 f9                	cmp    %edi,%ecx
  400ff9:	7d 0c                	jge    401007 <func4+0x39>
  400ffb:	8d 71 01             	lea    0x1(%rcx),%esi
  400ffe:	e8 cb ff ff ff       	callq  400fce <func4>
  401003:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax
  401007:	48 83 c4 08          	add    $0x8,%rsp
  40100b:	c3                   	retq   
```

After analyze this function, we can know that it's equivalent to codes in C format as follows:

```C
int func4(int n1, int n2) {
    int temp = (31/(pow(2, n1)) + n1)/2 + n2;
    if (n1 > temp) {
        return 2*func4(n1 - 1, n2);
    }
    else if (n1 < temp) {
        return 2*func4(n1 + 1, n2) + 1;
    }
    else {
        return 0;
    }
}
```

Line `<+65>` requires the return value of func4 is 0, so we can figure out that the first number we should input is 3. Line `<+65>` suggests that the second number should be 0. Therefore, our answer can be `3 0`. Now check the answer:

```
So you got that one. Try this one.
```

Let's go to phase 5.



<h3 id = "phase5">Phase 5</h3>

The fifth phase is about pointers. As usual first set break points before `run` and then use `disas`. 

```
abc

Breakpoint 2, 0x0000000000401062 in phase_5 ()
(gdb) disas
Dump of assembler code for function phase_5:
=> 0x0000000000401062 <+0>:	push   %rbx
   0x0000000000401063 <+1>:	sub    $0x20,%rsp
   0x0000000000401067 <+5>:	mov    %rdi,%rbx
   0x000000000040106a <+8>:	mov    %fs:0x28,%rax
   0x0000000000401073 <+17>:	mov    %rax,0x18(%rsp)
   0x0000000000401078 <+22>:	xor    %eax,%eax
   0x000000000040107a <+24>:	callq  0x40131b <string_length>
   0x000000000040107f <+29>:	cmp    $0x6,%eax
   0x0000000000401082 <+32>:	je     0x4010d2 <phase_5+112>
   0x0000000000401084 <+34>:	callq  0x40143a <explode_bomb>
   0x0000000000401089 <+39>:	jmp    0x4010d2 <phase_5+112>
   0x000000000040108b <+41>:	movzbl (%rbx,%rax,1),%ecx
   0x000000000040108f <+45>:	mov    %cl,(%rsp)
   0x0000000000401092 <+48>:	mov    (%rsp),%rdx
   0x0000000000401096 <+52>:	and    $0xf,%edx
   0x0000000000401099 <+55>:	movzbl 0x4024b0(%rdx),%edx
   0x00000000004010a0 <+62>:	mov    %dl,0x10(%rsp,%rax,1)
   0x00000000004010a4 <+66>:	add    $0x1,%rax
   0x00000000004010a8 <+70>:	cmp    $0x6,%rax
   0x00000000004010ac <+74>:	jne    0x40108b <phase_5+41>
   0x00000000004010ae <+76>:	movb   $0x0,0x16(%rsp)
   0x00000000004010b3 <+81>:	mov    $0x40245e,%esi
   0x00000000004010b8 <+86>:	lea    0x10(%rsp),%rdi
   0x00000000004010bd <+91>:	callq  0x401338 <strings_not_equal>
   0x00000000004010c2 <+96>:	test   %eax,%eax
   0x00000000004010c4 <+98>:	je     0x4010d9 <phase_5+119>
   0x00000000004010c6 <+100>:	callq  0x40143a <explode_bomb>
   0x00000000004010cb <+105>:	nopl   0x0(%rax,%rax,1)
   0x00000000004010d0 <+110>:	jmp    0x4010d9 <phase_5+119>
   0x00000000004010d2 <+112>:	mov    $0x0,%eax
   0x00000000004010d7 <+117>:	jmp    0x40108b <phase_5+41>
   0x00000000004010d9 <+119>:	mov    0x18(%rsp),%rax
   0x00000000004010de <+124>:	xor    %fs:0x28,%rax
   0x00000000004010e7 <+133>:	je     0x4010ee <phase_5+140>
   0x00000000004010e9 <+135>:	callq  0x400b30 <__stack_chk_fail@plt>
   0x00000000004010ee <+140>:	add    $0x20,%rsp
   0x00000000004010f2 <+144>:	pop    %rbx
   0x00000000004010f3 <+145>:	retq   
End of assembler dump.
```

The program requires a string with length of 6 (line `<+29>` ). Line `<+52>` gets the last 4 bits of each char in the string and line `<+55>` uses it as an index to get a char from address  `0x4024b0`. Because 4 bits can be used to repesent 16 cases, we use `x/16c 0x4024b0` to figure out the content:

```
(gdb) x/16d 0x4024b0
0x4024b0 <array.3449>:	109 'm'	97 'a'	100 'd'	117 'u'	105 'i'	101 'e'	114 'r'	115 's'
0x4024b8 <array.3449+8>:	110 'n'	102 'f'	111 'o'	116 't'	118 'v'	98 'b'	121 'y'	108 'l'
```

During the loop from line `<+41>` to line `<+74>`, each char is sequentially stored in the stack. Then the whole string in the stack is compared with the string stored at address `0x40245e`.

```
(gdb) x/6c 0x40245e
0x40245e:	102 'f'	108 'l'	121 'y'	101 'e'	114 'r'	115 's'
```

Each char in the string will be the offset. According to the code the offsets should be `9 15 14 5 6 7`, and the corresponding letters are `i o n e f g` (Only count the last 4 digits of their binary values). So the answer should be `ionefg`. Check the result:

```
Good work! On the next...
```



<h3 id = "phase6">Phase 6</h3>

The sixth phase is about linked lists/pointers/structs. Let's read the codes part by part.

```
abc

Breakpoint 2, 0x00000000004010f4 in phase_6 ()
(gdb) disas
Dump of assembler code for function phase_6:
=> 0x00000000004010f4 <+0>:	push   %r14
   0x00000000004010f6 <+2>:	push   %r13
   0x00000000004010f8 <+4>:	push   %r12
   0x00000000004010fa <+6>:	push   %rbp
   0x00000000004010fb <+7>:	push   %rbx
   0x00000000004010fc <+8>:	sub    $0x50,%rsp
   0x0000000000401100 <+12>:	mov    %rsp,%r13
   0x0000000000401103 <+15>:	mov    %rsp,%rsi
   0x0000000000401106 <+18>:	callq  0x40145c <read_six_numbers>
```

&uarr;Line `<+18>` indicates that we need to input 6 numbers.

```
   0x000000000040110b <+23>:	mov    %rsp,%r14
   0x000000000040110e <+26>:	mov    $0x0,%r12d
   0x0000000000401114 <+32>:	mov    %r13,%rbp
   0x0000000000401117 <+35>:	mov    0x0(%r13),%eax
   0x000000000040111b <+39>:	sub    $0x1,%eax
   0x000000000040111e <+42>:	cmp    $0x5,%eax
   0x0000000000401121 <+45>:	jbe    0x401128 <phase_6+52>		# every number is <= 6
   0x0000000000401123 <+47>:	callq  0x40143a <explode_bomb>
```

&uarr;From line `<+26>` to line `<+47>`, the program exams if all numbers we input are <= 6. 

```
0x0000000000401128 <+52>:	add    $0x1,%r12d
   0x000000000040112c <+56>:	cmp    $0x6,%r12d
   0x0000000000401130 <+60>:	je     0x401153 <phase_6+95>
   0x0000000000401132 <+62>:	mov    %r12d,%ebx
   0x0000000000401135 <+65>:	movslq %ebx,%rax
   0x0000000000401138 <+68>:	mov    (%rsp,%rax,4),%eax
   0x000000000040113b <+71>:	cmp    %eax,0x0(%rbp)				# if the number == 0
   0x000000000040113e <+74>:	jne    0x401145 <phase_6+81>
   0x0000000000401140 <+76>:	callq  0x40143a <explode_bomb>
   0x0000000000401145 <+81>:	add    $0x1,%ebx
   0x0000000000401148 <+84>:	cmp    $0x5,%ebx
   0x000000000040114b <+87>:	jle    0x401135 <phase_6+65>
   0x000000000040114d <+89>:	add    $0x4,%r13
   0x0000000000401151 <+93>:	jmp    0x401114 <phase_6+32>
```

&uarr;From line `<+52>` to line `<+87>` is a loop which use `r12d` and `ebx` as counters to check if every number is not 0 (checked at line `<+71>`, using `info register` and `x/d` we can know that the value at the address stored in `rbp` is 0). 

```
   0x0000000000401153 <+95>:	lea    0x18(%rsp),%rsi
   0x0000000000401158 <+100>:	mov    %r14,%rax
   0x000000000040115b <+103>:	mov    $0x7,%ecx
   0x0000000000401160 <+108>:	mov    %ecx,%edx
   0x0000000000401162 <+110>:	sub    (%rax),%edx
   0x0000000000401164 <+112>:	mov    %edx,(%rax)					# input[i] = 7 - input[i]
   0x0000000000401166 <+114>:	add    $0x4,%rax					# next number
   0x000000000040116a <+118>:	cmp    %rsi,%rax					# if overflow
   0x000000000040116d <+121>:	jne    0x401160 <phase_6+108>
```

&uarr;From line `<+95>` to line `<+121>`, the program changed each input number like this: `input[i] = 7 - input[i]`. 

```
   0x000000000040116f <+123>:	mov    $0x0,%esi
   0x0000000000401174 <+128>:	jmp    0x401197 <phase_6+163>
   0x0000000000401176 <+130>:	mov    0x8(%rdx),%rdx				# fine the `exc`th node
   0x000000000040117a <+134>:	add    $0x1,%eax
   0x000000000040117d <+137>:	cmp    %ecx,%eax
   0x000000000040117f <+139>:	jne    0x401176 <phase_6+130>
   0x0000000000401181 <+141>:	jmp    0x401188 <phase_6+148>
   0x0000000000401183 <+143>:	mov    $0x6032d0,%edx
   0x0000000000401188 <+148>:	mov    %rdx,0x20(%rsp,%rsi,2)		# put the node at specified position
   0x000000000040118d <+153>:	add    $0x4,%rsi					# next input number
   0x0000000000401191 <+157>:	cmp    $0x18,%rsi					# if overflow
   0x0000000000401195 <+161>:	je     0x4011ab <phase_6+183>
   0x0000000000401197 <+163>:	mov    (%rsp,%rsi,1),%ecx
   0x000000000040119a <+166>:	cmp    $0x1,%ecx
   0x000000000040119d <+169>:	jle    0x401183 <phase_6+143>
   0x000000000040119f <+171>:	mov    $0x1,%eax
   0x00000000004011a4 <+176>:	mov    $0x6032d0,%edx
   0x00000000004011a9 <+181>:	jmp    0x401176 <phase_6+130>
```

&uarr;From line `<+123>` to line `<+181>`, the program reorders the content stored at `0x6032d0` according to our input. Dereference this address:

```
(gdb) x/24 0x6032d0
0x6032d0 <node1>:	332	1	6304480	0
0x6032e0 <node2>:	168	2	6304496	0
0x6032f0 <node3>:	924	3	6304512	0
0x603300 <node4>:	691	4	6304528	0
0x603310 <node5>:	477	5	6304544	0
0x603320 <node6>:	443	6	0	0
```

&uarr;So the program reordered these 6 nodes according to our input sequence. The structure of the node is like this:

```
struct {
    int value;
    int order;
    node* next;
} node;
```



```
   0x00000000004011ab <+183>:	mov 0x20(%rsp),%rbx
   0x00000000004011b0 <+188>:	lea    0x28(%rsp),%rax
   0x00000000004011b5 <+193>:	lea    0x50(%rsp),%rsi
   0x00000000004011ba <+198>:	mov    %rbx,%rcx
   0x00000000004011bd <+201>:	mov    (%rax),%rdx
   0x00000000004011c0 <+204>:	mov    %rdx,0x8(%rcx)				# set the pointer to the next
   0x00000000004011c4 <+208>:	add    $0x8,%rax
   0x00000000004011c8 <+212>:	cmp    %rsi,%rax
   0x00000000004011cb <+215>:	je     0x4011d2 <phase_6+222>
   0x00000000004011cd <+217>:	mov    %rdx,%rcx
   0x00000000004011d0 <+220>:	jmp    0x4011bd <phase_6+201>
   0x00000000004011d2 <+222>:	movq   $0x0,0x8(%rdx)				# the last pointer is NULL
```

&uarr; Rebuild the linked list

```
   0x00000000004011da <+230>:	mov    $0x5,%ebp					# counter
   0x00000000004011df <+235>:	mov    0x8(%rbx),%rax
   0x00000000004011e3 <+239>:	mov    (%rax),%eax
   0x00000000004011e5 <+241>:	cmp    %eax,(%rbx)					# if decreasing
   0x00000000004011e7 <+243>:	jge    0x4011ee <phase_6+250>
   0x00000000004011e9 <+245>:	callq  0x40143a <explode_bomb>
   0x00000000004011ee <+250>:	mov    0x8(%rbx),%rbx
   0x00000000004011f2 <+254>:	sub    $0x1,%ebp
   0x00000000004011f5 <+257>:	jne    0x4011df <phase_6+235>
   0x00000000004011f7 <+259>:	add    $0x50,%rsp
   0x00000000004011fb <+263>:	pop    %rbx
   0x00000000004011fc <+264>:	pop    %rbp
   0x00000000004011fd <+265>:	pop    %r12
   0x00000000004011ff <+267>:	pop    %r13
   0x0000000000401201 <+269>:	pop    %r14
   0x0000000000401203 <+271>:	retq  
End of assembler dump.
```

&uarr; If the linked list is not decreasing, the bomb will explode.

`924 > 691 > 477 > 443 > 332 > 168 ==> 3 4 5 6 1 2 `

Therefore, the order we input should be `4 3 2 1 6 5 ` (because of the step `input[i] = 7 - input[i]` ). 

Check out answer:

```
Congratularions! You've defused the bomb!
```

