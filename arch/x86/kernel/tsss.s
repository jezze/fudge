global tss_flush
tss_flush:
    mov ax, 0x2B
    ltr ax
    ret

