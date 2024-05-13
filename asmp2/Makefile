#
#
#
.SUFFIXES:

.SUFFIXES:	.c .o

.c.o:
	gcc -c -Wall -O2 -o $*.o $*.c
#
#
files	= main.o asmp2.o hash.o debug.o opcode.o
#
#
#
#
asmp2.exe : $(files)
	gcc -O2 -s $(files) -o $@
#	cp asm16.exe cina16
#
nibl:
	./asmp2.exe -l nibl.asm
#
test:
	./asmp2.exe -l test.asm
	cat test.lst
#
test2:
	./asmp2.exe -l infile.asm
	cat infile.lst
	cp  infile.bin ../scmp2sim/testdata.bin
#
#
#
clean:
	-rm *.o
	-rm *~
	-rm asmp2.exe
	-rm main
#
#
