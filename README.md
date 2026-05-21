# AS65 - Ensamblador 6502 residente para Tang Nano 9K

Ensamblador 6502 que corre directamente en la **Tang Nano 9K** (FPGA con CPU 6502 a 3.375 MHz), sin necesidad de PC. Lee código fuente desde una tarjeta SD y genera binarios ejecutables.

## Requisitos

- Tang Nano 9K con el monitor 6502 (ROM API v2.4+)
- Tarjeta SD con formato MicroFS
- Terminal serie (115200 baud)

## Compilación cruzada (PC)

```sh
make          # Compila AS65.BIN
make clean    # Limpia archivos generados
```

Requiere **CC65** (cl65, ca65, ld65) en `CC65_HOME` (por defecto `D:/cc65`).

## Uso en la Tang Nano

```
LOAD AS65.BIN 0800
R 0800
```

Menú:
- **A** - Ensamblar un archivo (pide nombre fuente y nombre de salida)
- **L** - Listar archivos en la SD
- **Q** - Salir al monitor

### Ejemplo

```
> A
Source: TSLEDS.ASM
Pass 1
  ...
Size 0081b
Pass 2
Output file: TSLEDS.BIN
OK! 0081b
LOAD TSLEDS.BIN 0800
R 0800
```

## Archivos de ejemplo

| Archivo | Descripción |
|---|---|
| `examples/hello_led.asm` | Mínimo: enciende un LED |
| `examples/test_leds.asm` | Efectos LED: Knight Rider, Blink, Contador binario |
| `examples/led_demo_enhanced.asm` | Efectos adicionales: Chaser, Heartbeat, Random |

## Estructura del proyecto

```
src/
  as65_main.c       - Flujo principal, menú, out_save
  as65_scan.c       - Scanner/tokenizador de líneas
  as65_opcodes.c    - Tabla de opcodes 6502 y detección de modos
  as65_symbols.c    - Tabla de símbolos (etiquetas)
  as65_mfs.c        - E/S de archivos vía MicroFS (mfs_open_read)
  io_rom.s          - I/O en ASM (put_str, put_chr, put_hex, get_chr)
  mfs_rom.s         - MFS en ASM (mfs_init, mfs_close, mfs_read_line)
  startup.s         - Código de inicio (stack, BSS, salto a main)
include/
  as65.h            - Prototipos, constantes, estructuras
  romapi.h          - ROM API: jump table y wrappers
config/
  as65.cfg          - Linker config (memoria y segmentos)
```

## Características

- **Ensamblador de 2 pasadas** (Pass 1: etiquetas, Pass 2: genera código)
- **Directivas**: `.org`, `.byte`, `.word`, `.res`
- **Etiquetas locales** con prefijo `@` (se redefinen en cada ámbito)
- **Modos de direccionamiento**: implícito, acumulador, inmediato, ZP, ZP,X/Y, absoluto, absoluto X/Y, indirecto, (indirecto,X), (indirecto),Y, relativo
- **Hasta 16 etiquetas** (12 bytes máximo por nombre)
- **Binarios de hasta 508 bytes** (buffer en $3C00)
- **Funciones críticas en ASM** para reducir tamaño (~795 bytes libres de 13824)

## Limitaciones

- Máximo 32 caracteres por línea de código fuente
- Máximo 1 argumento por `.byte`/`.word`
- Archivos en MicroFS (FAT simple, sin subdirectorios)
- El buffer de salida (508 bytes) limita el tamaño del binario generado

## Memoria (Tang Nano 9K)

```
$0000-$00FF  Zero Page
$0100-$01FF  Stack 6502
$0200-$07FF  Monitor BSS (reutilizable por el assembler)
$0800-$3DFF  Assembler (~13KB)
$3E00-$3FFF  Stack CC65
$8000-$BFFF  ROM del monitor
$C000-$C0FF  Puertos I/O (LEDs en $C001)
```

## Licencia

Código abierto.
