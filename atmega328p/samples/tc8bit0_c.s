// Test du composant T/C 8 bits 0 - Mode CTC avec LED qui clignote

.global _start
.global _exit

// Définition des adresses
.equ    LED,            0xA0000000  // Adresse de la LED
.equ    TC8BIT0,        0xFF202000  // Base du composant
.equ    TIFR0_OFFSET,   0x35        // Offset de TIFR0 dans le composant
.equ    TCCR0A_OFFSET,  0x44        // Offset de TCCR0A dans le composant
.equ    TCCR0B_OFFSET,  0x45        // Offset de TCCR0B dans le composant
.equ    OCR0A_OFFSET,   0x47        // Offset de OCR0A
.equ    TIMSK0_OFFSET,  0x6E        // Offset de TIMSK0 dans le composant

_start:
    ldr     R0, =LED                // LED
    ldr     R1, =TC8BIT0            // Base timer
    add     R2, R1, #TIFR0_OFFSET   // TIFR0
    add     R3, R1, #TCCR0A_OFFSET  // TCCR0A
    add     R4, R1, #TCCR0B_OFFSET  // TCCR0B
    add     R5, R1, #OCR0A_OFFSET   // OCR0A
    add     R6, R1, #TIMSK0_OFFSET  // TIMSK0

    // Mode CTC = WGM = 010
    mov     R7, #0x02               // Mode CTC
    strb    R7, [R3]

    mov     R7, #0x05                     // CS02:0
    strb    R7, [R4]

    mov     R7, #0xFF                      // Valeur de comparaison OCR0A
    strb    R7, [R5]


_loop:
    ldrb    R8, [R2]                      // Lire TIFR0
    
    tst     R8, #0x02                     // Tester le bit OCF0A
    bne     led_on                        // Si OCF0A == 1, allumer LED

    mov     R9, #0                        // Sinon, éteindre LED
    str     R9, [R0]

    beq     _loop                      

led_on:
    mov     R9,  #1
    str     R9, [R0]

    b       _exit

_exit:
    b       _exit
