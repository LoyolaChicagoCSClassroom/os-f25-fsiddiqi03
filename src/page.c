#include "page.h"
#include <stddef.h>

// Statically allocate array of 128 physical pages
// Each page is 2MB, covering 256MB total
struct ppage physical_page_array[128];

// Head of the free list
static struct ppage *free_list_head = NULL;

/**
 * Initialize the page frame allocator list
 * Links all physical pages into a doubly-linked free list
 */
void init_pfa_list(void) {
    free_list_head = NULL;
    
    // Loop through all pages and link them into the free list
    for (int i = 0; i < 128; i++) {
        // Calculate physical address: each page is 2MB (0x200000 bytes)
        physical_page_array[i].physical_addr = (void*)(i * 0x200000);
        
        // Link this page into the free list (insert at head)
        physical_page_array[i].next = free_list_head;
        physical_page_array[i].prev = NULL;
        
        if (free_list_head != NULL) {
            free_list_head->prev = &physical_page_array[i];
        }
        
        free_list_head = &physical_page_array[i];
    }
}

/**
 * Allocate one or more physical pages from the free list
 * @param npages - number of pages to allocate
 * @return pointer to the head of a new list of allocated pages, or NULL if not enough pages
 */
struct ppage *allocate_physical_pages(unsigned int npages) {
    if (npages == 0) {
        return NULL;
    }
    
    // Check if we have enough pages
    struct ppage *current = free_list_head;
    unsigned int count = 0;
    while (current != NULL && count < npages) {
        count++;
        current = current->next;
    }
    
    // Not enough free pages
    if (count < npages) {
        return NULL;
    }
    
    // Create the allocated list by unlinking npages from free_list
    struct ppage *allocd_list_head = free_list_head;
    struct ppage *allocd_list_tail = free_list_head;
    
    // Move npages-1 steps forward to find the tail of allocated list
    for (unsigned int i = 1; i < npages; i++) {
        allocd_list_tail = allocd_list_tail->next;
    }
    
    // Update the free_list_head to point past the allocated pages
    free_list_head = allocd_list_tail->next;
    
    // Disconnect the allocated list from the free list
    if (free_list_head != NULL) {
        free_list_head->prev = NULL;
    }
    allocd_list_tail->next = NULL;
    
    return allocd_list_head;
}

/**
 * Free physical pages by returning them to the free list
 * @param ppage_list - head of the list of pages to free
 */
void free_physical_pages(struct ppage *ppage_list) {
    if (ppage_list == NULL) {
        return;
    }
    
    // Find the tail of the ppage_list
    struct ppage *tail = ppage_list;
    while (tail->next != NULL) {
        tail = tail->next;
    }
    
    // Link the entire ppage_list to the front of free_list
    tail->next = free_list_head;
    if (free_list_head != NULL) {
        free_list_head->prev = tail;
    }
    
    ppage_list->prev = NULL;
    free_list_head = ppage_list;
}

