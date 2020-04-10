# create .o files from .bmp image files; to be included as RAW data sections
arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm panda.bmp panda.o
arm-none-eabi-objcopy -I binary -O elf32-littlearm -B arm porkcar.bmp porkcar.o

# show exported symbols
nm -S -t d panda.o
nm -S -t d porkcar.o

# compile-link
arm-none-eabi-as -mcpu=arm926ej-s -g ts.s -o ts.o
arm-none-eabi-gcc -c -mcpu=arm926ej-s -g t.c -o t.o
arm-none-eabi-ld -T t.ld ts.o t.o -o t.elf
arm-none-eabi-objcopy -O binary t.elf t.bin

rm *.o *.elf

echo ready to go?
read dummy

qemu-system-arm -M versatilepb -m 128M -kernel t.bin -serial mon:stdio



 





