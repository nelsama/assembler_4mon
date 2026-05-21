/* ==========================================================================
   AS65_OPCODES.C - Tabla de opcodes 6502 y detección de modos
   ========================================================================== */
#include "as65.h"

/* ==========================================================================
   TABLA DE OPCODES 6502 COMPLETA
   Formato: instrucción, modo, opcode, bytes
   Organizada por instrucción para búsqueda rápida
   ========================================================================== */
static const opcode_entry_t opcode_table[] = {
    /* ADC */
    { I_ADC, MODE_IMM,  0x69, 2 },
    { I_ADC, MODE_ZP,   0x65, 2 },
    { I_ADC, MODE_ZPX,  0x75, 2 },
    { I_ADC, MODE_ABS,  0x6D, 3 },
    { I_ADC, MODE_ABSX, 0x7D, 3 },
    { I_ADC, MODE_ABSY, 0x79, 3 },
    { I_ADC, MODE_INDX, 0x61, 2 },
    { I_ADC, MODE_INDY, 0x71, 2 },

    /* AND */
    { I_AND, MODE_IMM,  0x29, 2 },
    { I_AND, MODE_ZP,   0x25, 2 },
    { I_AND, MODE_ZPX,  0x35, 2 },
    { I_AND, MODE_ABS,  0x2D, 3 },
    { I_AND, MODE_ABSX, 0x3D, 3 },
    { I_AND, MODE_ABSY, 0x39, 3 },
    { I_AND, MODE_INDX, 0x21, 2 },
    { I_AND, MODE_INDY, 0x31, 2 },

    /* ASL */
    { I_ASL, MODE_ACC,  0x0A, 1 },
    { I_ASL, MODE_ZP,   0x06, 2 },
    { I_ASL, MODE_ZPX,  0x16, 2 },
    { I_ASL, MODE_ABS,  0x0E, 3 },
    { I_ASL, MODE_ABSX, 0x1E, 3 },

    /* Branches */
    { I_BCC, MODE_REL,  0x90, 2 },
    { I_BCS, MODE_REL,  0xB0, 2 },
    { I_BEQ, MODE_REL,  0xF0, 2 },
    { I_BMI, MODE_REL,  0x30, 2 },
    { I_BNE, MODE_REL,  0xD0, 2 },
    { I_BPL, MODE_REL,  0x10, 2 },
    { I_BVC, MODE_REL,  0x50, 2 },
    { I_BVS, MODE_REL,  0x70, 2 },

    /* BIT */
    { I_BIT, MODE_ZP,   0x24, 2 },
    { I_BIT, MODE_ABS,  0x2C, 3 },

    /* BRK */
    { I_BRK, MODE_IMPL, 0x00, 1 },

    /* CLC, CLD, CLI, CLV */
    { I_CLC, MODE_IMPL, 0x18, 1 },
    { I_CLD, MODE_IMPL, 0xD8, 1 },
    { I_CLI, MODE_IMPL, 0x58, 1 },
    { I_CLV, MODE_IMPL, 0xB8, 1 },

    /* CMP */
    { I_CMP, MODE_IMM,  0xC9, 2 },
    { I_CMP, MODE_ZP,   0xC5, 2 },
    { I_CMP, MODE_ZPX,  0xD5, 2 },
    { I_CMP, MODE_ABS,  0xCD, 3 },
    { I_CMP, MODE_ABSX, 0xDD, 3 },
    { I_CMP, MODE_ABSY, 0xD9, 3 },
    { I_CMP, MODE_INDX, 0xC1, 2 },
    { I_CMP, MODE_INDY, 0xD1, 2 },

    /* CPX */
    { I_CPX, MODE_IMM,  0xE0, 2 },
    { I_CPX, MODE_ZP,   0xE4, 2 },
    { I_CPX, MODE_ABS,  0xEC, 3 },

    /* CPY */
    { I_CPY, MODE_IMM,  0xC0, 2 },
    { I_CPY, MODE_ZP,   0xC4, 2 },
    { I_CPY, MODE_ABS,  0xCC, 3 },

    /* DEC */
    { I_DEC, MODE_ZP,   0xC6, 2 },
    { I_DEC, MODE_ZPX,  0xD6, 2 },
    { I_DEC, MODE_ABS,  0xCE, 3 },
    { I_DEC, MODE_ABSX, 0xDE, 3 },

    /* DEX, DEY */
    { I_DEX, MODE_IMPL, 0xCA, 1 },
    { I_DEY, MODE_IMPL, 0x88, 1 },

    /* EOR */
    { I_EOR, MODE_IMM,  0x49, 2 },
    { I_EOR, MODE_ZP,   0x45, 2 },
    { I_EOR, MODE_ZPX,  0x55, 2 },
    { I_EOR, MODE_ABS,  0x4D, 3 },
    { I_EOR, MODE_ABSX, 0x5D, 3 },
    { I_EOR, MODE_ABSY, 0x59, 3 },
    { I_EOR, MODE_INDX, 0x41, 2 },
    { I_EOR, MODE_INDY, 0x51, 2 },

    /* INC */
    { I_INC, MODE_ZP,   0xE6, 2 },
    { I_INC, MODE_ZPX,  0xF6, 2 },
    { I_INC, MODE_ABS,  0xEE, 3 },
    { I_INC, MODE_ABSX, 0xFE, 3 },

    /* INX, INY */
    { I_INX, MODE_IMPL, 0xE8, 1 },
    { I_INY, MODE_IMPL, 0xC8, 1 },

    /* JMP */
    { I_JMP, MODE_ABS,  0x4C, 3 },
    { I_JMP, MODE_IND,  0x6C, 3 },

    /* JSR */
    { I_JSR, MODE_ABS,  0x20, 3 },

    /* LDA */
    { I_LDA, MODE_IMM,  0xA9, 2 },
    { I_LDA, MODE_ZP,   0xA5, 2 },
    { I_LDA, MODE_ZPX,  0xB5, 2 },
    { I_LDA, MODE_ABS,  0xAD, 3 },
    { I_LDA, MODE_ABSX, 0xBD, 3 },
    { I_LDA, MODE_ABSY, 0xB9, 3 },
    { I_LDA, MODE_INDX, 0xA1, 2 },
    { I_LDA, MODE_INDY, 0xB1, 2 },

    /* LDX */
    { I_LDX, MODE_IMM,  0xA2, 2 },
    { I_LDX, MODE_ZP,   0xA6, 2 },
    { I_LDX, MODE_ZPY,  0xB6, 2 },
    { I_LDX, MODE_ABS,  0xAE, 3 },
    { I_LDX, MODE_ABSY, 0xBE, 3 },

    /* LDY */
    { I_LDY, MODE_IMM,  0xA0, 2 },
    { I_LDY, MODE_ZP,   0xA4, 2 },
    { I_LDY, MODE_ZPX,  0xB4, 2 },
    { I_LDY, MODE_ABS,  0xAC, 3 },
    { I_LDY, MODE_ABSX, 0xBC, 3 },

    /* LSR */
    { I_LSR, MODE_ACC,  0x4A, 1 },
    { I_LSR, MODE_ZP,   0x46, 2 },
    { I_LSR, MODE_ZPX,  0x56, 2 },
    { I_LSR, MODE_ABS,  0x4E, 3 },
    { I_LSR, MODE_ABSX, 0x5E, 3 },

    /* NOP */
    { I_NOP, MODE_IMPL, 0xEA, 1 },

    /* ORA */
    { I_ORA, MODE_IMM,  0x09, 2 },
    { I_ORA, MODE_ZP,   0x05, 2 },
    { I_ORA, MODE_ZPX,  0x15, 2 },
    { I_ORA, MODE_ABS,  0x0D, 3 },
    { I_ORA, MODE_ABSX, 0x1D, 3 },
    { I_ORA, MODE_ABSY, 0x19, 3 },
    { I_ORA, MODE_INDX, 0x01, 2 },
    { I_ORA, MODE_INDY, 0x11, 2 },

    /* PHA, PHP, PLA, PLP */
    { I_PHA, MODE_IMPL, 0x48, 1 },
    { I_PHP, MODE_IMPL, 0x08, 1 },
    { I_PLA, MODE_IMPL, 0x68, 1 },
    { I_PLP, MODE_IMPL, 0x28, 1 },

    /* ROL */
    { I_ROL, MODE_ACC,  0x2A, 1 },
    { I_ROL, MODE_ZP,   0x26, 2 },
    { I_ROL, MODE_ZPX,  0x36, 2 },
    { I_ROL, MODE_ABS,  0x2E, 3 },
    { I_ROL, MODE_ABSX, 0x3E, 3 },

    /* ROR */
    { I_ROR, MODE_ACC,  0x6A, 1 },
    { I_ROR, MODE_ZP,   0x66, 2 },
    { I_ROR, MODE_ZPX,  0x76, 2 },
    { I_ROR, MODE_ABS,  0x6E, 3 },
    { I_ROR, MODE_ABSX, 0x7E, 3 },

    /* RTI, RTS */
    { I_RTI, MODE_IMPL, 0x40, 1 },
    { I_RTS, MODE_IMPL, 0x60, 1 },

    /* SBC */
    { I_SBC, MODE_IMM,  0xE9, 2 },
    { I_SBC, MODE_ZP,   0xE5, 2 },
    { I_SBC, MODE_ZPX,  0xF5, 2 },
    { I_SBC, MODE_ABS,  0xED, 3 },
    { I_SBC, MODE_ABSX, 0xFD, 3 },
    { I_SBC, MODE_ABSY, 0xF9, 3 },
    { I_SBC, MODE_INDX, 0xE1, 2 },
    { I_SBC, MODE_INDY, 0xF1, 2 },

    /* SEC, SED, SEI */
    { I_SEC, MODE_IMPL, 0x38, 1 },
    { I_SED, MODE_IMPL, 0xF8, 1 },
    { I_SEI, MODE_IMPL, 0x78, 1 },

    /* STA */
    { I_STA, MODE_ZP,   0x85, 2 },
    { I_STA, MODE_ZPX,  0x95, 2 },
    { I_STA, MODE_ABS,  0x8D, 3 },
    { I_STA, MODE_ABSX, 0x9D, 3 },
    { I_STA, MODE_ABSY, 0x99, 3 },
    { I_STA, MODE_INDX, 0x81, 2 },
    { I_STA, MODE_INDY, 0x91, 2 },

    /* STX */
    { I_STX, MODE_ZP,   0x86, 2 },
    { I_STX, MODE_ZPY,  0x96, 2 },
    { I_STX, MODE_ABS,  0x8E, 3 },

    /* STY */
    { I_STY, MODE_ZP,   0x84, 2 },
    { I_STY, MODE_ZPX,  0x94, 2 },
    { I_STY, MODE_ABS,  0x8C, 3 },

    /* TAX, TAY, TSX, TXA, TXS, TYA */
    { I_TAX, MODE_IMPL, 0xAA, 1 },
    { I_TAY, MODE_IMPL, 0xA8, 1 },
    { I_TSX, MODE_IMPL, 0xBA, 1 },
    { I_TXA, MODE_IMPL, 0x8A, 1 },
    { I_TXS, MODE_IMPL, 0x9A, 1 },
    { I_TYA, MODE_IMPL, 0x98, 1 },
};

