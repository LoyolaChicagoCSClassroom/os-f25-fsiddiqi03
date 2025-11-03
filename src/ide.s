.globl ata_lba_read

    .text

# void ata_lba_read(unsigned int lba, unsigned char *buffer, unsigned int numsectors);
# args: [ebp+8] = lba, [ebp+12] = buffer, [ebp+16] = numsectors

ata_lba_read:
    pushl   %ebp
    movl    %esp, %ebp
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %edi

    # Disable interrupts on controller
    movl    $0x03F6, %edx
    movb    $0x02, %al
    outb    %al, %dx

    # Load args
    movl    8(%ebp), %eax      # LBA
    movl    12(%ebp), %edi     # buffer
    movl    16(%ebp), %ecx     # numsectors
    andl    $0x0FFFFFFF, %eax
    movl    %eax, %ebx         # save LBA

    # Drive/head register: high 4 bits of LBA + 0xE0
    movl    $0x01F6, %edx
    movl    %ebx, %eax
    shrl    $24, %eax
    orb     $0xE0, %al         # LBA mode, master
    outb    %al, %dx

    # Sector count
    movl    $0x01F2, %edx
    movb    %cl, %al
    outb    %al, %dx

    # LBA low (0-7)
    movl    $0x01F3, %edx
    movl    %ebx, %eax
    outb    %al, %dx

    # LBA mid (8-15)
    movl    $0x01F4, %edx
    movl    %ebx, %eax
    shrl    $8, %eax
    outb    %al, %dx

    # LBA high (16-23)
    movl    $0x01F5, %edx
    movl    %ebx, %eax
    shrl    $16, %eax
    outb    %al, %dx

    # Issue READ SECTORS (0x20)
    movl    $0x01F7, %edx
    movb    $0x20, %al
    outb    %al, %dx

read_loop:
    # Wait for DRQ set (bit 3) and BSY clear (bit 7)
wait_drq:
    inb     %dx, %al
    testb   $0x80, %al         # BSY?
    jnz     wait_drq
    testb   $0x08, %al         # DRQ?
    jz      wait_drq

    # Read 1 sector (512 bytes = 256 words)
    movl    $0x01F0, %edx
    movw    $256, %cx
    rep insw                    # reads from dx to [es:di], es assumed flat

    # One sector done
    decl    %ecx                # decrement sector count
    jnz     read_loop

    # Success: return 0
    xorl    %eax, %eax

done:
    popl    %edi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    leave
    ret

                              
                             
                            
                                
