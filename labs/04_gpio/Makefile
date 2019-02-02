#######################################################################
# Makefile for STM32F0 Discovery board projects

OUTPATH = build
PROJECT = $(OUTPATH)/gpio
OPENOCD_SCRIPT_DIR ?= /usr/share/openocd/scripts
HEAP_SIZE = 0x400

################
# Sources

SOURCES_S = core/startup_stm32f051x8.s

SOURCES_CORE = $(wildcard core/*.c)
SOURCES_PERIPH = $(wildcard plib/*.c)
SOURCES_LIB = $(wildcard lib/*.c)

SOURCES_C = $(wildcard *.c)
SOURCES_C += $(SOURCES_CORE)
SOURCES_C += $(SOURCES_PERIPH)
SOURCES_C += $(SOURCES_LIB)

SOURCES = $(SOURCES_S) $(SOURCES_C)
OBJS = $(SOURCES_S:.s=.o) $(SOURCES_C:.c=.o)

# Includes and Defines

INC_CORE = -Icore
INC_LIB = -Ilib
INC_PERIPH = -Iplib
INCLUDES += $(INC_CORE)
INCLUDES += $(INC_LIB)
INCLUDES += $(INC_PERIPH)

DEFINES = -DSTM32 -DSTM32F0 -DSTM32F051x8 -DHEAP_SIZE=$(HEAP_SIZE)

# Compiler/Assembler/Linker/etc

PREFIX = arm-none-eabi

CC = $(PREFIX)-gcc
AS = $(PREFIX)-as
AR = $(PREFIX)-ar
LD = $(PREFIX)-gcc
NM = $(PREFIX)-nm
OBJCOPY = $(PREFIX)-objcopy
OBJDUMP = $(PREFIX)-objdump
READELF = $(PREFIX)-readelf
SIZE = $(PREFIX)-size
GDB = $(PREFIX)-gdb
RM = rm -f
OPENOCD=openocd

# Compiler options

MCUFLAGS = -mcpu=cortex-m0 -mlittle-endian -mfloat-abi=soft -mthumb \
           -mno-unaligned-access

DEBUG_OPTIMIZE_FLAGS = -O0 -ggdb -gdwarf-2

CFLAGS = -Wall -Wextra --pedantic
CFLAGS_EXTRA = -nostartfiles -nodefaultlibs -nostdlib \
               -fdata-sections -ffunction-sections

CFLAGS += $(DEFINES) $(MCUFLAGS) $(DEBUG_OPTIMIZE_FLAGS) $(CFLAGS_EXTRA) $(INCLUDES)

LDFLAGS = -static $(MCUFLAGS) -Wl,--start-group -lgcc -lc -lg -Wl,--end-group \
          -Wl,--gc-sections -T core/stm32f051r8tx_mmap.ld

.PHONY: dirs all clean flash erase

all: dirs $(PROJECT).bin $(PROJECT).asm

dirs: ${OUTPATH}

${OUTPATH}:
	mkdir -p ${OUTPATH}

clean:
	$(RM) $(OBJS) $(PROJECT).elf $(PROJECT).bin $(PROJECT).asm
	rm -rf ${OUTPATH}

# Hardware specific

flash: $(PROJECT).bin
	st-flash write $(PROJECT).bin 0x08000000

erase:
	st-flash erase

gdb-server-ocd:
	$(OPENOCD) -f $(OPENOCD_SCRIPT_DIR)/interface/stlink-v2.cfg \
               -f $(OPENOCD_SCRIPT_DIR)/target/stm32f0x.cfg

gdb-server-st:
	st-util

OPENOCD_P=3333
gdb-openocd: $(PROJECT).elf
	$(GDB) --eval-command="target extended-remote localhost:$(OPENOCD_P)" \
           --eval-command="load" $(PROJECT).elf

GDB_P=4242
gdb-st-util: $(PROJECT).elf
	$(GDB) --eval-command="target extended-remote localhost:$(GDB_P)"\
           --eval-command="load" $(PROJECT).elf

$(PROJECT).elf: $(OBJS)

%.elf:
	$(LD) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) -A $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.asm: %.elf
	$(OBJDUMP) -dwh $< > $@
