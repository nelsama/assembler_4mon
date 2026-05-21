/* ==========================================================================
   AS65_SCAN.C - Scanner / tokenizador de líneas
   ========================================================================== */
#include "as65.h"

/* Variables globales del scanner (definidas aquí) */
char      g_line_buf[MAX_LINE];
char      g_label_buf[SYM_NAME_LEN];
char      g_mnemonic_buf[8];
char      g_operand_buf[20];
uint8_t   g_has_label;
uint8_t   g_has_operand;
uint8_t   g_is_directive;
uint8_t   g_directive;
uint16_t  g_directive_args[MAX_ARGS];
uint8_t   g_directive_argc;
uint8_t   g_is_constant;
uint16_t  g_constant_value;
uint8_t   g_instr;
uint8_t   g_mode;

/* ==========================================================================
   scan_init - Inicializa el scanner
   ========================================================================== */
/* scan_init en io_rom.s */

/* ==========================================================================
   toupper_local - Conversión a mayúscula (evita dependencia de ctype)
   ========================================================================== */
static char toupper_local(char c) {
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}

/* ==========================================================================
   is_space - Detecta espacio/tab
   ========================================================================== */
static uint8_t is_space(char c) {
    return (c == ' ' || c == '\t');
}

/* ==========================================================================
   is_hex_char - Detecta dígito hexadecimal
   ========================================================================== */
