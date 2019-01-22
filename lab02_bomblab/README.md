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

The program is at the line  `<+0>`. According to the codes, we can know that `callq` will check if the value stored at the first and second parameter are identical. The second parameter is stored in %esi, and the address is 0x402400. The first parameter is stored in %eax. We can use `x/s` to dereference this address.

```
(gdb) x/s $eax
0x603780 <input_strings>:		"abc"
```

This is what we just input. Therefore, we know that if the string we input is the same as the string stored at address 0x402400, the bomb will not explode. Let's find out the answer.

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

The second phase is about loops. As phase 1, we first type some random words.

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

The third phase is about conditionals/switches. Type some words and use `disas`

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

So propably in this phase we should input 2 intergers. The first number is stored at (%rsp + 0x8), the second is at (%rsp + 0xc). From line `<+29>`  to line `<+50>` we know that this construction is a swith. The first number is used to decide the case and it must between 0 to 7. `jmpq   *0x402470(,%rax,8)` means jump to address 0x402470 + %rax × 8, so 0x402470 must be where the jump table is. This switch has 8 cases, so there are 8 addresses in the table. Use `x/8x <address>` to print 8 addresses in hexadecimal format.

```
(gdb) x/8x 0x402470
0x402470:	0x0000000000400f7c	0x0000000000400fb9
0x402480:	0x0000000000400f83	0x0000000000400f8a
0x402490:	0x0000000000400f91	0x0000000000400f98
0x4024a0:	0x0000000000400f9f	0x0000000000400fa6
```

Compared to the code of phase_3, the answer can be one of these 8 pairs:

`0 207` `1 311` `2 707` `3 256` `4 389` `5 206` `6 682` `7 327`.

Check our answers:

```
Halfway there!
```

<h3 id = "phase4">Phase 4</h3>

The forth phase is about recursive calls and the stack discipline.

```

```





