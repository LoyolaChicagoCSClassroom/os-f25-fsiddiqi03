
#include <stdint.h>
#include "page.h"
#include "paging.h"
#include "fat.h"
#include "ide.h"
#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

const unsigned int multiboot_header[]  __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

uint8_t inb (uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void main() {
    unsigned short *vram = (unsigned short*)0xb8000;
    
    
    fatInit();

    struct file *f = fatOpen("testfile.txt");

    char buffer[512];

    for (int i = 0; i < 512; i++) buffer[i] = 0;

    int bytes = fatRead(f, buffer, 512);

    for (int i = 0; i < 200 && buffer[i] != '\0'; i++) {
    	vram[10 + i] = (0x0F << 8) | buffer[i]; 
    } 

   

    while(1) {
        uint8_t status = inb(0x64);

        if(status & 1) {
            uint8_t scancode = inb(0x60);
        }
    }
}
