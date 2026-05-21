/* ==========================================================================
   AS65_MAIN.C - Ensamblador 6502 residente para Tang Nano 9K
   ========================================================================== */
#include "as65.h"

/* Estado global del ensamblado */
assembler_state_t g_state;

static char g_save_name[32];

/* I/O en io_rom.s (ASM) */

/* ==========================================================================
   get_str - Lee un string por UART hasta Enter
   ========================================================================== */
void get_str(char *buf, uint8_t max) {
    uint8_t i = 0;
    char c;

    while (1) {
        c = get_chr();
        if (c == '\r' || c == '\n') {
            put_newline();
            break;
        }
        if (c == '\b' || c == 127) {
            if (i > 0) {
                i--;
                put_str("\b \b");
            }
            continue;
        }
        if (i < max - 1) {
            buf[i++] = c;
            put_chr(c);
        }
    }
    buf[i] = '\0';
}



/* Buffer de salida ($0464, 508 bytes, se escribe a SD tras cerrar fuente) */
#define OUT_BUF  0x3C00

static uint8_t g_out_buf[32];
static uint16_t g_out_pos;

/* ==========================================================================
   out_open - Prepara buffer
   ========================================================================== */
static void out_open(void) {
    g_out_pos = 0;
}

/* ==========================================================================
   out_write_byte - Acumula byte en buffer
   ========================================================================== */
static void out_write_byte(uint8_t byte) {
    *(uint8_t*)(OUT_BUF + g_out_pos) = byte;
    g_out_pos++;
    g_state.pc++;
    g_state.output_size++;
}

/* ==========================================================================
   out_save - Guarda buffer a SD via MFS (fuente ya cerrada)
   ========================================================================== */
static uint8_t out_save(const char *name, uint16_t size) {
    uint8_t i;
    for (i = 0; i < 31 && name[i]; i++) {
        char c = name[i];
        g_save_name[i] = (c >= 'a' && c <= 'z') ? c - 32 : c;
    }
    g_save_name[i] = '\0';
    rom_mfs_delete(g_save_name);
    if (rom_mfs_create_via_zp(g_save_name, size) != MFS_OK) return 0;
    {
        uint16_t remain = size;
        while (remain > 0) {
            uint16_t chunk = (remain > 32) ? 32 : remain;
            for (i = 0; i < chunk; i++) {
                g_out_buf[i] = *(uint8_t*)(OUT_BUF + size - remain + i);
            }
            rom_mfs_write_via_zp(g_out_buf, chunk);
            remain -= chunk;
        }
    }
    rom_mfs_close();
    return 1;
}


/* ==========================================================================
   out_write_word - Acumula un word (little-endian)
   ========================================================================== */
static void out_write_word(uint16_t word) {
    out_write_byte((uint8_t)(word & 0xFF));
    out_write_byte((uint8_t)(word >> 8));
}

/* ==========================================================================
   process_line_pass1 - Procesa una línea en la pasada 1
   ========================================================================== */
static void process_line_pass1(void) {
    uint16_t lab_addr;

    if (!scan_line(g_line_buf)) return;

    if (g_is_directive) {
        switch (g_directive) {
            case DIR_ORG:
                g_state.org_addr = g_directive_args[0];
                g_state.pc = g_state.org_addr;
                g_state.start_addr = g_state.org_addr;
                break;
            case DIR_BYTE:
                g_state.pc += g_directive_argc;
                break;
            case DIR_WORD:
                g_state.pc += g_directive_argc * 2;
                break;
            case DIR_RES:
                g_state.pc += g_directive_args[0];
                break;
        }
        return;
    }

    if (g_has_label) {
        if (g_label_buf[0] != '\0') {
            lab_addr = g_is_constant ? g_constant_value : g_state.pc;
            if (sym_add(g_label_buf, lab_addr)) {
                put_str("  ");
                put_str(g_label_buf);
                put_str(" = $");
                put_hex16(lab_addr);
                put_newline();
            }
        }
    }

    if (g_instr < I_LAST) {
        uint8_t bytes = opc_get_bytes(g_instr, g_mode);
        if (bytes == 0) {
            put_str("ERR bad mode ");
            put_str(g_mnemonic_buf);
            put_newline();
            g_state.error = 1;
        } else {
            g_state.pc += bytes;
        }
    }
}

/* ==========================================================================
   process_line_pass2 - Procesa una línea en la pasada 2 (genera código)
   ========================================================================== */
