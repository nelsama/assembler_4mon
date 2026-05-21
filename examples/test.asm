.org $0800
LEDS = $C001
start:
    lda #$AA
    sta LEDS
loop:
    jmp loop
	