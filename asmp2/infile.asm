
	org	0
	nop

work = 0x480

counter0 = 0
counter1 = 1
counter2 = 2
counter3 = 3

test1 = 12
test2 = 13
test3 = 14
test4 = 15
test5 = 16
test6 = 17
test7 = 18
test8 = 19
main:
	lea 2,work
	ld  #0
	st  counter0(p2)
	st  counter1(p2)
	st  counter2(p2)
	st  counter3(p2)
loop:
	ld  3(pc)
	ld  4(p0)
	ld  5(p1)
	ld  6(p2)
	ld  -3(pc)
	ld  -4(p0)
	ld  -5(p1)
	ld  -6(p2)
	st  test1(p2)
	st  test2(p2)
	st  test3(p2)
	st  test4(p2)
	st  test5(p2)
	st  test6(p2)
	st  test7(p2)
	st  test8(p2)

	DLD counter0(p2) ;B8 22 2      AC, EA <- (EA) - 1
    jnz loop

	DLD counter1(p2) ;B8 22 2      AC, EA <- (EA) - 1
    jnz loop

	DLD counter2(p2) ;B8 22 2      AC, EA <- (EA) - 1
    jnz loop

	halt

	DLD counter3(p2) ;B8 22 2      AC, EA <- (EA) - 1
    jnz loop

	halt

	ld  1
	st  127
	ldi  1
	adi  2
	ld  3(pc)
	ld  4(p0)
	ld  5(p1)
	ld  6(p2)
	ld  7(p3)
	ld  -3(pc)
	ld  -4(p0)
	ld  -5(p1)
	ld  -6(p2)
	ld  -7(p3)
	ld  @3(pc)
	ld  @4(p0)
	st  @5(p1)
	st  @6(p2)
	ld  @7(p3)
	ld  #1
	and #2
	xpah 2
	xpal 2




	jmp L1
	jp  L1
L1:
	jz  L1
	jnz L1

	ILD   3(p3) ;A8 22 2      AC, EA <- (EA) + 1
	DLD   4(p3) ;B8 22 2      AC, EA <- (EA) - 1
	LDE   ;40  6 1      AC <- (E)
	XAE   ;01  7 1      (AC) <-> (E)
	ANE   ;50  6 1      AC <- (AC) & (E)
	ORE   ;58  6 1      AC <- (AC) | (E)
	XRE   ;60  6 1      AC <- (AC) ^ (E)

	DAE   ;68 11 1  *   AC <- (AC) + (E) + (CY/L) {BCD format}
	ADE   ;70  7 1  **  AC <- (AC) + (E) + (CY/L)
	CAE   ;78  8 1  **  AC <- (AC) + !(E) + (CY/L)
	XPAL  1 ;30  8 1      (AC) <-> (PL)
	XPAH  2 ;34  8 1      (AC) <-> (PH)
	XPPC  3 ;3C  7 1      (PC) <-> (PTR)
;	//    LDI    L(SUBR1)
	SIO   ;19  5 1      Serial Input/Output
	SR    ;1C  5 1      Shift Right
	SRL   ;1D  5 1      Shift Right with Link
	RR    ;1E  5 1      Rotate Right
	RRL   ;1F  5 1  *   Rotate Right with Link
	CCL   ;02  5 1      CY/L <- 0
	SCL   ;03  5 1      CY/L <- 1
	IEN   ;05  6 1      IE <- 1
	DINT  ;04  6 1      IE <- 0
	CSA   ;06  5 1      AC <- (SR)
	CAS   ;07  6 1      SR <- (AC)
	NOP   ;08  5 1      No operation
	DLY   255 ;8F ?? 2      Delay

	ld    H(0x1234)
	ld    L(0x1234)
	ldh   0x1234
	ldL   0x1234
	ldh   sub
	ldL   sub

	nop


	lea 2,sub

	call	sub
	halt

sub:
	ret

	HALT  ;00  8 1      Output Halt pulse

;