static void process_line_pass2(void) {
    uint16_t line_pc;
    uint8_t opcode, bytes;
    int16_t val;
    int16_t offset;
    uint8_t i;

    if (!scan_line(g_line_buf)) return;

    line_pc = g_state.pc;

    if (g_is_directive) {
        switch (g_directive) {
            case DIR_ORG:
                g_state.pc = g_directive_args[0];
                line_pc = g_state.pc;
                break;
            case DIR_BYTE:
                for (i = 0; i < g_directive_argc; i++) {
                    out_write_byte((uint8_t)g_directive_args[i]);
                }
                break;
            case DIR_WORD:
                for (i = 0; i < g_directive_argc; i++) {
                    out_write_word(g_directive_args[i]);
                }
                break;
            case DIR_RES:
                g_state.pc += g_directive_args[0];
                break;
        }
    }

    if (g_instr >= I_LAST) return;

    opcode = opc_find(g_instr, g_mode);
    bytes = opc_get_bytes(g_instr, g_mode);

    if (opcode == 0xFF || bytes == 0) {
        put_str("ERR bad op ");
        put_str(g_mnemonic_buf);
        put_newline();
        g_state.error = 1;
        return;
    }

    out_write_byte(opcode);

    switch (g_mode) {
        case MODE_IMPL:
        case MODE_ACC:
            break;

        case MODE_IMM:
            val = parse_expr(g_operand_buf + 1);
            if (val == -1) {
                put_str("ERR invalid\r\n");
                g_state.error = 1;
                return;
            }
            out_write_byte((uint8_t)(val & 0xFF));
            break;

        case MODE_ZP:
        case MODE_ZPX:
        case MODE_ZPY:
            val = parse_expr(g_operand_buf);
            if (val == -1) {
                put_str("ERR invalid\r\n");
                g_state.error = 1;
                return;
            }
            out_write_byte((uint8_t)(val & 0xFF));
            break;

        case MODE_ABS:
        case MODE_ABSX:
        case MODE_ABSY:
            val = parse_expr(g_operand_buf);
            if (val == -1) {
                put_str("ERR no lbl ");
                put_str(g_operand_buf);
                put_str("\r\n");
                g_state.error = 1;
                return;
            }
            out_write_word((uint16_t)val);
            break;

        case MODE_INDX:
        case MODE_INDY:
        case MODE_IND: {
            uint8_t pos = 0;
            char num_buf[32];
            const char *p2 = g_operand_buf;
            if (*p2 == '(') p2++;
            while (*p2 && *p2 != ',' && *p2 != ')' && *p2 != ' ' && pos < 15) {
                num_buf[pos++] = *p2++;
            }
            num_buf[pos] = '\0';
            val = parse_expr(num_buf);
            if (val == -1) {
                put_str("ERR invalid\r\n");
                g_state.error = 1;
                return;
            }
            if (g_mode == MODE_IND) {
                out_write_word((uint16_t)val);
            } else {
                out_write_byte((uint8_t)(val & 0xFF));
            }
            break;
        }

        case MODE_REL: {
            val = parse_expr(g_operand_buf);
            if (val == -1) {
                put_str("ERR no lbl ");
                put_str(g_operand_buf);
                put_newline();
                g_state.error = 1;
                return;
            }
            offset = val - (int16_t)(g_state.pc + 1);
            out_write_byte((uint8_t)(offset & 0xFF));
            break;
        }
    }
}

/* ==========================================================================
   assemble_file - Ensambla un archivo completo (pasadas 1 y 2)
   ========================================================================== */