#define NUM_OPCODES (sizeof(opcode_table) / sizeof(opcode_entry_t))

/* ==========================================================================
   Nombres de instrucciones (en mayúsculas, orden = enum)
   ========================================================================== */
static const char *instr_names[] = {
    "ADC", "AND", "ASL",
    "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL", "BRK", "BVC", "BVS",
    "CLC", "CLD", "CLI", "CLV", "CMP", "CPX", "CPY",
    "DEC", "DEX", "DEY",
    "EOR",
    "INC", "INX", "INY",
    "JMP", "JSR",
    "LDA", "LDX", "LDY", "LSR",
    "NOP",
    "ORA",
    "PHA", "PHP", "PLA", "PLP",
    "ROL", "ROR", "RTI", "RTS",
    "SBC", "SEC", "SED", "SEI",
    "STA", "STX", "STY",
    "TAX", "TAY", "TSX", "TXA", "TXS", "TYA",
};

/* ==========================================================================
   toupper_local
   ========================================================================== */
static char toupper_local(char c) {
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}

/* ==========================================================================
   strcmp_local - Case insensitive
   ========================================================================== */
static uint8_t strcmp_local(const char *a, const char *b) {
    while (*a && *b) {
        if (toupper_local(*a) != toupper_local(*b)) return 1;
        a++; b++;
    }
    return (*a != *b) ? 1 : 0;
}

