/* ==========================================================================
   AS65_MFS.C - E/S de archivos via MicroFS en SD
   ========================================================================== */
#include "as65.h"

/* Estado del archivo */
uint8_t  g_mfs_open;
char     g_mfs_buffer[BUFFER_SIZE];
uint16_t g_mfs_buf_pos;
uint16_t g_mfs_buf_len;
uint8_t  g_mfs_eof;

/* ==========================================================================
   mfs_open_read - Abre un archivo para lectura
   ========================================================================== */
uint8_t mfs_open_read(const char *filename) {
    uint8_t res;
    uint8_t i;
    char upname[32];

    if (g_mfs_open) mfs_close();

    for (i = 0; i < 31 && filename[i]; i++) {
        char c = filename[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        upname[i] = c;
    }
    upname[i] = '\0';

    res = rom_mfs_open(upname);
    if (res != MFS_OK) return 0;

    g_mfs_open = 1;
    g_mfs_buf_pos = 0;
    g_mfs_buf_len = 0;
    g_mfs_eof = 0;
    return 1;
}

/* mfs_read_line en mfs_rom.s */

/* mfs_close en mfs_rom.s */
/* mfs_save_bin eliminado (no usado) */
