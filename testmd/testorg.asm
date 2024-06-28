Start:		HLT
		NOP
		; Conditional Jump Instructions
CJ:		JOV	*+2
		JAZ	2
		NAB	CJ
		NAX	*+300			; out of range
		SSS	CJ+2			; Illegal mnemnotic operation code
		; Shift instructions
Alpha:		LLA	5
		LLL	-5			; Negative operants are not allowed for shift
		ALA	0x10
Alpha:		ARL	TEN+2			; Symbol name is multi defined
		ALA	0
		; Input / output
Beta:		IBS				; no operant for serial IO
		IBA	2,2			; Device order, device number
		OBM	4,4,Buf,X		; Memory I/O with index flag = X
		; Register operate
		XRA
		; non litereral memory referencing instructions
123456:		LDA	*
		ORG	256
.DOT		STA	*+2			; Mode 1
		LDX*	OOPS			; Mode 2 Symbol in operant is undefined
		LDX*	Buf+5			; Mode 3
A23456:		SIX-				; Mode 4
		ADV+	30			; Mode 5
		MUL/	Sam,X			; Mode 6 with address flag
		LDV/	Sam			; Mode 6
		; Fixed word length literal
		LDA=	-1			; Two byte literal
Spot:		STA=	*			; A reg. stored in Spot+1
		MUL=	0
		LDA=	Alpha
		LDA=	Alpha,X
		LDX=	Alpha
		CPA=	'A'			; Two byte literal with first byte zeros
		ANA=	0ffh			; Two byte literal with hex right justified
		; Variable word length literals (Mode 7)
		LDV=	Sam			; Two byte address literal
		ADV=	1023
		ANV=	$9ABCDEF
		CPV=	'A'
		; Assembler instructions
TEN:		EQU	8
