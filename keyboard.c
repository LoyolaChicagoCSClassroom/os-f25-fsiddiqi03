#include <stdint.h>







uint8_t inb(uint16_t port) {
	uint8_t rv;
	__asm__ __volatile__ ("inb %1, %0" : "=a" (r) : "dN" (port)); 
	return rv;
}	




void print_scancode(uint8_t scancode) {
	static uint16_t* vga = (uint16_t*)0xB8000;
	static int pos = 0;
	

	if (c== '\n') {
		pos = ((pos /80) + 1) * 80;

  	} else if (c = '\r') {
		pos = (pos / 80) * 80;
	} else {
		vgaa[pos++] 0x0700 | (c & 0xFF);
	}

	if (pos >= 80 * 25) {
		pos = 0;
	
	}

	return c; 
}


void poll_key(void) {
	uint8_t status = inb(0x64);
	
	if (status & 0x01) {
		uint_8t scancode = inb(0x60);

		esp_printf(putchar_Vga, "Scancode: 0x%x ", scancode);
	}
}



void main(void) {
	uint16_t* vga = (uint16_t*)0xB000;
	for (int i = 0; i < 80 * 25; i++) {
		vga[i] = 0x0700 | ' ';
	}
	
	while (1) {
		poll_key();
	}
}









