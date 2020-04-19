.intel_syntax noprefix

.code32

.global _get_video_mode
_get_video_mode:
    pushad

    mov eax, 0x7000
    mov [eax], esp

    mov eax, cr0
    and eax, ~0x80000000
    mov cr0, eax

    mov eax, 0x9000
    lgdt [eax]
    ljmp 0x8:(0x8000 + 1f - _get_video_mode)

.code16

1:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ax, (0x9000 + realmode_idt - realmode_gdt)
    lidt [eax]

    mov eax, cr0
    and eax, 0x7FFFFFFE
    mov cr0, eax

    ljmp 0x0:(0x8000 + 1f - _get_video_mode)

1:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov sp, 0xa000

    # real mode. do stuff here.

    mov di, 0xc000
    mov eax, 0x32454256
    mov dword [di], eax
    mov ax, 0x4F00
    int 0x10

    cmp ax, 0x4F    # BIOS doesn't support VBE?
    jne .Xerror

    mov bx, (0xc000 + 16) # get video_modes (.segment)
    mov fs, [bx]
    mov bx, (0xc000 + 14) # get video_modes (.offset)
    mov si, [bx]

    add si, 0 # OFFSET

    mov dx, fs:[si] # grab the mode

    cmp dx, 0xFFFF   # end of list?
    je .Xerror

    mov ax, 0x4F01    # get VBE mode info
    mov cx, dx
    mov di, 0xd000
    int 0x10

    cmp ax, 0x4F
    je .Xsuccess

.Xerror:
    xor ebx, ebx
    jmp .Xfinish

.Xsuccess:
    mov ebx, 1

.Xfinish:
    mov eax, 0x1000
    lgdt [eax]

    mov eax, 0x2000
    lidt [eax]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ljmp 0x8:(0x8000 + 1f - _get_video_mode)

.code32

1:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov eax, 0x7000
    mov esp, [eax]

    popad
    ret

.code32

.global _set_video_mode
_set_video_mode:
    pushad

    mov eax, 0x7000
    mov [eax], esp

    mov eax, cr0
    and eax, ~0x80000000
    mov cr0, eax

    mov eax, 0x9000
    lgdt [eax]
    ljmp 0x8:(0x8000 + 1f - _set_video_mode)

.code16

1:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ax, (0x9000 + realmode_idt - realmode_gdt)
    lidt [eax]

    mov eax, cr0
    and eax, 0x7FFFFFFE
    mov cr0, eax

    ljmp 0x0:(0x8000 + 1f - _set_video_mode)

1:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov sp, 0xa000

    # real mode. do stuff here.

    mov di, 0xc000
    mov eax, 0x32454256  # "VBE2"
    mov dword [di], eax
    mov ax, 0x4F00    # get VBE BIOS info
    int 0x10

    mov bx, (0xc000 + 16) # get video_modes (.segment)
    mov fs, [bx]
    mov bx, (0xc000 + 14) # get video_modes (.offset)
    mov si, [bx]

    add si, 0

    mov ax, 0x4F02
    mov bx, fs:[si] # mode
    or bx, 0x4000   # enable LFB
    mov di, 0
    int 0x10

    cmp ax, 0x4F
    jne .Yerror
    jmp .Ysuccess

.Yerror:
    xor ebx, ebx
    jmp .Yfinish

.Ysuccess:
    mov ebx, 1

.Yfinish:
    mov eax, 0x1000
    lgdt [eax]

    mov eax, 0x2000
    lidt [eax]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ljmp 0x8:(0x8000 + 1f - _set_video_mode)

.code32

1:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov eax, 0x7000
    mov esp, [eax]

    popad
    ret

.code32

.global realmode_gdt
realmode_gdt:
.word (3 * 8) - 1
.long 0x9008
.word 0

.long 0x0
.long 0x0
                
.word 0xFFFF
.word 0x0
.byte 0x0
.byte 0x9A
.byte 0x8F
.byte 0x00

.word 0xFFFF
.word 0x0
.byte 0x0
.byte 0x92
.byte 0x8F
.byte 0x00

.global realmode_idt
realmode_idt:
.word 0x3FF
.long 0x0

