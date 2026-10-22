# fudgeboot: AmigaDOS loads this one-hunk executable anywhere in memory and starts it in user mode.
# It takes over the machine, copies the kernel with the ramdisk after it to the kernel's address
# (KERNEL, set by the build) and enters it with the ramdisk's address in a0 and its size in d0.
.set EXEC_SUPERVISOR,                   -30
.set CUSTOM,                            0xDFF000
.set CUSTOM_DMACON,                     0x96
.set CUSTOM_INTENA,                     0x9A
.set CUSTOM_INTREQ,                     0x9C
.set COPIER,                            0x00010000
.set HUNK_HEADER,                       0x3F3
.set HUNK_CODE,                         0x3E9
.set HUNK_END,                          0x3F2

.section .text

    .long HUNK_HEADER, 0, 1, 0, 0, (end - code) / 4
    .long HUNK_CODE, (end - code) / 4

code:
    move.l 4.w, %a6
    lea supervisor(%pc), %a5
    jsr EXEC_SUPERVISOR(%a6)

supervisor:
    move.w #0x2700, %sr
    lea CUSTOM, %a0
    move.w #0x7FFF, CUSTOM_INTENA(%a0)
    move.w #0x7FFF, CUSTOM_INTREQ(%a0)
    move.w #0x7FFF, CUSTOM_DMACON(%a0)
    cpusha %bc
    moveq #0, %d0
    movec %d0, %cacr
    cinva %bc
    movec %d0, %tc
    movec %d0, %itt0
    movec %d0, %itt1
    movec %d0, %dtt0
    movec %d0, %dtt1
    pflusha

# the copy may overwrite this code, so it runs from chip memory, which neither the image nor the kernel uses
    lea copy(%pc), %a0
    lea COPIER, %a1
    move.l #(copyend - copy) / 2 - 1, %d0
1:
    move.w (%a0)+, (%a1)+
    dbf %d0, 1b
    lea payload(%pc), %a0
    move.l #KERNEL, %a1
    move.l #(end - payload) / 4, %d0
    jmp COPIER

# copies d0 longs from a0 to a1, from the end when the destination is above the source
copy:
    cmp.l %a0, %a1
    bhi 2f
1:
    move.l (%a0)+, (%a1)+
    subq.l #1, %d0
    bne 1b
    bra 4f
2:
    move.l %d0, %d1
    lsl.l #2, %d1
    add.l %d1, %a0
    add.l %d1, %a1
3:
    move.l -(%a0), -(%a1)
    subq.l #1, %d0
    bne 3b
4:
    move.l #KERNEL + (ramdisk - payload), %a0
    move.l #ramdiskend - ramdisk, %d0
    jmp KERNEL
copyend:

.balign 4
payload:
    .incbin "fudge.bin"

.balign 4
ramdisk:
    .incbin "fudge.cpio"
ramdiskend:

.balign 4
end:
    .long HUNK_END

