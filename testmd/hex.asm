

; Pseude code
		.CPU	MD1600
		ORG	0200h
		OLDSYN	0

		JMP	Main

;	SUBR charOut
charOut:	OBA	0,0		; for basic four 210, other address for the 510
					; how to check for transmit empty, for now delay for a while
		PUSHB
		LDB	D000h		; delay count
.charOutDelay:	INB			; increment B
		NBZ	.charOutDelay
		POPB
		RTN

crlf:		PUSHA
		LDA	0x0D
		CALL	charOut
		LDA	0x0A
		CALL	charOut
		POPA
		RTN


nibOut:		CPV	09H		; assumed word size is set to 1, A<OP
		JR	.nibOut2		; CPV jumps here if A<OP
		JR	.NibOutP		; and here if A=OP
					; and here if A>OP
.nibOutP:	ADV	'9'
		CALL	charOut		
		JR	charOut
.nibOut2:	ADV	'A'-'9'
		JR	charOut


; print bex byte of A
hexOut		PushA
		LRA	4
		CALL	nibOut
		PopA
		CALL	nibOut
		RTN

Main:		LDV	0f0h
		CALL	hexOut
		LDV	019h
		CALL	hexOut
		CALL	crlf
		HLT
