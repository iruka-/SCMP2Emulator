
	org	0
	nop

work2 = work + 32

work = 0x480

counter0 = 0
test1 = 12

main:
	lea 2,work
	ld  #0
	st  counter0(p2)
loop:
	ld  3(pc)
	DLD counter0(p2) ;B8 22 2      AC, EA <- (EA) - 1
    jnz loop

