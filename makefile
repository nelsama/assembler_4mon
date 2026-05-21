CC65_HOME ?= D:/cc65
CC = $(CC65_HOME)/bin/cl65
CA65 = $(CC65_HOME)/bin/ca65
LD65 = $(CC65_HOME)/bin/ld65

CFLAGS = -t none -O --cpu 6502 -I include
ASFLAGS = -t none --cpu 6502

all: output/as65.bin

build:
	-mkdir build 2>nul || ver >nul
output:
	-mkdir output 2>nul || ver >nul

# ======================================================================
# AS65 - Assembler 6502 residente para Tang Nano 9K
# ======================================================================
AS65_OBJS = build/startup.o build/io_rom.o build/mfs_rom.o \
            build/as65_main.o build/as65_scan.o build/as65_opcodes.o \
            build/as65_symbols.o build/as65_mfs.o

output/as65.bin: $(AS65_OBJS) | build output
	$(LD65) -C config/as65.cfg -m output/as65.map -o $@ $^ $(CC65_HOME)/lib/none.lib
	@py -c 's=open("$@","rb").read(); print("Binario: %d bytes = 0x%04X" % (len(s), len(s))); print("LOAD AS65.BIN 0800 %04X" % len(s))'

build/startup.o: src/startup.s | build
	$(CA65) $(ASFLAGS) -o $@ $<

build/io_rom.o: src/io_rom.s | build
	$(CA65) $(ASFLAGS) -o $@ $<

build/mfs_rom.o: src/mfs_rom.s | build
	$(CA65) $(ASFLAGS) -o $@ $<

build/as65_main.o: src/as65_main.c include/as65.h include/romapi.h | build
	$(CC) -c $(CFLAGS) -o $@ $<

build/as65_scan.o: src/as65_scan.c include/as65.h | build
	$(CC) -c $(CFLAGS) -o $@ $<

build/as65_opcodes.o: src/as65_opcodes.c include/as65.h | build
	$(CC) -c $(CFLAGS) -o $@ $<

build/as65_symbols.o: src/as65_symbols.c include/as65.h | build
	$(CC) -c $(CFLAGS) -o $@ $<

build/as65_mfs.o: src/as65_mfs.c include/as65.h include/romapi.h | build
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	-rm -rf build output
	@echo Limpio.

.PHONY: all build output clean
