int global_extern_int = 2;

int add(int a_, int b_)
{
    return a_+ b_;
}

/*

Dynamic linking

gcc -g -fno-pie -no-pie -m32 -fPIC -c add.c
gcc -g -fno-pie -no-pie -m32 -shared add.o  -o libadd.so
gcc -g -fno-pie -no-pie -m32 -fPIC -c main.c
gcc -g -fno-pie -no-pie -m32 -o main2 main.o -L . -ladd


>>> readelf -r main2

Relocation section '.rel.dyn' at offset 0x35c contains 2 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
08049ffc  00000206 R_386_GLOB_DAT    00000000   __gmon_start__
0804a024  00000905 R_386_COPY        0804a024   global_extern_int

Relocation section '.rel.plt' at offset 0x36c contains 2 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
0804a00c  00000107 R_386_JUMP_SLOT   00000000   add
0804a010  00000307 R_386_JUMP_SLOT   00000000   __libc_start_main@GLIBC_2.

>>> readelf -r main

Relocation section '.rel.dyn' at offset 0x27c contains 1 entry:
 Offset     Info    Type            Sym.Value  Sym. Name
08049ffc  00000106 R_386_GLOB_DAT    00000000   __gmon_start__

Relocation section '.rel.plt' at offset 0x284 contains 1 entry:
 Offset     Info    Type            Sym.Value  Sym. Name
0804a00c  00000207 R_386_JUMP_SLOT   00000000   __libc_start_main@GLIBC_2.0

* Link library when loading

gcc -g -fno-pie -no-pie -m32 -o main2 main.o -L . -ladd -Wl,-rpath=/home/ga/workspace/strawberry/c++/advance/csapp/ch7/relocation
*/