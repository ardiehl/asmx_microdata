; dont put the dot commands to the first column
    .CPU MD1600
; use .org or ORG but ORG must mot be at first column
    .ORG 200h
; OLDSYN enabled the 'old' chars appended to opcodes
    OLDSYN 1

Start:  	DIN
lab1:		LDA PAGE0 [ffh]		; m=0, direct page 0
		LDA SHORT [0x190]		; m=1
		LDA SHORT [Start]	; m=1, direct relative
		LDA	[*1]		; m=2, indirect page 0, SHORT not needed
		LDA	[*Start]	; m=3, indirect relative, SHORT not needed
		LDA	[X]		; m=4, indexed
lab2		LDA	[X+12h]		; m=5, indexed with bias
		LDA	[7ef0h]		; m=6, extended address, x=0
		LDA	[X+0x7ef0]	; m=6, extended address, x=1
lab3		LDA	7F91h		; m=7 literal

		; Jump and Return Jump
lab4:		JMP	short 200h	; m=0, direct page 0
		JMP	200h
lab4_1:		JMP SHORT Lab4		; m=1, direct relative
;		JMP SHORT TheEnd	; error
lab4_2:		JMP	[1]		; m=2, indirect page 0
lab5:		JMP	[Start]		; m=3, indirect relative
		JMP	X		; m=4, indexed
		JMP	X+12h		; m=5, indexed with bias
lab6:		JMP	7ef0h		; m=6, extended address, x=0
		JMP	X+0x7ef0	; m=6, extended address, x=1
		JMP	X+[$7F91]	; m=7, indirect extended, x=1
lab7:		JMP	[7F91H]		; m=7, indirect extended, x=0
		JOV	nxt		; should be zero
nxt:		JOV	*+2		; zero as well, does NOT WORK if old sytax is enabled, will be interpreted as JOV*
		JOV*	+2

	; variable length
		ADV	ffffh		; this is only one byte as this is the default after reset
		.W2
		ADV	1234h		; two bytes
		.W3
		ADV	123456h		; tree bytes
		.W4
		ADV	12345678h	; four bytes
		.W1
		ADV	1,12h
		ADV	2,1224h
		ADV	3,123456h
		ADV	4,12345678h

	; old style
	; underscore instead of blank
		JMP_	0		; m=0
		JMP_	*+2		; m=1
		JMP*	0		; m=2
		JMP*	*+2		; m=3
		JMP-			; m=4
		JMP+	12h		; m=5
		RTJ/	TheEnd		; m=6
		JMP/	TheEnd,X	; m=6, x=1
		RTJ=	TheEnd		; m=7
		JMP=	TheEnd,X	; m=7, x=1

		IWM_	0		; m=0
		DWM_	*+2		; m=1
		LDX*	0		; m=2
		STX*	*+2		; m=3
		LDB-			; m=4
		STB+	12h		; m=5
		ADA/	TheEnd		; m=6
;		ADV/	TheEnd,X	; m=6, x=1
;		SBA=	TheEnd		; m=7


                HLT
	; DB/DS must not start @ col 1
		DB		0,0,0,0,0,0,0,0
		DS		5,0
		DS		2,'S'
		DB		"StackStackStackStack"
		DS 1
 ORG 1000h
TheEnd:


