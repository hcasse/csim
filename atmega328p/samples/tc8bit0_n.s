// Test du composant T/C 8 bits 0 - Mode normal

.global _start
.global _exit

// Définition des adresses
.equ    LED,            0xA0000000  // Adresse de la LED
.equ    TC8BIT0,        0xFF202000  // Base du composant
.equ    TIFR0_OFFSET,   0x35        // Offset de TIFR0 dans le composant
.equ    TCCR0A_OFFSET,  0x44        // Offset de TCCR0A dans le composant
.equ    TCCR0B_OFFSET,  0x45        // Offset de TCCR0B dans le composant
.equ    TIMSK0_OFFSET,  0x6E        // Offset de TIMSK0 dans le composant

_start:
    ldr     R0, =LED                // Charger l’adresse de la LED
    ldr     R1, =TC8BIT0            // Charger la base du timer
    add     R2, R1, #TIFR0_OFFSET   // Calculer l’adresse absolue de TIFR0
    add     R3, R1, #TCCR0A_OFFSET  // Calculer l'adresse absolue de TCCR0A
    add     R4, R1, #TCCR0B_OFFSET  // Calculer l'adresse absolue de TCCR0B
    add     R5, R1, #TIMSK0_OFFSET  // Calculer l'adresse absolue de TIMSK0

    mov     R6, #0b00000000         // Charger mode dans R6
    strb    R6, [R3]                // Écrire le mode (R6) à l'adresse de TCCR0A

    mov     R6, #0x04                // Charger prescaler dans R6
    strb    R6, [R4]                // Ecrire le prescaler (R6) à l'adresse de TCCR0B




_loop: 
    ldrb    R7, [R2]                // Lire TIFR0 (1 octet)

    tst     R7, #0b00000001         // Tester le bit TOV0 (bit 0)
    bne     led_on                  // Si TOV0 == 1, allumer LED

    mov     R8, #0                  // Sinon, éteindre LED
    str     R8, [R0]
    
    b       _loop

led_on:
    mov     R8, #1                  // Affecter 1 dans R8
    str     R8, [R0]                // Allumer la LED

    //mov     R7, #0b00000000       // Réinitialiser le flage
    //strb     R7, [R2]

    b       _exit

_exit:
    b       _exit
