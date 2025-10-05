#include "paging.h"
#include <stdint.h>
#include <stddef.h>
// Page directory and page table must be 4096-byte aligned
// Must be global variables, not local
struct page_directory_entry pd[1024] __attribute__((aligned(4096)));
struct page pt[1024] __attribute__((aligned(4096)));

// External symbol from linker script
extern char _end_kernel;

/**
 * Map physical pages to a virtual address
 * @param vaddr - Virtual address to map to
 * @param pglist - Linked list of physical pages from allocator
 * @param pd - Page directory
 * @return The virtual address that was mapped
 */
void *map_pages(void *vaddr, struct ppage *pglist, struct page_directory_entry *pd) {
    uint32_t virt_addr = (uint32_t)vaddr;
    struct ppage *current_page = pglist;
    
    while (current_page != NULL) {
        // Calculate page directory index (top 10 bits of virtual address)
        uint32_t pd_index = virt_addr >> 22;
        
        // Calculate page table index (middle 10 bits of virtual address)
        uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
        
        // If page directory entry not present, initialize it
        if (!pd[pd_index].present) {
            pd[pd_index].present = 1;
            pd[pd_index].rw = 1;
            pd[pd_index].user = 0;
            pd[pd_index].writethru = 0;
            pd[pd_index].cachedisabled = 0;
            pd[pd_index].accessed = 0;
            pd[pd_index].pagesize = 0;  // 4KB pages
            pd[pd_index].ignored = 0;
            pd[pd_index].os_specific = 0;
            // Point to our page table (shift right 12 bits to get frame number)
            pd[pd_index].frame = ((uint32_t)pt) >> 12;
        }
        
        // Set up the page table entry
        pt[pt_index].present = 1;
        pt[pt_index].rw = 1;
        pt[pt_index].user = 0;
        pt[pt_index].accessed = 0;
        pt[pt_index].dirty = 0;
        pt[pt_index].unused = 0;
        // Physical address shifted right 12 bits
        pt[pt_index].frame = ((uint32_t)current_page->physical_addr) >> 12;
        
        // Move to next page in the list
        current_page = current_page->next;
        
        // Move to next virtual page (4KB = 0x1000 bytes)
        virt_addr += 0x1000;
    }
    
    return vaddr;
}

/**
 * Initialize paging and identity map kernel memory
 */
void init_paging(void) {
    // Initialize page directory and page table to all zeros
    for (int i = 0; i < 1024; i++) {
        pd[i].present = 0;
        pt[i].present = 0;
    }
    
    // Identity map kernel from 0x100000 to _end_kernel
    uint32_t kernel_start = 0x100000;
    uint32_t kernel_end = (uint32_t)&_end_kernel;
    
    for (uint32_t addr = kernel_start; addr < kernel_end; addr += 0x1000) {
        struct ppage tmp;
        tmp.next = NULL;
        tmp.prev = NULL;
        tmp.physical_addr = (void*)addr;
        map_pages((void*)addr, &tmp, pd);
    }
    
    // Identity map the stack
    uint32_t esp;
    asm("mov %%esp,%0" : "=r" (esp));
    
    // Map a few pages for the stack (align down to page boundary)
    uint32_t stack_start = esp & 0xFFFFF000;  // Align to 4KB boundary
    
    for (int i = 0; i < 4; i++) {  // Map 4 pages (16KB) for stack
        struct ppage tmp;
        tmp.next = NULL;
        tmp.prev = NULL;
        tmp.physical_addr = (void*)(stack_start - (i * 0x1000));
        map_pages((void*)(stack_start - (i * 0x1000)), &tmp, pd);
    }
    
    // Identity map video memory at 0xB8000
    struct ppage tmp_video;
    tmp_video.next = NULL;
    tmp_video.prev = NULL;
    tmp_video.physical_addr = (void*)0xB8000;
    map_pages((void*)0xB8000, &tmp_video, pd);
    
    // Load page directory into CR3
    loadPageDirectory(pd);
    
    // Enable paging
    enable_paging();
}

/**
 * Load page directory address into CR3 register
 */
void loadPageDirectory(struct page_directory_entry *pd) {
    asm("mov %0,%%cr3"
        :
        : "r"(pd)
        :);
}

/**
 * Enable paging by setting bits 0 and 31 of CR0
 */
void enable_paging(void) {
    asm("mov %cr0, %eax\n"
        "or $0x80000001,%eax\n"
        "mov %eax,%cr0");
}

