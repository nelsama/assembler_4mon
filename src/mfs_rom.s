;; ===========================================================================
;; mfs_rom.s - MicroFS functions via ROM API (ahorra espacio vs C)
;; ===========================================================================

.export _mfs_init, _mfs_close, _mfs_read_line

.import _g_mfs_open, _g_mfs_buf_pos, _g_mfs_buf_len, _g_mfs_eof
.import _g_mfs_buffer, _g_line_buf

.segment "CODE"

ROMAPI_SD_INIT    = $BF00
ROMAPI_MFS_MOUNT  = $BF03
ROMAPI_MFS_READ   = $BF09
ROMAPI_MFS_CLOSE  = $BF0C

;; ======================================================================
;; mfs_init
;; ======================================================================
_mfs_init:
    lda #0
    sta _g_mfs_open
    sta _g_mfs_buf_pos
    sta _g_mfs_buf_pos+1
    sta _g_mfs_buf_len
    sta _g_mfs_buf_len+1
    sta _g_mfs_eof
    jsr ROMAPI_SD_INIT
    cmp #0
    bne @fail
    jsr ROMAPI_MFS_MOUNT
    cmp #0
    bne @fail
    lda #1
    rts
@fail:
    lda #0
    rts

;; ======================================================================
;; mfs_close
;; ======================================================================
_mfs_close:
    lda _g_mfs_open
    beq @done
    jsr ROMAPI_MFS_CLOSE
    lda #0
    sta _g_mfs_open
    sta _g_mfs_buf_pos
    sta _g_mfs_buf_pos+1
    sta _g_mfs_buf_len
    sta _g_mfs_buf_len+1
    sta _g_mfs_eof
@done:
    rts

;; ======================================================================
;; mfs_read_line - Lee linea del archivo abierto
;; Retorna: 1 si hay linea, 0 si EOF/error
;; Usa $F4 como contador i
;; ======================================================================
_mfs_read_line:
    lda _g_mfs_open
    beq _eof
    lda _g_mfs_eof
    bne _eof
    lda #0
    sta $F4
_loop:
    lda _g_mfs_buf_pos
    cmp _g_mfs_buf_len
    lda _g_mfs_buf_pos+1
    sbc _g_mfs_buf_len+1
    bcc _get
    jsr _refill
    bcc _get
    lda $F4
    beq _eof
    jmp _eol
_get:
    lda _g_mfs_buf_pos
    ldx _g_mfs_buf_pos+1
    clc
    adc #<_g_mfs_buffer
    sta $F0
    txa
    adc #>_g_mfs_buffer
    sta $F1
    ldy #0
    lda ($F0),y
    inc _g_mfs_buf_pos
    bne :+
    inc _g_mfs_buf_pos+1
:
    cmp #$0A
    beq _eol
    cmp #$0D
    beq _loop
    cmp #0
    beq _nul
    ldy $F4
    cpy #31
    bcs _loop
    sta _g_line_buf,y
    inc $F4
    jmp _loop
_eol:
    ldy $F4
    lda #0
    sta _g_line_buf,y
    lda #1
    rts
_nul:
    ldy $F4
    lda #0
    sta _g_line_buf,y
    lda #1
    sta _g_mfs_eof
    lda $F4
    beq _eof
    lda #1
    rts
_eof:
    lda #0
    rts

_refill:
    lda #<_g_mfs_buffer
    sta $F0
    lda #>_g_mfs_buffer
    sta $F1
    lda #8
    sta $F2
    lda #0
    sta $F3
    jsr ROMAPI_MFS_READ
    sta _g_mfs_buf_len
    stx _g_mfs_buf_len+1
    lda #0
    sta _g_mfs_buf_pos
    sta _g_mfs_buf_pos+1
    lda _g_mfs_buf_len
    ora _g_mfs_buf_len+1
    bne :+
    lda #1
    sta _g_mfs_eof
    sec
    rts
:
    clc
    rts
