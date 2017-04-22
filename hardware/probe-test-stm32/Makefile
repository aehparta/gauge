F_CPU		= 32000000UL
CPU_TYPE	= STM32L1

DEFS		= -DCPY_TYPE="$(CPU_TYPE)" -DF_CPU=$(F_CPU)

FP_FLAGS	?= -msoft-float -mfloat-abi=soft
ARCH_FLAGS	= -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd

LDSCRIPT    = stm32l100rc.ld

################################################################

# Main file
BINARY      = main

INCL_DIRS   = . lib utils

STARTUP_SCRIPT = startup_stm32l100xc.s

OBJS += $(BINARY).o
OBJS += $(STARTUP_SCRIPT:.s=.o)
OBJS += utils/gpio.o
OBJS += utils/usart.o
OBJS += utils/timebase.o
OBJS += utils/debounce.o
OBJS += utils/nvic.o
OBJS += utils/str_utils.o
OBJS += init.o
OBJS += blink.o
OBJS += lib/common.o

################################################################

JUNK            = *.o *.d *.elf *.bin *.hex *.srec *.list *.map

###############################################################################
###############################################################################
###############################################################################

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
  Q     := @
  NULL  := 2>/dev/null
endif

###############################################################################
# Executables

PREFIX  ?= arm-none-eabi

CC      := $(PREFIX)-gcc
CXX     := $(PREFIX)-g++
LD      := $(PREFIX)-gcc
AR      := $(PREFIX)-ar
AS      := $(PREFIX)-as
OBJCOPY := $(PREFIX)-objcopy
OBJDUMP := $(PREFIX)-objdump
GDB     := $(PREFIX)-gdb
STFLASH := $(shell which st-flash)

###############################################################################

# For CMSIS compatibility
DEFS          += -D__weak="__attribute__((weak))" -D__packed="__attribute__((__packed__))"

###############################################################################
# C flags

CFLAGS      += -Os -ggdb -std=gnu99 -Wfatal-errors
CFLAGS      += -Wall -Wextra -Wshadow
CFLAGS      += -Wwrite-strings -Wold-style-definition -Winline -Wmissing-noreturn -Wstrict-prototypes
CFLAGS      += -Wredundant-decls -Wfloat-equal -Wsign-compare
CFLAGS      += -fno-common -ffunction-sections -fdata-sections -Wunused-function
CFLAGS      += -MD
CFLAGS      += $(INCL_DIRS:%=-I%) $(DEFS)

###############################################################################
# Linker flags

LDFLAGS     += --static -specs=nano.specs -lm -lc
#LDFLAGS     += -L$(OCM3_LIB_DIR)
LDFLAGS     += -T$(LDSCRIPT)
LDFLAGS     += -Wl,-Map=$(*).map
LDFLAGS     += -Wl,--gc-sections

###############################################################################
# Used libraries

LDLIBS		+= -lm
LDLIBS		+= -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

###############################################################################
###############################################################################
###############################################################################

.SUFFIXES: .elf .bin .hex .srec .list .map .images
.SECONDEXPANSION:
.SECONDARY:

all: elf

elf: $(BINARY).elf
bin: $(BINARY).bin
hex: $(BINARY).hex
srec: $(BINARY).srec
list: $(BINARY).list

images: $(BINARY).images
program: $(BINARY).flash

%.images: %.bin %.hex %.srec %.list %.map

%.bin: %.elf
	$(Q)$(OBJCOPY) -Obinary $(*).elf $(*).bin

%.hex: %.elf
	$(Q)$(OBJCOPY) -Oihex $(*).elf $(*).hex

%.srec: %.elf
	$(Q)$(OBJCOPY) -Osrec $(*).elf $(*).srec

%.list: %.elf
	$(Q)$(OBJDUMP) -S $(*).elf > $(*).list

%.elf %.map: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) $(ARCH_FLAGS) $(OBJS) $(LDLIBS) -o $(*).elf

%.o: %.c
	$(Q)$(CC) $(CFLAGS) $(ARCH_FLAGS) -o $(*).o -c $(*).c

%.o: %.s
	$(Q)$(CC) $(CFLAGS) $(ARCH_FLAGS) -o $(*).o -c $(*).s

clean:
	$(Q)$(RM) $(JUNK)
	$(Q)cd lib && $(RM) $(JUNK)

%.flash: %.bin
	@printf "  FLASH  $<\n"
	$(Q)$(STFLASH) --reset write $(*).bin 0x8000000
	$(Q)$(STFLASH) reset

.PHONY: images clean elf bin hex srec list

-include $(OBJS:.o=.d)