/* ==========================================================================
   strstr_local - Busca substring case-insensitive
   ========================================================================== */
static const char* strstr_local(const char *haystack, const char *needle) {
    const char *h, *n;
    if (*needle == '\0') return haystack;
    while (*haystack) {
        h = haystack; n = needle;
        while (*h && *n && toupper_local(*h) == toupper_local(*n)) {
            h++; n++;
        }
        if (*n == '\0') return haystack;
        haystack++;
    }
    return 0;
}

/* ==========================================================================
   strchr_local - Busca caracter en string
   ========================================================================== */
static const char* strchr_local(const char *s, char c) {
    while (*s) {
        if (*s == c) return s;
        s++;
    }
    return 0;
}

/* ==========================================================================
   is_space
   ========================================================================== */
static uint8_t is_space(char c) {
    return (c == ' ' || c == '\t');
}

/* ==========================================================================
   skip_spaces
   ========================================================================== */
static const char* skip_spaces(const char *p) {
    while (*p && is_space(*p)) p++;
    return p;
}

/* ==========================================================================
   is_ident - Caracter de identificador
   ========================================================================== */
static uint8_t is_ident(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_' || c == '@';
}

/* ==========================================================================
   is_digit
   ========================================================================== */
static uint8_t is_digit(char c) {
    return (c >= '0' && c <= '9');
}

/* ==========================================================================
   is_hex_char
   ========================================================================== */
