; ============================================================================
; test_leds.asm - Juego de luces LED para Tang Nano 9K
; Adaptado para AS65 (ensamblador residente)
;
; Compilar con AS65 en la Tang Nano:
;   LOAD AS65.BIN 0800 346B
;   R 0800
;   > A
;   Archivo fuente: TEST_LEDS.ASM
;   Nombre del archivo de salida: TEST_LEDS.BIN
;
; Cargar y ejecutar:
;   LOAD TEST_LEDS.BIN 0400
;   R 0400
;
; Puerto de LEDs: $C001 (lógica negativa: 0=encendido, 1=apagado)
; ============================================================================

.org $0800

; ============================================================================
; CONSTANTES
; ============================================================================
LEDS    = $C001         ; Puerto de LEDs

; ============================================================================
; VARIABLES EN ZERO PAGE
; ============================================================================
led     = $20           ; LED actual
temp    = $21           ; Variable temporal
count   = $22           ; Contador general

; ============================================================================
; PUNTO DE ENTRADA
; ============================================================================
start:
    jsr init

main_loop:
    jsr effect_knight_rider
    jmp main_loop

; ============================================================================
; INICIALIZACION
; ============================================================================
init:
    lda #$FF
    sta LEDS
    rts

; ============================================================================
; EFECTO: KNIGHT RIDER (luz que va y viene)
; ============================================================================
effect_knight_rider:
    lda #$01
    sta led

@go_left:
    lda led
    eor #$FF
    sta LEDS
    jsr delay_long
    asl led
    lda led
    cmp #$40
    bne @go_left

    lda #$20
    sta led

@go_right:
    lda led
    eor #$FF
    sta LEDS
    jsr delay_long
    lsr led
    lda led
    bne @go_right

    rts

; ============================================================================
; EFECTO: PARPADEO TODOS LOS LEDS
; ============================================================================
effect_blink:
    ldx #$05

@loop:
    lda #$00
    sta LEDS
    jsr delay_long
    lda #$FF
    sta LEDS
    jsr delay_long
    dex
    bne @loop
    rts

; ============================================================================
; EFECTO: CONTADOR BINARIO
; ============================================================================
effect_counter:
    lda #$00
    sta count

@loop:
    lda count
    eor #$FF
    sta LEDS
    jsr delay_long
    inc count
    lda count
    cmp #$40
    bne @loop
    rts

; ============================================================================
; DELAY LARGO (~0.5 segundos a 3.375 MHz)
; ============================================================================
delay_long:
    ldx #$30
@outer:
    ldy #$00
@inner:
    nop
    nop
    nop
    nop
    dey
    bne @inner
    dex
    bne @outer
    rts

; ============================================================================
; DELAY CORTO
; ============================================================================
delay_short:
    ldx #$10
@outer:
    ldy #$00
@inner:
    dey
    bne @inner
    dex
    bne @outer
    rts
