
.ORG 200h
    
; LDV with m=7 depends on the "variable precition mode" (R01..R04)
; seems to be 1 (=8 bit) after reset

Start:  	DIN		        ; 04        Disable interrupts
                LDX	FFBFh	    ; 87FFBF    X=FFBF
                LDV	AFh     	; EFAF      Load variable, A=AF, Sets bit 8..15=bit7 => FFAF
                                ;           w/o bit 7=2F 2F+1='0'
Loop:           INA     		; 48        inc A
                OBA 0    		; 3900      output byte from A to port 0, dev=0(bit 0..4)
                LDB	D000h	    ; 97D000    Reg B=D000
DelayB:			INB		        ; 49        increment B
                NBZ	DelayB  	; 1ADF      -3 Jump relative if B not equal to 0
                NAX	Loop	    ; 1FF5      -11 Jump relative if A <> X

                JMP Start	    ; 660200 repeat loop
                HLT             ; 00

	
