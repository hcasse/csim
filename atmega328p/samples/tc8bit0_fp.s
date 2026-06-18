// Test du composant T/C 8 bits 0 - Mode Fast PWM

.global _start
.global _exit

// Définition des adresses
.equ    LED,            0xA0000000  // Adresse de la LED
.equ    TC8BIT0,        0xFF202000  // Base du composant
.equ    TIFR0_OFFSET,   0x35        // Offset de TIFR0 dans le composant
.equ    TCCR0A_OFFSET,  0x44        // Offset de TCCR0A dans le composant
.equ    TCCR0B_OFFSET,  0x45        // Offset de TCCR0B dans le composant
.equ    OCR0A_OFFSET,   0x47        // Offset de OCR0A
.equ    OCR0B_OFFSET,   0x48        // Offset de OCR0B
.equ    TIMSK0_OFFSET,  0x6E        // Offset de TIMSK0 dans le composant

_start:
    ldr     R0, =LED                // LED
    ldr     R1, =TC8BIT0            // Base timer
    add     R2, R1, #TIFR0_OFFSET   // TIFR0
    add     R3, R1, #TCCR0A_OFFSET  // TCCR0A
    add     R4, R1, #TCCR0B_OFFSET  // TCCR0B
    add     R5, R1, #OCR0A_OFFSET   // OCR0A
    add     R6, R1, #OCR0B_OFFSET   // OCR0B
    add     R7, R1, #TIMSK0_OFFSET  // TIMSK0

    // Configuration du mode Fast PWM (WGM = 111)
    mov     R8, #0x03               // WGM bits (111 pour Fast PWM)
    strb    R8, [R3]                // Ecriture dans TCCR0A
    mov     R8, #0x03               // CS02:0 = 011 -> prescaler 64
    strb    R8, [R4]                // Ecriture dans TCCR0B

    // Configuration des valeurs OCR0A et OCR0B pour tester Fast PWM
    mov     R8, #0x80               // Valeur de comparaison OCR0A (50% du cycle)
    strb    R8, [R5]                // Ecriture dans OCR0A

    mov     R8, #0x40               // Valeur de comparaison OCR0B (25% du cycle)
    strb    R8, [R6]                // Ecriture dans OCR0B

    // Activer les interruptions liées au Timer (OCIE0A et OCIE0B)
    mov     R8, #0x03               // Activer OCIE0A et OCIE0B
    strb    R8, [R7]

_loop:
    ldrb    R9, [R2]                      // Lire TIFR0
    
    tst     R9, #0x02                     // Tester le bit OCF0A (signal de comparaison OC0A)
    bne     led_on                        // Si OCF0A == 1, allumer LED

    tst     R9, #0x04                     // Tester le bit OCF0B (signal de comparaison OC0B)
    bne     led_on_b                      // Si OCF0B == 1, allumer LED (ou autre action)

    mov     R10, #0                       // Si aucune condition n'est vraie, éteindre LED
    str     R10, [R0]

    beq     _loop                       // Continuer la boucle principale

led_on:
    mov     R10,  #1
    str     R10, [R0]                    // Allumer la LED si OCF0A

    b       _loop

led_on_b:
    mov     R10,  #1
    str     R10, [R0]                    // Allumer la LED si OCF0B (ou un autre effet)

    b       _loop

_exit:
    b       _exit                        // Boucle infinie à la fin
