	.global _start
	.global _exit

	.equ	LEDS10,	0xA0000000
	.equ    BUTTON, 0xB0000000

_start:
	LDR	R0, =LEDS10
	LDR R1, [R0]
	LDR R2, =BUTTON

loop:
    STR R1, [R0]
	LDR R3,[R2]
	CMP R3,#1
	BNE loop
	BL wait_button
    ADD R1,R1,#1
	CMP R1,#1024
	MOVEQ R1,#0
	b	loop

wait_button :
	LDR R3,[R2]
	CMP R3,#0
	BNE wait_button
	MOV pc,lr

_exit:
	b	_exit
