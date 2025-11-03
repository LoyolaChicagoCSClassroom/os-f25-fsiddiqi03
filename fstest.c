#include "fat.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

char sector_buf[512];
int fd = 0
int read_sector_from_disk_image(unsigned int sector_num, char *buf) {
	// position the OS index 
	lseek(fd, sector_num * 512, SEEK_SET);

	// Read one sector from the disk image 
	int n = read(fd, buf, 512);
	
}



int main() {
	struct boot_sector *bs = sector_buf;
	int fd = open("disk.img", O_RDONLY);
	read_sector_from_disk_image(0, sector_buf);

	printf("sectors per cluster = %d\n", sector_buf[13]);





