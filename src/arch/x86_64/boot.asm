global start
extern long_mode_start

section .text
bits 32
start:
    mov esp, stack_top

    mov dword [multiboot_info_ptr], ebx

    call check_multiboot
    call check_cpuid
    call check_long_mode

    call set_up_page_tables
    call enable_paging

    ; load the 64-bit GDT
    lgdt [gdt64.pointer]

    jmp gdt64.code:long_mode_start

    ; print `OK` to screen
    mov dword [0xb8000], 0x2f4b2f4f
    hlt

check_multiboot:
    cmp eax, 0x36d76289
    jne .no_multiboot
    ret
.no_multiboot:
    mov al, "0"
    jmp error

check_cpuid:
    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21)
    ; in the FLAGS register. If we can flip it, CPUID is available.

    ; Copy FLAGS in to EAX via stack
    pushfd
    pop eax

    ; Copy to ECX as well for comparing later on
    mov ecx, eax

    ; Flip the ID bit
    xor eax, 1 << 21

    ; Copy EAX to FLAGS via the stack
    push eax
    popfd

    ; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    pushfd
    pop eax

    ; Restore FLAGS from the old version stored in ECX (i.e. flipping the
    ; ID bit back if it was ever flipped).
    push ecx
    popfd

    ; Compare EAX and ECX. If they are equal then that means the bit
    ; wasn't flipped, and CPUID isn't supported.
    cmp eax, ecx
    je .no_cpuid
    ret
.no_cpuid:
    mov al, "1"
    jmp error

check_long_mode:
    ; test if extended processor info in available
    mov eax, 0x80000000    ; implicit argument for cpuid
    cpuid                  ; get highest supported argument
    cmp eax, 0x80000001    ; it needs to be at least 0x80000001
    jb .no_long_mode       ; if it's less, the CPU is too old for long mode

    ; use extended info to test if long mode is available
    mov eax, 0x80000001    ; argument for extended processor info
    cpuid                  ; returns various feature bits in ecx and edx
    test edx, 1 << 29      ; test if the LM-bit is set in the D-register
    jz .no_long_mode       ; If it's not set, there is no long mode
    ret
.no_long_mode:
    mov al, "2"
    jmp error

set_up_page_tables:
    ; clear all page tables first
    mov eax, 0
    mov ecx, 4096 * 5  ; 5 pages: PML4 + 4 PDPTs
    mov edi, p4_table
    rep stosb
    
    ; setting up p4_table (PML4)
    ; entry 0: identity map for physical memory (0x0000000000000000)
    mov eax, pdpt_low
    or eax, 0b11 ; present + writable
    mov [p4_table], eax
    
    ; entry 1: kernel heap (0x0000010000000000)
    mov eax, pdpt_heap
    or eax, 0b11 ; present + writable
    mov [p4_table + 8], eax  ; 8 bytes per entry
    
    ; entry 15: kernel stacks (0x00000F0000000000)
    mov eax, pdpt_stacks
    or eax, 0b11 ; present + writable
    mov [p4_table + 15*8], eax
    
    ; entry 16: user space (0x0000100000000000)
    mov eax, pdpt_user
    or eax, 0b11 ; present + writable
    mov [p4_table + 16*8], eax
    
    ; setting up pdpt_low (for identity mapping)
    mov eax, pd_table
    or eax, 0b11 ; present + writable
    mov [pdpt_low], eax

    mov ecx, 0

.map_p2_table:
    ; map ecx-th P2 entry to a huge page that starts at address 2MiB*ecx
    mov eax, 0x200000  ; 2MiB
    mul ecx            ; start address of ecx-th page
    or eax, 0b10000011 ; present + writable + huge
    mov [pd_table + ecx * 8], eax ; map ecx-th entry

    inc ecx            ; increase counter
    cmp ecx, 512       ; if counter == 512, the whole P2 table is mapped
    jne .map_p2_table  ; else map the next entry

    ret

enable_paging:
    ; load P4 to cr3 register (cpu uses this to access the P4 table)
    mov eax, p4_table
    mov cr3, eax

    ; enable PAE-flag in cr4 (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; set the long mode bit in the EFER MSR (model specific register)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; enable paging in the cr0 register
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

section .rodata
global gdt64
gdt64:
    dq 0                          ; zero entry
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; code segment
.data: equ $ - gdt64
    dq (1<<41) | (1<<44) | (1<<47) | (1<<53) ; data segment
.tss: equ $ - gdt64
    dq 0                          ; TSS descriptor part 1
    dq 0                          ; TSS descriptor part 2
.pointer:
    dw $ - gdt64 - 1              ; Limit (size of GDT)
    dq gdt64                      ; Base address of GDT

error:
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte  [0xb800a], al
    hlt

section .bss
align 4096
; Page tables
p4_table:
    resb 4096
pdpt_low:
    resb 4096
pdpt_heap:
    resb 4096
pdpt_stacks:
    resb 4096
pdpt_user:
    resb 4096
pd_table:
    resb 4096
stack_bottom:
    resb 64
stack_top:

section .data
multiboot_info_ptr: dd 0
global multiboot_info_ptr