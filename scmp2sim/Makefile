#################################################################
#	コンパイラは MinGw gccを使用します。
#	試してはいませんがcygwinでも大丈夫なはずです。
#	(cygwinでは、コンパイルオプション -mno-cygwinを付けます)
#	DEBUGの時には、-gをつけ、-sを外します。
#################################################################
# REMOVE DEFAULT MAKE RULE
MAKEFLAGS = -r

.SUFFIXES:

.SUFFIXES:	.c .o


#============================
# DOSかどうかチェック.
 ifdef ComSpec
MSDOS=1
 endif

 ifdef COMSPEC
MSDOS=1
 endif
#============================

 ifdef MSDOS
DOSFLAGS = -D_MSDOS_
EXE_SUFFIX = .exe
WIN32LIB= -lkernel32 -luser32 -lgdi32 -lsetupapi 
 else
DOSFLAGS = -D_LINUX_
EXE_SUFFIX = .exe
WIN32LIB= 
 endif

CFLAGS	= $(DOSFLAGS) $(CDEFS) -O2 -Wall
LIBS	=

TARGET  = scmp2.exe
#
#
#

CC = gcc
RC = windres

.c.o:
	$(CC) $(CFLAGS) -c $<
#
#
files	= main.o opcode.o disasm.o gr.o led.o debug.o linux.o
#
#
#
#
$(TARGET) : $(files)
	$(CC) -s -o $@ $(files) $(WIN32LIB) -lm $(LIBS)
#
debugtest:
	./$(TARGET) NIBL.bin
#
test:
	./$(TARGET) -q NIBL.bin
#
#
clean:
	-rm *~
	-rm *.o
	-rm $(TARGET)
#
#