static uint8_t is_hex_char(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

/* ==========================================================================
   is_digit - Detecta dígito decimal
   ========================================================================== */
static uint8_t is_digit(char c) {
    return (c >= '0' && c <= '9');
}

/* ==========================================================================
   is_ident_start - Inicio de identificador (letra o _)
   ========================================================================== */
static uint8_t is_ident_start(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_' || c == '@';
}

/* ==========================================================================
   is_ident - Caracter de identificador
   ========================================================================== */
static uint8_t is_ident(char c) {
    return is_ident_start(c) || is_digit(c);
}

/* ==========================================================================
   strcmp_local - Compara strings (case-insensitive)
   Retorna 0 si son iguales
   ========================================================================== */
static uint8_t strcmp_local(const char *a, const char *b) {
    while (*a && *b) {
        if (toupper_local(*a) != toupper_local(*b)) return 1;
        a++; b++;
    }
    return (*a != *b) ? 1 : 0;
}

/* ==========================================================================
   skip_spaces - Avanza puntero sobre espacios
   ========================================================================== */
static const char* skip_spaces(const char *p) {
    while (*p && is_space(*p)) p++;
    return p;
}

/* ==========================================================================
   parse_number - Parsea un número: decimal, $hex, %binary, 'char'
   Retorna el valor numérico, o -1 si error
   ========================================================================== */
static int16_t parse_number(const char **pp) {
    const char *p = *pp;
    int16_t val = 0;
    uint8_t nibble;

    if (*p == '\0') return -1;

    /* Caracter en comillas simples */
    if (*p == '\'') {
        p++;
        if (*p == '\0' || *(p+1) != '\'') return -1;
        val = *p;
        p += 2;
        *pp = p;
        return val;
    }

    /* Hexadecimal: $xx */
    if (*p == '$') {
        p++;
        val = 0;
        while (is_hex_char(*p)) {
            val <<= 4;
            if (*p >= '0' && *p <= '9') nibble = *p - '0';
            else if (*p >= 'a' && *p <= 'f') nibble = *p - 'a' + 10;
            else nibble = *p - 'A' + 10;
            val |= nibble;
            p++;
        }
        *pp = p;
        return val;
    }

    /* Binario: %0101 */
    if (*p == '%') {
        p++;
        val = 0;
        while (*p == '0' || *p == '1') {
            val = (val << 1) | (*p - '0');
            p++;
        }
        *pp = p;
        return val;
    }

    /* Decimal */
    if (is_digit(*p)) {
        val = 0;
        while (is_digit(*p)) {
            val = val * 10 + (*p - '0');
            p++;
        }
        *pp = p;
        return val;
    }

    /* Es un identificador (etiqueta) - retornar -1, el llamador debe resolver */
    if (is_ident_start(*p)) {
        *pp = p;
        return -2; /* indica que es un símbolo */
    }

    return -1;
}

/* ==========================================================================
   parse_expr - Parsea expresión simple: número, símbolo, o símbolo±cte
   ========================================================================== */
int16_t parse_expr(const char *str) {
    const char *p = str;
    int16_t val;
    int16_t offset = 0;
    char sym_name[SYM_NAME_LEN];
    uint8_t i;

    p = skip_spaces(p);
    if (*p == '\0') return -1;

    /* Intentar parsear número directamente */
    val = parse_number(&p);
    if (val >= 0 || val <= -3) {
        /* Número válido (incluso > 32767 como $C001) */
        val = (int16_t)(uint16_t)val;
        p = skip_spaces(p);
        if (*p == '+' || *p == '-') {
            int8_t sign = (*p == '+') ? 1 : -1;
            p++;
            p = skip_spaces(p);
            offset = parse_number(&p);
            if (offset < 0) return -1;
            val += sign * offset;
        }
        return val;
    }

    /* Si es un identificador, copiar nombre */
    if (val == -2) {
        i = 0;
        while (is_ident(*p) && i < SYM_NAME_LEN - 1) {
            sym_name[i++] = toupper_local(*p);
            p++;
        }
        sym_name[i] = '\0';

        /* Buscar en tabla de símbolos */
        val = sym_find(sym_name);
        if (val == -1) return -1; /* símbolo no encontrado */

        /* Posible + o - después */
        p = skip_spaces(p);
        if (*p == '+' || *p == '-') {
            int8_t sign = (*p == '+') ? 1 : -1;
            p++;
            p = skip_spaces(p);
            offset = parse_number(&p);
            if (offset < 0) return -1;
            val += sign * offset;
        }
        return val;
    }

    return -1;
}

/* ==========================================================================
   parse_string_arg - Parsea un argumento string entre comillas
   Devuelve los bytes en args y la cantidad. Máximo 8 chars por ahora.
   ========================================================================== */
static uint8_t parse_string_arg(const char **pp, uint16_t *args, uint8_t max) {
    const char *p = *pp;
    uint8_t count = 0;

    if (*p != '"') return 0;
    p++; /* saltar " */

    while (*p && *p != '"' && count < max) {
        args[count++] = (uint8_t)(*p);
        p++;
    }

    if (*p == '"') p++; /* saltar " cierre */
    *pp = p;
    return count;
}

/* ==========================================================================
   parse_byte_args - Parsea argumentos de .byte
   Soporta: números, 'chars', "strings", separados por coma
   ========================================================================== */
uint8_t parse_byte_args(const char *op, uint16_t *args, uint8_t max) {
    const char *p = op;
    uint8_t count = 0;
    int16_t val;
    uint8_t str_count;

    if (*p == '\0') return 0;

    while (*p && count < max) {
        p = skip_spaces(p);
        if (*p == '\0') break;

        /* String entre comillas */
        if (*p == '"') {
            str_count = parse_string_arg(&p, args + count, max - count);
            count += str_count;
        } else {
            val = parse_number(&p);
            if (val >= 0) {
                if (count < max) args[count++] = (uint16_t)(val & 0xFF);
            } else {
                break; /* error */
            }
        }

        p = skip_spaces(p);
        if (*p == ',') p++;
        else break;
    }

    return count;
}

/* ==========================================================================
   parse_word_args - Parsea argumentos de .word (16 bits)
   ========================================================================== */
uint8_t parse_word_args(const char *op, uint16_t *args, uint8_t max) {
    const char *p = op;
    uint8_t count = 0;
    int16_t val;

    if (*p == '\0') return 0;

    while (*p && count < max) {
        p = skip_spaces(p);
        if (*p == '\0') break;

        val = parse_expr(p);
        if (val >= 0) {
            args[count++] = (uint16_t)val;
            /* Avanzar p después del número parseado */
            while (is_hex_char(*p) || is_digit(*p)) p++;
        } else if (val == -2) {
            /* Es un símbolo, copiarlo hasta coma */
            uint8_t i = 0;
            char sym_buf[SYM_NAME_LEN];
            while (is_ident(*p) && i < SYM_NAME_LEN - 1) {
                sym_buf[i++] = toupper_local(*p);
                p++;
            }
            sym_buf[i] = '\0';
            val = sym_find(sym_buf);
            if (val >= 0) {
                args[count++] = (uint16_t)val;
            } else {
                args[count++] = 0; /* forward ref, poner 0 */
            }
        } else {
            break;
        }

        p = skip_spaces(p);
        if (*p == ',') p++;
        else break;
    }

    return count;
}

/* ==========================================================================
   scan_line - Procesa una línea de assembler
   Extrae: etiqueta, mnémico/directiva, operandos
   Retorna: 1 si es una línea válida, 0 si vacía o solo comentario
   ========================================================================== */
uint8_t scan_line(const char *line) {
    const char *p = line;
    uint8_t i;

    /* Inicializar */
    scan_init();
    g_label_buf[0] = '\0';
    g_mnemonic_buf[0] = '\0';
    g_operand_buf[0] = '\0';
    g_has_label = 0;
    g_has_operand = 0;
    g_is_directive = 0;
    g_directive = 0;
    g_directive_argc = 0;
    g_instr = I_LAST;
    g_mode = 0;

    p = skip_spaces(p);

    /* Línea vacía */
    if (*p == '\0') return 0;

    /* Comentario */
    if (*p == ';') return 0;

    /* --- ¿Directiva? (línea empieza con '.') --- */
    if (*p == '.') {
        p++;
        g_is_directive = 1;

        /* Leer nombre de directiva */
        i = 0;
        while (is_ident(*p) && i < 7) {
            g_mnemonic_buf[i++] = toupper_local(*p);
            p++;
        }
        g_mnemonic_buf[i] = '\0';

        /* Identificar directiva */
        if (strcmp_local(g_mnemonic_buf, "ORG") == 0) g_directive = DIR_ORG;
        else if (strcmp_local(g_mnemonic_buf, "BYTE") == 0) g_directive = DIR_BYTE;
        else if (strcmp_local(g_mnemonic_buf, "WORD") == 0) g_directive = DIR_WORD;
        else if (strcmp_local(g_mnemonic_buf, "RES") == 0) g_directive = DIR_RES;
        else if (strcmp_local(g_mnemonic_buf, "INCBIN") == 0) g_directive = DIR_INCBIN;
        else return 0; /* directiva desconocida, ignorar */

        /* Parsear operandos */
        p = skip_spaces(p);
        if (*p != '\0' && *p != ';') {
            /* Copiar operandos */
            i = 0;
            while (*p && *p != ';' && i < 19) {
                g_operand_buf[i++] = *p++;
            }
            g_operand_buf[i] = '\0';

            /* Parsear según directiva */
            g_operand_buf[i] = '\0';
            if (g_directive == DIR_BYTE) {
                g_directive_argc = parse_byte_args(g_operand_buf, g_directive_args, MAX_ARGS);
            } else if (g_directive == DIR_WORD) {
                g_directive_argc = parse_word_args(g_operand_buf, g_directive_args, MAX_ARGS);
            } else if (g_directive == DIR_ORG || g_directive == DIR_RES) {
                g_directive_args[0] = (uint16_t)parse_expr(g_operand_buf);
                g_directive_argc = 1;
            }
        }
        return 1;
    }

    /* --- ¿Etiqueta? --- */
    {
        const char *pe = p;
        uint8_t has_colon = 0;

        /* Buscar el primer espacio o ':' */
        while (*pe && !is_space(*pe) && *pe != ':' && *pe != ';') pe++;

        if (*pe == ':') {
            has_colon = 1;
        } else {
            /* Podría ser etiqueta sin ':' si es un identificador seguido de espacio
               y luego un mnémico conocido */
            has_colon = 0;
        }

        /* Solo detectar etiqueta si termina en ':' */
        if (has_colon) {
            /* Extraer etiqueta */
            i = 0;
            while (p < pe && i < SYM_NAME_LEN - 1) {
                g_label_buf[i++] = toupper_local(*p);
                p++;
            }
            g_label_buf[i] = '\0';
            g_has_label = 1;

            p = pe + 1; /* saltar ':' */
            p = skip_spaces(p);
        }
    }

    /* --- ¿Constante? (NOMBRE = VALOR) --- */
    if (!g_has_label && is_ident_start(*p)) {
        const char *peq = p;
        const char *peq2 = 0;
        while (*peq && *peq != ';' && *peq != '=' && !is_space(*peq)) peq++;
        if (*peq == '=') {
            peq2 = peq;
        } else if (is_space(*peq)) {
            peq2 = peq;
            while (*peq2 && *peq2 != '=') peq2++;
            if (*peq2 != '=') peq2 = 0;
        }
        if (peq2) {
            i = 0;
            while (p < peq && i < SYM_NAME_LEN - 1) {
                g_label_buf[i++] = toupper_local(*p);
                p++;
            }
            g_label_buf[i] = '\0';
            if (i > 0) {
                g_has_label = 1;
                g_is_constant = 1;
                p = peq2 + 1;
                p = skip_spaces(p);
                i = 0;
                while (*p && *p != ';' && i < 19) {
                    g_operand_buf[i++] = *p++;
                }
                g_operand_buf[i] = '\0';
                g_constant_value = (uint16_t)parse_expr(g_operand_buf);
                g_instr = I_LAST;
                return 1;
            }
        }
    }

    /* --- ¿Mnémico? --- */
    p = skip_spaces(p);
    if (*p == '\0' || *p == ';') return 1; /* solo etiqueta, sin instrucciÃ³n */

    /* Extraer mnémico */
    i = 0;
    while (is_ident(*p) && i < 7) {
        g_mnemonic_buf[i++] = toupper_local(*p);
        p++;
    }
    g_mnemonic_buf[i] = '\0';

    /* Buscar si es instrucción conocida */
    g_instr = opc_instr_from_name(g_mnemonic_buf);

    /* --- ¿Operandos? --- */
    p = skip_spaces(p);
    if (*p != '\0' && *p != ';') {
        i = 0;
        while (*p && *p != ';' && i < 19) {
            g_operand_buf[i++] = *p++;
        }
        g_operand_buf[i] = '\0';
        g_has_operand = 1;

        /* Detectar modo de direccionamiento */
        if (g_instr < I_LAST) {
            g_mode = opc_detect_mode(g_operand_buf, g_instr);
        }
    } else {
        g_has_operand = 0;
        g_operand_buf[0] = '\0';
        /* Instrucción sin operandos: modo implícito o acumulador */
        if (g_instr < I_LAST) {
            g_mode = opc_detect_mode("", g_instr);
        }
    }

    return 1;
}
