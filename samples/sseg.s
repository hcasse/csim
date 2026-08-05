	.global _start
	.global _exit

	.equ GPIOA_BASE,	0xff000000
	.equ GPIO_DAR, 		0x0
	.equ GPIO_DIR, 		0x4
	.equ GPIO_IMR, 		0x8
	.equ GPIO_ECR, 		0xc

	.equ V0,	0b1110111

_start:

	@ configuration
	ldr	R0, =GPIOA_BASE
	mov R1, #V0
	str R1, [R0, #GPIO_DAR]
	adr R2, digits
	mov R3, #0

loop:

	ldr	R1, =1000

wait:
	sub R1, R1, #1
	cmp R1, #0
	bgt wait

cont:
	add R3, R3, #1
	cmp R3, #10
	movhs R3, #0

	ldr R1, [R2, R3, LSL #2]
	str R1, [R0, #GPIO_DAR]

	b	loop

_exit:
	b	_exit

digits:
	.int	V0
	.int	0b0100100
	.int	0b1011101
	.int	0b1101101
	.int	0b0101110
	.int	0b1101011
	.int	0b1111011
	.int	0b0100101
	.int	0b1111111
	.int	0b1101111


