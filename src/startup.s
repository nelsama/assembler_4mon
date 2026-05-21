; ============================================
; startup.s - Código de inicio para AS65
; ============================================
; Inicializa el runtime CC65 para el ensamblador
; SIN variables ZP propias para no desplazar sp
; (sp debe estar en $0002 como en el monitor)
; ============================================

.export _init
.export __STARTUP__ : absolute = 1

.import _main
.import __BSS_RUN__, __BSS_SIZE__
.importzp sp

; Usamos $FA-$FF en ZP para zerobss (evita desplazar sp)
PTR1    = $FA
PTR2    = $FC
CNT     = $FE

.segment "STARTUP"

_init:
    sei
    cld

    ; Inicializar stack pointer del 6502
    ldx #$FF
    txs

    ; Inicializar stack pointer de CC65 (software stack)
    lda #<$3FFF
    sta sp
    lda #>$3FFF
    sta sp+1

    ; Inicializar BSS a ceros
    jsr zerobss

    ; Llamar a main
    jsr _main

    ; Volver al monitor
    jmp $8000

; ============================================
; zerobss - Inicializa BSS a ceros
; Usa ZP fijo $FA-$FF para no interferir con sp
; ============================================
zerobss:
    lda #<__BSS_SIZE__
    ora #>__BSS_SIZE__
    beq @done

    lda #<__BSS_RUN__
    sta PTR1
    lda #>__BSS_RUN__
    sta PTR1+1

    lda #<__BSS_SIZE__
    sta CNT
    lda #>__BSS_SIZE__
    sta CNT+1

    ldy #0
    lda #0
@loop:
    sta (PTR1),y
    inc PTR1
    bne @skip
    inc PTR1+1
@skip:
    lda CNT
    bne @dec_low
    dec CNT+1
@dec_low:
    dec CNT
    lda CNT
    ora CNT+1
    bne @loop
@done:
    rts
