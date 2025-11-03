
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
    
    // Clear screen
    for (int i = 0; i < 80 * 25; i++) {
        vram[i] = (0x0F << 8) | ' ';
    }
    
    vram[0] = (0x0A << 8) | 'A';
    init_pfa_list();
    
    vram[1] = (0x0A << 8) | 'B';
    init_paging();
    
    vram[2] = (0x0A << 8) | 'C';
    int result = fatInit();
    
    vram[3] = (0x0A << 8) | 'D';
    if (result != 0) {
        vram[4] = (0x0C << 8) | 'X';  // Red X
        while(1);
    }
    
    vram[5] = (0x0A << 8) | 'E';
    struct file *f = fatOpen("testfile.txt");
    
    vram[6] = (0x0A << 8) | 'F';
    if (!f) {
        vram[7] = (0x0C << 8) | 'X';  // Red X - file not found
        while(1);
    }
    
    vram[8] = (0x0A << 8) | 'G';
    char buffer[512];
    for (int i = 0; i < 512; i++) buffer[i] = 0;
    
    int bytes = fatRead(f, buffer, 512);
    vram[9] = (0x0A << 8) | 'H';
    
    // Print file contents starting at row 2
    for (int i = 0; i < 200 && buffer[i] != '\0'; i++) {
        vram[160 + i] = (0x0F << 8) | buffer[i];
    }

   

    while(1) {
        uint8_t status = inb(0x64);

        if(status & 1) {
            uint8_t scancode = inb(0x60);
        }
    }
}
