/* ==========================================================================
   AS65_SYMBOLS.C - Tabla de símbolos (etiquetas)
   ========================================================================== */
#include "as65.h"

/* Tabla de símbolos en BSS */
symbol_t g_symbols[MAX_SYMBOLS];
uint8_t  g_num_symbols;

/* sym_init en io_rom.s */

/* ==========================================================================
   toupper_local
   ========================================================================== */
static char toupper_local(char c) {
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}

/* ==========================================================================
   strncmp_local - Comparación case-insensitive con límite
   ========================================================================== */
static uint8_t strncmp_local(const char *a, const char *b, uint8_t n) {
    uint8_t i;
    for (i = 0; i < n; i++) {
        if (a[i] == '\0' && b[i] == '\0') return 0;
            if (toupper_local(a[i]) != toupper_local(b[i])) return 1;
    }
    return 0;
}

/* ==========================================================================
   strncpy_local
   ========================================================================== */
static void strncpy_local(char *dst, const char *src, uint8_t n) {
    uint8_t i;
    for (i = 0; i < n && *src; i++) {
        *dst++ = toupper_local(*src);
        src++;
    }
    for (; i < n; i++) *dst++ = '\0';
}

/* ==========================================================================
   sym_add - Agrega un símbolo a la tabla
   Retorna: 1 si éxito, 0 si tabla llena o duplicado
   ========================================================================== */
uint8_t sym_add(const char *name, uint16_t addr) {
    char upper[SYM_NAME_LEN];
    uint8_t i;

    if (g_num_symbols >= MAX_SYMBOLS) return 0;

    /* Convertir a mayúsculas */
    strncpy_local(upper, name, SYM_NAME_LEN);

    /* Buscar si ya existe */
    for (i = 0; i < g_num_symbols; i++) {
        if (strncmp_local(g_symbols[i].name, upper, SYM_NAME_LEN) == 0) {
            /* Etiqueta @: reemplazar (permite @loop en distintas funciones) */
            if (upper[0] == '@') {
                g_symbols[i].addr = addr;
                return 1;
            }
            return 0; /* duplicado no-local */
        }
    }

    /* Agregar nuevo */
    strncpy_local(g_symbols[g_num_symbols].name, upper, SYM_NAME_LEN);
    g_symbols[g_num_symbols].addr = addr;
    g_num_symbols++;
    return 1;
}

/* ==========================================================================
   sym_remove_locals - Elimina todos los símbolos que empiezan con @
   Se llama cuando se encuentra una etiqueta no-local
   ========================================================================== */
void sym_remove_locals(void) {
    uint8_t i = 0;
    while (i < g_num_symbols) {
        if (g_symbols[i].name[0] == '@') {
            uint8_t j;
            for (j = i; j < g_num_symbols - 1; j++) {
                g_symbols[j] = g_symbols[j + 1];
            }
            g_num_symbols--;
        } else {
            i++;
        }
    }
}

/* ==========================================================================
   sym_find - Busca un símbolo por nombre
   Retorna: dirección del símbolo, o -1 si no encontrado
   ========================================================================== */
int16_t sym_find(const char *name) {
    uint8_t i;
    for (i = 0; i < g_num_symbols; i++) {
        if (strncmp_local(g_symbols[i].name, name, SYM_NAME_LEN) == 0) {
            return (int16_t)g_symbols[i].addr;
        }
    }
    return -1;
}

/* sym_get_count en io_rom.s */
