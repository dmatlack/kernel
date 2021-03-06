#
#
# Operating System Makefile
#
#
.PHONY: default kernel user tools iso clean tags

default: iso

#
# Create an .iso (CDROM image) that can boot the os
#
iso: kernel user initrd boot/grub.cfg 
	mkdir -p iso
	mkdir -p iso/boot
	mkdir -p iso/boot/grub
	cp kernel/KERNEL.o 	iso/boot/KERNEL.o
	cp boot/grub.cfg 	iso/boot/grub/grub.cfg
	cp initrd.img 		iso/boot/
	grub-mkrescue -o OS.iso iso

#
# Create the initial ramdisk needed to use the operating system
#
initrd: kernel user tools
	tools/create_initrd initrd.img user/bin/* boot/initrd/*

#
# Build the kernel sources into an object file
#
kernel:
	+make -j8 -C kernel/

#
# Build the user progams into executables compatable with kernel
#
user:
	+make -C user/

#
# Build the tools used to build the kernel and os
#
tools:
	+make -C tools/

tags:
	find . -name '*.[cshS]' | xargs ctags

clean:
	+make -C kernel/ clean
	+make -C user/ clean
	+make -C tools/ clean
	rm  -f initrd.img
	rm -rf iso
	rm -rf OS.iso
	rm -rf tags
