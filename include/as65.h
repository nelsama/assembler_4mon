/* ==========================================================================
   AS65.H - Assembler 6502 residente para Tang Nano 9K
   ========================================================================== */
#ifndef AS65_H
#define AS65_H

#include <stdint.h>
#include "romapi.h"

/* ==========================================================================
   CONSTANTES DEL SISTEMA
   ========================================================================== */
#define VERSION         "1.0"
#define MAX_LINE        32            /* Tamaño máximo de línea */
#define MAX_SYMBOLS     16            /* Máximo de etiquetas */
#define SYM_NAME_LEN    12            /* Largo máximo de nombre de etiqueta */
#define MAX_ARGS        1             /* Máximo args en .byte/.word */
#define BUFFER_SIZE     8             /* Buffer de lectura SD */

/* ==========================================================================
   CÓDIGOS DE INSTRUCCIÓN 6502
   ========================================================================== */
enum instr_code {
    I_ADC, I_AND, I_ASL,
    I_BCC, I_BCS, I_BEQ, I_BIT, I_BMI, I_BNE, I_BPL, I_BRK, I_BVC, I_BVS,
    I_CLC, I_CLD, I_CLI, I_CLV, I_CMP, I_CPX, I_CPY,
    I_DEC, I_DEX, I_DEY,
    I_EOR,
    I_INC, I_INX, I_INY,
    I_JMP, I_JSR,
    I_LDA, I_LDX, I_LDY, I_LSR,
    I_NOP,
    I_ORA,
    I_PHA, I_PHP, I_PLA, I_PLP,
    I_ROL, I_ROR, I_RTI, I_RTS,
    I_SBC, I_SEC, I_SED, I_SEI,
    I_STA, I_STX, I_STY,
    I_TAX, I_TAY, I_TSX, I_TXA, I_TXS, I_TYA,
    I_LAST
};

/* ==========================================================================
   MODOS DE DIRECCIONAMIENTO
   ========================================================================== */
#define MODE_IMPL       0    /* Implícito (RTS, NOP, TAX...) */
#define MODE_ACC        1    /* Acumulador (ASL A, LSR A...) */
#define MODE_IMM        2    /* Inmediato (#$xx) */
#define MODE_ZP         3    /* Zero page ($xx) */
#define MODE_ZPX        4    /* Zero page,X ($xx,X) */
#define MODE_ZPY        5    /* Zero page,Y ($xx,Y) */
#define MODE_ABS        6    /* Absoluto ($xxxx) */
#define MODE_ABSX       7    /* Absoluto,X ($xxxx,X) */
#define MODE_ABSY       8    /* Absoluto,Y ($xxxx,Y) */
#define MODE_INDX       9    /* (Indirecto,X)  — ($xx,X) */
#define MODE_INDY       10   /* (Indirecto),Y — ($xx),Y */
#define MODE_IND        11   /* Indirecto ($xxxx) — solo JMP */
#define MODE_REL        12   /* Relativo (branch target) */

/* ==========================================================================
   CÓDIGOS DE DIRECTIVA
   ========================================================================== */
#define DIR_ORG         1
#define DIR_BYTE        2
#define DIR_WORD        3
#define DIR_RES         4
#define DIR_INCBIN      5

/* ==========================================================================
   ESTRUCTURAS DE DATOS
   ========================================================================== */

/* Entrada de la tabla de opcodes */
typedef struct {
    uint8_t instr;     /* código de instrucción (enum instr_code) */
    uint8_t mode;      /* modo de direccionamiento */
    uint8_t opcode;    /* opcode 6502 */
    uint8_t bytes;     /* tamaño en bytes (1, 2 o 3) */
} opcode_entry_t;

/* Entrada de la tabla de símbolos */
typedef struct {
    char     name[SYM_NAME_LEN];  /* nombre de la etiqueta (relleno con 0) */
    uint16_t addr;                /* dirección */
} symbol_t;

/* Estado del ensamblado */
typedef struct {
    uint16_t pc;             /* program counter actual */
    uint16_t org_addr;       /* dirección base (.org) */
    uint16_t start_addr;     /* dirección del primer byte */
    uint16_t end_addr;       /* dirección del último byte + 1 */
    uint8_t  pass;           /* 1 o 2 */
    uint8_t  error;          /* flag de error */
    uint16_t line_num;       /* número de línea actual */
    uint16_t output_size;    /* tamaño del código generado */
} assembler_state_t;

/* ==========================================================================
   VARIABLES GLOBALES (en BSS)
   ========================================================================== */
extern symbol_t  g_symbols[MAX_SYMBOLS];
extern uint8_t   g_num_symbols;
extern char      g_line_buf[MAX_LINE];
extern char      g_label_buf[SYM_NAME_LEN];
extern char      g_mnemonic_buf[8];
extern char      g_operand_buf[20];
extern uint8_t   g_has_label;
extern uint8_t   g_has_operand;
extern uint8_t   g_is_directive;
extern uint8_t   g_directive;
extern uint16_t  g_directive_args[MAX_ARGS];
extern uint8_t   g_directive_argc;
extern uint8_t   g_is_constant;
extern uint16_t  g_constant_value;
extern uint8_t   g_instr;
extern uint8_t   g_mode;

/* Estado del archivo MFS */
extern uint8_t  g_mfs_open;
extern char     g_mfs_buffer[BUFFER_SIZE];
extern uint16_t g_mfs_buf_pos;
extern uint16_t g_mfs_buf_len;
extern uint8_t  g_mfs_eof;

/* ==========================================================================
   PROTOTIPOS - scanner.c
   ========================================================================== */
void    scan_init(void);
uint8_t scan_line(const char *line);
int16_t parse_expr(const char *str);
uint8_t parse_byte_args(const char *op, uint16_t *args, uint8_t max);
uint8_t parse_word_args(const char *op, uint16_t *args, uint8_t max);

/* ==========================================================================
   PROTOTIPOS - opcodes.c
   ========================================================================== */
void    opc_init(void);
uint8_t opc_find(uint8_t instr, uint8_t mode);
uint8_t opc_get_bytes(uint8_t instr, uint8_t mode);
uint8_t opc_instr_from_name(const char *name);
uint8_t opc_detect_mode(const char *operand, uint8_t instr);

/* ==========================================================================
   PROTOTIPOS - symbols.c
   ========================================================================== */
void    sym_init(void);
uint8_t sym_add(const char *name, uint16_t addr);
extern void    sym_remove_locals(void);
int16_t sym_find(const char *name);
uint8_t sym_get_count(void);

/* ==========================================================================
   PROTOTIPOS - mfs_io.c
   ========================================================================== */
uint8_t mfs_init(void);
uint8_t mfs_open_read(const char *filename);
uint8_t mfs_read_line(void);
void    mfs_close(void);
uint8_t mfs_save_bin(const char *filename, uint16_t addr, uint16_t size);

/* ==========================================================================
   PROTOTIPOS - main.c
   ========================================================================== */
void    put_str(const char *s);
void    put_chr(char c);
void    put_hex8(uint8_t val);
void    put_hex16(uint16_t val);
void    put_newline(void);
char    get_chr(void);
void    get_str(char *buf, uint8_t max);
uint8_t prompt_bin(const char *msg);

#endif /* AS65_H */