static uint8_t is_hex_char(char c) {
    return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

/* ==========================================================================
   opc_init - Inicializa módulo de opcodes
   ========================================================================== */
void opc_init(void) {
    /* Nada que inicializar por ahora */
}

/* ==========================================================================
   opc_instr_from_name - Busca instrucción por nombre
   Retorna: código de instrucción (enum), o I_LAST si no encontrada
   ========================================================================== */
uint8_t opc_instr_from_name(const char *name) {
    uint8_t i;
    for (i = 0; i < I_LAST; i++) {
        if (strcmp_local(name, instr_names[i]) == 0) return i;
    }
    return I_LAST;
}

/* ==========================================================================
   opc_find - Busca opcode en la tabla
   ========================================================================== */
uint8_t opc_find(uint8_t instr, uint8_t mode) {
    uint8_t i;
    for (i = 0; i < NUM_OPCODES; i++) {
        if (opcode_table[i].instr == instr && opcode_table[i].mode == mode) {
            return opcode_table[i].opcode;
        }
    }
    return 0xFF;
}

/* ==========================================================================
   opc_get_bytes - Obtiene tamaño de instrucción
   Retorna: bytes (1, 2 o 3), o 0 si no encontrado
   ========================================================================== */
uint8_t opc_get_bytes(uint8_t instr, uint8_t mode) {
    uint8_t i;
    for (i = 0; i < NUM_OPCODES; i++) {
        if (opcode_table[i].instr == instr && opcode_table[i].mode == mode) {
            return opcode_table[i].bytes;
        }
    }
    return 0;
}

/* ==========================================================================
   is_zp_value - Determina si un valor numérico es zero page (< $100)
   p apunta al inicio del número. NO modifica p.
   ========================================================================== */
static uint8_t is_zp_value(const char *p) {
    const char *q;
    uint8_t d;
    uint16_t v;

    q = p;
    while (*q == ' ' || *q == '\t') q++;

    if (*q == '$') {
        q++;
        d = 0;
        while (is_hex_char(*q)) { d++; q++; }
        return (d <= 2);
    }
    if (is_digit(*q)) {
        v = 0;
        while (is_digit(*q)) { v = v * 10 + (*q - '0'); q++; }
        return (v < 256);
    }
    if (*q == '%') {
        q++;
        v = 0;
        while (*q == '0' || *q == '1') { v = (v << 1) | (*q - '0'); q++; }
        return (v < 256);
    }
    return 0;
}

/* ==========================================================================
   opc_detect_mode - Detecta modo de direccionamiento desde el operando
   ========================================================================== */
uint8_t opc_detect_mode(const char *operand, uint8_t instr) {
    const char *p;

    /* Vacío → implícito o acumulador según instrucción */
    if (operand == 0 || *operand == '\0') {
        if (instr == I_ASL || instr == I_LSR ||
            instr == I_ROL || instr == I_ROR) {
            return MODE_ACC;
        }
        return MODE_IMPL;
    }

    p = skip_spaces(operand);

    /* "A" → acumulador */
    if (toupper_local(*p) == 'A' && (*(p+1) == '\0' || is_space(*(p+1)))) {
        if (instr == I_ASL || instr == I_LSR ||
            instr == I_ROL || instr == I_ROR) {
            return MODE_ACC;
        }
    }

    /* # → inmediato */
    if (*p == '#') return MODE_IMM;

    /* ( → indirecto */
    if (*p == '(') {
        if (strstr_local(p, ",X)") || strstr_local(p, ",x)")) return MODE_INDX;
        if (strstr_local(p, "),Y") || strstr_local(p, "),y")) return MODE_INDY;
        if (strchr_local(p, ')')) return MODE_IND;
    }

    /* ,X → indexado por X */
    if (strstr_local(p, ",X") || strstr_local(p, ",x")) {
        return is_zp_value(p) ? MODE_ZPX : MODE_ABSX;
    }

    /* ,Y → indexado por Y */
    if (strstr_local(p, ",Y") || strstr_local(p, ",y")) {
        return is_zp_value(p) ? MODE_ZPY : MODE_ABSY;
    }

    /* Valor simple */
    if (*p == '$' || is_digit(*p) || *p == '%') {
        return is_zp_value(p) ? MODE_ZP : MODE_ABS;
    }

    /* Etiqueta u otro identificador */
    if (is_ident(*p)) {
        if (instr == I_JMP || instr == I_JSR) return MODE_ABS;
        if (instr == I_BCC || instr == I_BCS || instr == I_BEQ ||
            instr == I_BMI || instr == I_BNE || instr == I_BPL ||
            instr == I_BVC || instr == I_BVS) return MODE_REL;
        {   int16_t val = sym_find(p);
            if (val >= 0 && val < 256) return MODE_ZP;
        }
        return MODE_ABS;
    }

    return MODE_IMPL;
}
