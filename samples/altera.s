	.global _start
	.global _exit

	.equ	LED,	0xA0000000
	.equ	TIM,	0xFF202000
    .equ    DURATION, 0xFFFF

_start:
	ldr	R0, =LED
	ldr	R1, =TIM

    add R2, R1, #0x8 // R2  = counter start low
    add R3, R1, #0x4 // R3  = control register
    mov R11, #1 //      R11 = bit flipped when timer reaches zero
    ldr R10, =DURATION
    str R10, [R2]
    mov R10, #0b0110
    str R10, [R3]

loop:
	ldr	R10, [R1]
    cmp R10, #3 //on teste si il tourne et a fini
    mov R10, #2
    str R10, [R1] // on reset TO
    eoreq R11, R11, #1
    str R11, [R0]
	b	loop

_exit:
	b	_exit

