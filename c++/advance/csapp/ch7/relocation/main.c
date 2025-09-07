// Reference: https://zhuanlan.zhihu.com/p/588913819

int add(int a_, int b_);
extern int global_extern_int;
int global_int = 3;

int main()
{
    static int a = 19;
    global_int = 5;
    int rtv = 0;
    rtv = add(global_int, global_extern_int);
    return rtv;
}

/*
>>> gcc -g -fno-pie  -no-pie -m32 -c main.c

>>> readelf -s main.o
Symbol table '.symtab' contains 18 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 00000000     0 FILE    LOCAL  DEFAULT  ABS main.c
     2: 00000000     0 SECTION LOCAL  DEFAULT    1
     3: 00000000     0 SECTION LOCAL  DEFAULT    3
     4: 00000000     0 SECTION LOCAL  DEFAULT    4
     5: 00000004     4 OBJECT  LOCAL  DEFAULT    3 a.1416
     6: 00000000     0 SECTION LOCAL  DEFAULT    5
     7: 00000000     0 SECTION LOCAL  DEFAULT    7
     8: 00000000     0 SECTION LOCAL  DEFAULT    8
     9: 00000000     0 SECTION LOCAL  DEFAULT   10
    10: 00000000     0 SECTION LOCAL  DEFAULT   12
    11: 00000000     0 SECTION LOCAL  DEFAULT   14
    12: 00000000     0 SECTION LOCAL  DEFAULT   15
    13: 00000000     0 SECTION LOCAL  DEFAULT   13
    14: 00000000     4 OBJECT  GLOBAL DEFAULT    3 global_int
    15: 00000000    72 FUNC    GLOBAL DEFAULT    1 main
    16: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND global_extern_int
    17: 00000000     0 NOTYPE  GLOBAL DEFAULT  UND add


>>> readelf -r ./main.o

Relocation section '.rel.text' at offset 0x450 contains 4 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
00000013  00000e01 R_386_32          00000000   global_int
00000024  00001001 R_386_32          00000000   global_extern_int
00000029  00000e01 R_386_32          00000000   global_int
00000033  00001102 R_386_PC32        00000000   add
...

>>> gcc -g -fno-pie -no-pie -m32 -o main main.o add.o
>>> readelf -s main

>>> objdump -d main

080483f6 <main>:
 80483f6:       8d 4c 24 04             lea    0x4(%esp),%ecx
 80483fa:       83 e4 f0                and    $0xfffffff0,%esp
 80483fd:       ff 71 fc                pushl  -0x4(%ecx)
 8048400:       55                      push   %ebp
 8048401:       89 e5                   mov    %esp,%ebp
 8048403:       51                      push   %ecx
 8048404:       83 ec 14                sub    $0x14,%esp
 8048407:       c7 05 18 a0 04 08 05    movl   $0x5,0x804a018
 804840e:       00 00 00 
 8048411:       c7 45 f4 00 00 00 00    movl   $0x0,-0xc(%ebp)
 8048418:       8b 15 20 a0 04 08       mov    0x804a020,%edx
 804841e:       a1 18 a0 04 08          mov    0x804a018,%eax
 8048423:       83 ec 08                sub    $0x8,%esp
 8048426:       52                      push   %edx
 8048427:       50                      push   %eax
 8048428:       e8 11 00 00 00          call   804843e <add>
 804842d:       83 c4 10                add    $0x10,%esp
 8048430:       89 45 f4                mov    %eax,-0xc(%ebp)
 8048433:       8b 45 f4                mov    -0xc(%ebp),%eax
 8048436:       8b 4d fc                mov    -0x4(%ebp),%ecx
 8048439:       c9                      leave  
 804843a:       8d 61 fc                lea    -0x4(%ecx),%esp
 804843d:       c3                      ret 

0804843e <add>:
 804843e:       55                      push   %ebp
*/