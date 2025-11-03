#include "fat.h"
#include "ide.h"
#include <stdint.h>

#define SECTOR_SIZE 512


static int k_strncmp(const char *a, const char *b, int n) {
    for (int i = 0; i < n; i++) {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];
        if (ca != cb || ca == '\0' || cb == '\0')
            return ca - cb;
    }
    return 0;
}

static int k_strcmp(const char *a, const char *b) {
    while (*a && *b && (*a == *b)) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static void k_memcpy(void *dst, const void *src, unsigned int n) {
    unsigned char *d = (unsigned char*)dst;
    const unsigned char *s = (const unsigned char*)src;
    while (n--) {
        *d++ = *s++;
    }
}


static struct boot_sector *bs;
static unsigned int root_sector;
static unsigned int data_region_start;

static unsigned char boot_sector_buf[512];
static unsigned char rde_region[16384];
static unsigned char fat_table[262144];



static void extract_filename(struct root_directory_entry *rde, char *out) {
    int i = 0;

    
    for (int k = 0; k < 8 && rde->file_name[k] != ' '; k++) {
        out[i++] = rde->file_name[k];
    }

    
    if (rde->file_extension[0] != ' ') {
        out[i++] = '.';
        for (int k = 0; k < 3 && rde->file_extension[k] != ' '; k++) {
            out[i++] = rde->file_extension[k];
        }
    }

    out[i] = '\0';
}

// Uppercase helper
static char upper(char c) {
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}



int fatInit(void) {

   
    ata_lba_read(2048, boot_sector_buf, 1);
    bs = (struct boot_sector*)boot_sector_buf;

    
    if (bs->boot_signature != 0xAA55)
        return -1;

    // Accept only FAT16 (what mkfs.fat -F16 produces)
    if (k_strncmp(bs->fs_type, "FAT16", 5) != 0)
        return -2;

    
    unsigned int fat_start = bs->num_reserved_sectors;
    unsigned int fatsize   = bs->num_sectors_per_fat;

   
    ata_lba_read(2048 + fat_start, fat_table, fatsize);

   
    root_sector = bs->num_reserved_sectors +
                  (bs->num_fat_tables * bs->num_sectors_per_fat);

    
    unsigned int root_dir_sectors =
        ((bs->num_root_dir_entries * 32) + (bs->bytes_per_sector - 1)) /
        bs->bytes_per_sector;

    
    data_region_start = root_sector + root_dir_sectors;

    // Load entire root directory into rde_region
    ata_lba_read(2048 + root_sector, rde_region, root_dir_sectors);

    return 0;
}



struct file* fatOpen(const char *filename) {

    static struct file f; // single static file object
    struct root_directory_entry *rde_tbl =
        (struct root_directory_entry*)rde_region;

    // Uppercase copy of input filename
    char uppername[32];
    int i = 0;
    while (filename[i] && i < 31) {
        uppername[i] = upper(filename[i]);
        i++;
    }
    uppername[i] = '\0';

    // Scan root directory entries
    for (int k = 0; k < bs->num_root_dir_entries; k++) {

        
        if (rde_tbl[k].file_name[0] == 0x00)
            break;

        
        if (rde_tbl[k].file_name[0] == 0xE5)
            continue;

        char temp[32];
        extract_filename(&rde_tbl[k], temp);

        
        for (int t = 0; temp[t]; t++) {
            temp[t] = upper(temp[t]);
        }

        if (k_strcmp(temp, uppername) == 0) {
            // Found matching file
            k_memcpy(&f.rde, &rde_tbl[k], sizeof(struct root_directory_entry));
            f.start_cluster = rde_tbl[k].cluster;
            f.next = 0;
            f.prev = 0;
            return &f;
        }
    }

    return 0; // not found
}



int fatRead(struct file *file, char *buffer, unsigned int size) {

    if (!file) return -1;

    uint32_t remaining = (file->rde.file_size < size)
                         ? file->rde.file_size
                         : size;

    uint32_t cluster = file->start_cluster;
    uint32_t bytes_read = 0;

    uint32_t cluster_size =
        bs->bytes_per_sector * bs->num_sectors_per_cluster;

    
    unsigned char clusterbuf[8192];

    uint16_t *fat16 = (uint16_t*)fat_table;

    while (remaining > 0 && cluster < 0xFFF8) {

       
        uint32_t first_sector =
           2048 +  data_region_start + (cluster - 2) * bs->num_sectors_per_cluster;

      
        ata_lba_read(first_sector, clusterbuf, bs->num_sectors_per_cluster);

       
        uint32_t chunk = (remaining < cluster_size) ? remaining : cluster_size;
        k_memcpy(buffer + bytes_read, clusterbuf, chunk);

        bytes_read += chunk;
        remaining  -= chunk;

        
        cluster = fat16[cluster];
    }

    return bytes_read;
}