static void assemble_file(const char *filename) {
    uint16_t total_size;
    char srcname[32];
    char outname[32];
    uint8_t si;

    /* Copiar filename a buffer local (ya viene en mayusculas) */
    for (si = 0; si < 31 && filename[si]; si++) srcname[si] = filename[si];
    srcname[si] = '\0';

    g_state.pc = 0;
    g_state.org_addr = 0;
    g_state.start_addr = 0;
    g_state.end_addr = 0;
    g_state.pass = 1;
    g_state.error = 0;
    g_state.line_num = 0;
    g_state.output_size = 0;

    sym_init();
    if (!mfs_open_read(srcname)) {
        put_str("Not found: ");
        put_str(srcname);
        put_str("\r\n");
        return;
    }
    put_str("Pass 1\r\n");

    while (mfs_read_line()) {
        g_state.line_num++;
        process_line_pass1();
        if (g_state.error) {
            put_str("Line ");
            put_hex16(g_state.line_num);
            put_newline();
            mfs_close();
            return;
        }
    }
    mfs_close();

    g_state.end_addr = g_state.pc;

    put_str("  ");
    put_hex8(sym_get_count());
    put_str(" sym");
    put_newline();
    put_str(" Size ");
    put_hex16(g_state.end_addr - g_state.start_addr);
    put_str("b");
    put_newline();

    if (g_state.start_addr == 0 && g_state.end_addr == 0) {
        put_str("ERR No ORG\r\n");
        return;
    }

    total_size = g_state.end_addr - g_state.start_addr;
    if (total_size == 0) {
        put_str("ERR Empty\r\n");
        return;
    }

    /* ========== PASADA 2 ========== */
    put_str("Pass 2\r\n");

    /* Preguntar nombre de archivo de salida */
    put_str("Output file: ");
    get_str(outname, 28);

    g_state.pass = 2;
    g_state.pc = g_state.start_addr;
    g_state.line_num = 0;
    g_state.error = 0;
    g_state.output_size = 0;

    out_open();

    if (!mfs_open_read(srcname)) {
        put_str("Not found\r\n");
        return;
    }

    while (mfs_read_line()) {
        g_state.line_num++;
        process_line_pass2();
        if (g_state.error) {
            put_str("Line ");
            put_hex16(g_state.line_num);
            put_newline();
            mfs_close();
            return;
        }
    }
    mfs_close();

    if (!out_save(outname, g_state.output_size)) {
        put_str("ERR Save\r\n");
        return;
    }

    /* Imprimir sin usar RODATA (podria estar sobreescrita por el output) */
    put_chr('\r'); put_chr('\n');
    put_chr('O'); put_chr('K'); put_chr('!'); put_chr(' ');
    put_hex16(g_state.output_size);
    put_chr('b'); put_chr('\r'); put_chr('\n');
    put_chr('L'); put_chr('O'); put_chr('A'); put_chr('D'); put_chr(' ');
    put_str(outname);
    put_chr(' ');
    put_hex16(g_state.start_addr);
    put_chr('\r'); put_chr('\n');
    put_chr('R'); put_chr(' ');
    put_hex16(g_state.start_addr);
    put_chr('\r'); put_chr('\n');
}


/* ==========================================================================
   cmd_list - Lee lista de archivos desde filetab en RAM
   ========================================================================== */
static void cmd_list(void) {
    uint8_t idx, ci;
    uint8_t found = 0;
    uint8_t *ent;

    put_str("SD:\r\n");
    for (idx = 0; idx < 16; idx++) {
        ent = (uint8_t*)(0x0254 + 16 + (idx * 32));
        if (ent[0] == 0) continue;
        put_str("  ");
        for (ci = 0; ci < 11 && ent[ci]; ci++) {
            if (ent[ci] >= ' ') put_chr(ent[ci]);
        }
        put_str(" (");
        put_hex16(ent[14] | ((uint16_t)ent[15] << 8));
        put_str("b)\r\n");
        found = 1;
    }
    if (!found) put_str("  -\r\n");
}

/* ==========================================================================
   show_prompt
   ========================================================================== */
static void show_prompt(void) {
    put_chr('\r'); put_chr('\n');
    put_chr(' '); put_chr('['); put_chr('A'); put_chr(']');
    put_chr('s'); put_chr('s'); put_chr('e'); put_chr('m');
    put_chr(' '); put_chr('['); put_chr('L'); put_chr(']');
    put_chr('i'); put_chr('s'); put_chr('t');
    put_chr(' '); put_chr('['); put_chr('Q'); put_chr(']');
    put_chr('u'); put_chr('i'); put_chr('t');
    put_chr('\r'); put_chr('\n'); put_chr('>'); put_chr(' ');
}

/* ==========================================================================
   main
   ========================================================================== */
int main(void) {
    char cmd;
    char fname[32];

    opc_init();
    scan_init();

    put_str("\033[2J\033[H");
    put_str("AS65 v");
    put_str(VERSION);
    put_str("\r\n");

    if (!mfs_init()) {
        put_str("ERR SD\r\n");
        return 0;
    }
    put_str("SD OK\r\n");
    put_newline();

    while (1) {
        show_prompt();
        cmd = get_chr();
        put_chr(cmd);
        put_newline();

        switch (cmd) {
            case 'A':
            case 'a':
                put_chr('S'); put_chr('o'); put_chr('u'); put_chr('r'); put_chr('c'); put_chr('e'); put_chr(':'); put_chr(' ');
                get_str(fname, 28);
                assemble_file(fname);
                break;

            case 'L':
            case 'l':
                cmd_list();
                break;

            case 'Q':
            case 'q':
                put_chr('B'); put_chr('y'); put_chr('e'); put_chr('\r'); put_chr('\n');
                return 0;

            default:
                put_chr('?'); put_chr('\r'); put_chr('\n');
                break;
        }
    }
}
