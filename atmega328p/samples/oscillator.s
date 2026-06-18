// Test du composant Oscillator - Activation et Vérification du Ready

.global _start
.global _exit

.equ    LED,                0xA0000000  // Adresse de la LED
.equ    OSCILLATOR,         0xFF203000  // Base du composant
.equ    XTAL_CTRL_OFFSET,   0x40        // Offset de XTAL_CTRL
.equ    OSC_READY_OFFSET,   0x71        // Offset de OSC_READY (interne)

_start:
    ldr     R0, =LED                    // Charger l'adresse de la LED
    ldr     R1, =OSCILLATOR              // Charger la base du composant Oscillator
    add     R2, R1, #XTAL_CTRL_OFFSET    // Adresse absolue de XTAL_CTRL
    add     R3, R1, #OSC_READY_OFFSET    // Adresse absolue de OSC_READY

    // Activer l'oscillateur
    mov     R4, #0b10000000              // Bit 7 à 1 pour activer
    strb    R4, [R2]                     // Écrire dans XTAL_CTRL

wait_ready:
    ldrb    R5, [R3]                     // Lire OSC_READY

    cmp     R5, #1                       // Est-il prêt ?
    beq     led_on                       // Si prêt, allumer la LED

    mov     R6, #0                       // Sinon, LED éteinte
    str     R6, [R0]

    b       wait_ready                   // Boucle

led_on:
    mov     R6, #1
    str     R6, [R0]

    b       _exit

_exit:
    b       _exit
