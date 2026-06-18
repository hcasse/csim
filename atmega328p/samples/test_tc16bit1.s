	.global _start
	.global _exit

    .equ LED,  0xA0000000 
    .equ TIMER, 0xFF201000
    .equ DURATION, 0x01FF
    .equ TCCR1A, 0x80
    .equ TCCR1B, 0x81
    .equ TCCR1C, 0x82
    .equ TCNT1H, 0x85
    .equ TCNT1L, 0x84
    .equ OCR1BH, 0x8B
    .equ OCR1BL, 0x8A
    .equ ICR1H, 0x87
    .equ ICR1L, 0x86
    .equ TIMSK1, 0x6F
    .equ TIFR1, 0x36
    .equ OCR1AH, 0x89
    .equ OCR1AL, 0x88


_start :
    ldr	r0, =LED 
	ldr	r1, =TIMER
    add     R2, R1, #TCCR1A     // R2 = adresse de TCCR1A
    add     R3, R1, #TCCR1B     // R3 = adresse de TCCR1B
    add     R4, R1, #OCR1AH     // R4 = adresse de OCR1A High
    add     R5, R1, #OCR1AL     // R5 = adresse de OCR1A Low
    add     R6, R1, #TIFR1      // R6 = adresse de TIFR1
    add     R7, R1, #TIMSK1     // R7 = adresse de TIMSK1

    //mode normal initialisation
    mov r8, #0x00
    strb r8, [r2]
    add r8,r8, #1
    strb r8, [r3]

_init :
    ldrb r9, [r6] // verification du flag TOV1
    cmp r9, r8
    bne _off
    b _on

_on :
    mov r10,#1
  //  cmp r9
   // b _on
   b _exit


_off :
    mov r10,#0
    cmp r9, r0
    beq _on
    b _off

_exit:
    b       _exit  
