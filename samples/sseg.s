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

loop:
	b	loop

_exit:
	b	_exit


