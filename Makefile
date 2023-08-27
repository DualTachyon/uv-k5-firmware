TARGET = firmware
OVERLAY = sram-overlay
BLOB_OVERLAY = blob-overlay
LINK_OVERLAY = link-overlay

BSP_DEFINITIONS := $(wildcard hardware/*/*.def)
BSP_HEADERS := $(patsubst hardware/%,bsp/%,$(BSP_DEFINITIONS))
BSP_HEADERS := $(patsubst %.def,%.h,$(BSP_HEADERS))

OBJS =
# Startup files
OBJS += start.o
OBJS += init.o
OBJS += overlay.o
OBJS += $(LINK_OVERLAY).o
OBJS += $(BLOB_OVERLAY).o
OBJS += external/printf/printf.o

# Drivers
OBJS += driver/adc.o
OBJS += driver/backlight.o
OBJS += driver/bk1080.o
OBJS += driver/bk4819.o
OBJS += driver/crc.o
OBJS += driver/eeprom.o
OBJS += driver/flash.o
OBJS += driver/gpio.o
OBJS += driver/i2c.o
OBJS += driver/keyboard.o
OBJS += driver/spi.o
OBJS += driver/st7565.o
OBJS += driver/system.o
OBJS += driver/systick.o
OBJS += driver/uart.o

# Main
OBJS += app/menu.o
OBJS += aircopy.o
OBJS += app.o
OBJS += audio.o
OBJS += battery.o
OBJS += bitmaps.o
OBJS += board.o
OBJS += dcs.o
OBJS += dtmf.o
OBJS += fm.o
OBJS += font.o
OBJS += frequencies.o
OBJS += functions.o
OBJS += gui.o
OBJS += helper.o
OBJS += misc.o
OBJS += radio.o
OBJS += scheduler.o
OBJS += settings.o

OBJS += main.o

TOP := $(shell pwd)

AS = arm-none-eabi-as
CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

ASFLAGS = -mcpu=cortex-m0
CFLAGS = -Os -Wall -Werror -mcpu=cortex-m0 -fno-builtin -fshort-enums -std=c11 -MMD
CFLAGS += -DPRINTF_INCLUDE_CONFIG_H
LDFLAGS = -mcpu=cortex-m0 -nostartfiles -Wl,-T,firmware.ld

OVERLAY_CFLAGS = $(CFLAGS) -fno-inline -fno-toplevel-reorder
OVERLAY_LD = arm-none-eabi-ld
OVERLAY_LDFLAGS = -T $(OVERLAY).ld -S

ifeq ($(DEBUG),1)
ASFLAGS += -g
CFLAGS += -g
LDFLAGS += -g
endif

INC =
INC += -I $(TOP)
INC += -I $(TOP)/external/CMSIS_5/CMSIS/Core/Include/
INC += -I $(TOP)/external/CMSIS_5/Device/ARM/ARMCM0/Include

LIBS =

DEPS = $(OBJS:.o=.d)

all: $(TARGET)
	$(OBJCOPY) -O binary $< $<.bin
	$(SIZE) $<

$(OVERLAY).bin: $(OVERLAY)
	$(OBJCOPY) -O binary $< $@

$(OVERLAY): $(OVERLAY).o
	$(OVERLAY_LD) $(OVERLAY_LDFLAGS) $< -o $@

$(OVERLAY).o: $(OVERLAY).c
	$(CC) $(OVERLAY_CFLAGS) $(INC) -c $< -o $@

$(LINK_OVERLAY).o: $(LINK_OVERLAY).S
	$(AS) $(ASFLAGS) $< -o $@

$(LINK_OVERLAY).S: $(OVERLAY)
	./gen-overlay-symbols.sh $< $@

$(BLOB_OVERLAY).S: $(OVERLAY).bin

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

bsp/dp32g030/%.h: hardware/dp32g030/%.def

%.o: %.c | $(BSP_HEADERS)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

-include $(DEPS)

clean:
	rm -f $(TARGET).bin $(TARGET) $(OBJS) $(DEPS) $(OVERLAY).bin $(OVERLAY) $(OVERLAY).o $(OVERLAY).d $(LINK_OVERLAY).o $(LINK_OVERLAY).S $(BLOB_OVERLAY).o

