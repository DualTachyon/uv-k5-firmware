#!/bin/bash

usage()
{
	echo "$0 <ELF> <ASM>"
	exit 1
}

if [ "x$1" == "x" ]
then
	usage
	return 1
fi
if [ "x$2" == "x" ]
then
	usage
	return 1
fi

echo "# .text section" > "$2"
echo "" >> "$2"

arm-none-eabi-nm -g "$1" | grep " T " | sort |
while read SYM
do
	ADDR=$(echo $SYM | cut -f 1 -d " ")
	FUNC=$(echo $SYM | cut -f 3 -d " ")
	echo ".global $FUNC" >> "$2"
	if [ "$FUNC" = "overlay_text_start" ] || [ "$FUNC" = "overlay_text_end" ]
	then
		echo "$FUNC = 0x$ADDR;" >> "$2"
	else
		echo "$FUNC:" >> "$2"
		echo "push {r4, lr}" >> "$2"
		echo "ldr r4, =(0x$ADDR + 1)" >> "$2"
		echo "blx r4" >> "$2"
		echo "pop {r4, pc}" >> "$2"
	fi
	echo "" >> "$2"
done

echo "# .bss section" >> "$2"
echo "" >> "$2"

arm-none-eabi-nm -g sram-overlay | grep " B " | sort |
while read SYM
do
	ADDR=$(echo $SYM | cut -f 1 -d " ")
	DATA=$(echo $SYM | cut -f 3 -d " ")
	echo ".global $DATA" >> "$2"
	echo "$DATA = 0x$ADDR;" >> "$2"
	echo "" >> "$2"
done

echo "# .data section" >> "$2"
echo "" >> "$2"

arm-none-eabi-nm -g sram-overlay | grep " D " | sort |
while read SYM
do
	ADDR=$(echo $SYM | cut -f 1 -d " ")
	DATA=$(echo $SYM | cut -f 3 -d " ")
	echo ".global $DATA" >> "$2"
	echo "$DATA = 0x$ADDR;" >> "$2"
	echo "" >> "$2"
done

