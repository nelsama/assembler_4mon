;; ===========================================================================
;; io_rom.s - I/O functions via ROM API (ahorra espacio vs C)
;; ===========================================================================
;; Convierte a ASM las funciones de I/O mas simples para evitar el overhead
;; del runtime CC65 (pushax, etc.) y reducir el tamano del binario.
;; ===========================================================================

.export _put_str, _put_chr, _put_hex8, _put_hex16
.export _put_newline, _get_chr, _sym_init, _scan_init
.export _sym_get_count

.import _g_symbols, _g_num_symbols
.import _g_has_label, _g_has_operand, _g_is_directive, _g_directive
.import _g_directive_argc, _g_is_constant, _g_constant_value
.import _g_instr, _g_mode, _g_label_buf, _g_mnemonic_buf, _g_operand_buf

.segment "CODE"

;; ======================================================================
;; put_str(const char *s)  - fastcall: s en A/X
;; Llama a rom_uart_puts($BF1E) que recibe el string en A/X
;; ======================================================================
_put_str:
    jmp $BF1E

;; ======================================================================
;; put_chr(char c)  - fastcall: c en A
;; Llama a rom_uart_putc($BF18) que recibe el char en A
;; ======================================================================
_put_chr:
    jmp $BF18

;; ======================================================================
;; put_newline(void)  - envia \r\n
;; ======================================================================
_put_newline:
    lda #$0D
    jsr $BF18
    lda #$0A
    jmp $BF18

;; ======================================================================
;; get_chr(void)  - espera y recibe un char, retorna en A
;; ======================================================================
_get_chr:
    jsr $BF21       ; uart_rx_ready
    cmp #0
    beq _get_chr    ; loop mientras no haya char
    jmp $BF1B       ; uart_getc, retorna char en A

;; ======================================================================
;; put_hex_nibble - Convierte nibble (0-15) a hex y lo envia
;; Entrada: A = nibble
;; ======================================================================
_put_hex_nibble:
    cmp #10
    bcc :+          ; branch if A < 10 (carry clear)
    clc
    adc #$37        ; 'A' - 10 = $37
    jmp $BF18
:   clc
    adc #$30        ; '0' = $30
    jmp $BF18

;; ======================================================================
;; put_hex8(uint8_t val)  - fastcall: val en A
;; Imprime byte como 2 digitos hex
;; ======================================================================
_put_hex8:
    pha
    lsr a
    lsr a
    lsr a
    lsr a           ; A = nibble alto
    jsr _put_hex_nibble
    pla
    and #$0F        ; A = nibble bajo
    jsr _put_hex_nibble
    rts

;; ======================================================================
;; put_hex16(uint16_t val)  - fastcall: val en A/X (A=lo, X=hi)
;; Imprime word como 4 digitos hex
;; ======================================================================
_put_hex16:
    pha             ; guardar lo byte
    txa             ; hi byte a A
    jsr _put_hex8   ; imprimir hi byte
    pla             ; recuperar lo byte
    jmp _put_hex8   ; imprimir lo byte

;; ======================================================================
;; sym_init - Inicializa tabla de simbolos a cero
;; ======================================================================
_sym_init:
    ldx #(16 * 14 / 4) - 1  ; 224/4=56 palabras
    lda #0
@lp:sta _g_symbols,x
    sta _g_symbols+$40,x
    sta _g_symbols+$80,x
    sta _g_symbols+$C0,x
    dex
    bpl @lp
    sta _g_num_symbols
    rts

;; ======================================================================
;; scan_init - Inicializa variables del scanner
;; ======================================================================
_scan_init:
    lda #0
    sta _g_has_label
    sta _g_has_operand
    sta _g_is_directive
    sta _g_directive
    sta _g_directive_argc
    sta _g_is_constant
    sta _g_constant_value
    sta _g_constant_value+1
    lda #56         ; I_LAST
    sta _g_instr
    lda #0
    sta _g_mode
    sta _g_label_buf
    sta _g_mnemonic_buf
    sta _g_operand_buf
    rts

;; ======================================================================
;; sym_get_count - Retorna cantidad de simbolos
;; ======================================================================
_sym_get_count:
    lda _g_num_symbols
    rts
