	.global _start
	.global _exit

	.equ GPIOA_BASE,	0xff000000
	.equ GPIO_DAR, 		0x0
	.equ GPIO_DIR, 		0x4
	.equ GPIO_IMR, 		0x8
	.equ GPIO_ECR, 		0xc

	.equ LED, 0
	.equ BUT, 1

_start:

	@ configuration
	ldr	R0, =GPIOA_BASE
	mov R1, #0
	str R1, [R0, #GPIO_DAR]
	mov R1, #1 << LED
	str R1, [R0, #GPIO_DIR]

loop:

	ldr R1, [R0, #GPIO_ECR]
	and R1, R1, #1 << BUT
	cmp R1, #0
	beq loop

	mov R1, #0
	str R1, [R0, #GPIO_ECR]
	ldr R1, [R0, #GPIO_DAR]
	eor R1, R1, #1<<LED
	str R1, [R0, #GPIO_DAR]
	b	loop

_exit:
	b	_exit
