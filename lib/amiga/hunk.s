# fudgeboot: AmigaDOS loads this one-hunk executable anywhere in memory and starts it in user mode.
# It first prints what it finds (cpu, memory, display chip) to its output and returns to the shell if the
# kernel can't run there. Otherwise it takes over the machine, copies the kernel with the ramdisk
# after it to the kernel's address (KERNEL, set by the build) and enters it with the ramdisk's address
# in a0 and its size in d0.
.set EXEC_SUPERVISOR,                   -30
.set EXEC_CLOSELIBRARY,                 -414
.set EXEC_OPENLIBRARY,                  -552
.set EXEC_ATTNFLAGS,                    0x128
.set EXEC_MEMLIST,                      0x142
.set DOS_WRITE,                         -48
.set DOS_OUTPUT,                        -60
.set DOS_DELAY,                         -198
.set MEMHEADER_NAME,                    10
.set MEMHEADER_UPPER,                   24
.set ATTNB_68040,                       3
.set ATTNB_68060,                       7
.set ATTNB_68080,                       10
.set DENISEID_AGA,                      0xF8
# ARCH_RAMBASE and the end of ARCH_MEM in src/kernel/m68k/arch.h
.set RAMBASE,                           0x10000000
.set RAMEND,                            0x1E000000
.set CUSTOM,                            0xDFF000
.set CUSTOM_DENISEID,                   0x7C
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
    movem.l %d2-%d7/%a2-%a6, -(%sp)
    move.l 4.w, %a4
    moveq #0, %d6
    moveq #0, %d7
    move.l %a4, %a6
    lea dosname(%pc), %a1
    moveq #0, %d0
    jsr EXEC_OPENLIBRARY(%a6)
    move.l %d0, %a5
    tst.l %d0
    beq 1f
    move.l %a5, %a6
    jsr DOS_OUTPUT(%a6)
    move.l %d0, %d7
1:
    lea textstart(%pc), %a0
    bsr print

# d6: bit 0 a 68040, 68060 or 68080 (compatible MMUs), bit 1 ram at RAMBASE, bit 2 ram up to RAMEND
    lea textcpu(%pc), %a0
    bsr print
    moveq #0, %d0
    move.w EXEC_ATTNFLAGS(%a4), %d0
    move.l %d0, %d5
    bsr printhex
    lea text68080(%pc), %a0
    btst #ATTNB_68080, %d5
    bne 1f
    lea text68060(%pc), %a0
    btst #ATTNB_68060, %d5
    bne 1f
    lea textold(%pc), %a0
    btst #ATTNB_68040, %d5
    beq 2f
    lea text68040(%pc), %a0
1:
    bset #0, %d6
2:
    bsr print

# every board is in the list before the boot shell runs
    lea textmemory(%pc), %a0
    bsr print
    move.l EXEC_MEMLIST(%a4), %a2
3:
    tst.l (%a2)
    beq 5f
    lea textindent(%pc), %a0
    bsr print
    move.l %a2, %d0
    bsr printhex
    lea textdash(%pc), %a0
    bsr print
    move.l MEMHEADER_UPPER(%a2), %d0
    bsr printhex
    lea textspace(%pc), %a0
    bsr print
    move.l MEMHEADER_NAME(%a2), %d0
    beq 4f
    move.l %d0, %a0
    bsr print
4:
    lea textnewline(%pc), %a0
    bsr print
    move.l MEMHEADER_UPPER(%a2), %d0
    cmp.l #RAMBASE, %a2
    bhi 6f
    cmp.l #RAMBASE, %d0
    bls 6f
    bset #1, %d6
6:
    cmp.l #RAMEND, %a2
    bcc 7f
    cmp.l #RAMEND, %d0
    bcs 7f
    bset #2, %d6
7:
    move.l (%a2), %a2
    bra 3b
5:

# graphics.library's ChipRevBits0 stays 0 under AROS, so ask lisa itself
    lea textchipset(%pc), %a0
    bsr print
    moveq #0, %d0
    move.b CUSTOM + CUSTOM_DENISEID + 1, %d0
    move.l %d0, %d5
    bsr printhex
    lea textaga(%pc), %a0
    cmp.b #DENISEID_AGA, %d5
    beq 8f
    lea textnoaga(%pc), %a0
8:
    bsr print

    lea textnocpu(%pc), %a0
    btst #0, %d6
    beq 10f
    moveq #6, %d0
    and.l %d6, %d0
    cmp.l #6, %d0
    bne 13f
    lea textgo(%pc), %a0
    bsr print
    move.l %a5, %d0
    beq 11f
    move.l %a5, %a6
    move.l #250, %d1
    jsr DOS_DELAY(%a6)
11:
    move.l 4.w, %a6
    lea supervisor(%pc), %a5
    jsr EXEC_SUPERVISOR(%a6)

13:
    lea textnoram(%pc), %a0
    bsr print
    move.l #RAMBASE, %d0
    bsr printhex
    lea textto(%pc), %a0
    bsr print
    move.l #RAMEND, %d0
    bsr printhex
    lea textnewline(%pc), %a0
10:
    bsr print
    lea textstop(%pc), %a0
    bsr print
    move.l %a5, %d0
    beq 12f
    move.l %a4, %a6
    move.l %a5, %a1
    jsr EXEC_CLOSELIBRARY(%a6)
12:
    movem.l (%sp)+, %d2-%d7/%a2-%a6
    moveq #20, %d0
    rts

# writes the string at a0 to the output in d7 through dos in a5
print:
    movem.l %d2-%d3/%a6, -(%sp)
    move.l %a0, %d2
    moveq #-1, %d3
1:
    addq.l #1, %d3
    tst.b (%a0)+
    bne 1b
    tst.l %d7
    beq 2f
    move.l %d7, %d1
    move.l %a5, %a6
    jsr DOS_WRITE(%a6)
2:
    movem.l (%sp)+, %d2-%d3/%a6
    rts

# writes d0 as 0x and eight hex digits
printhex:
    move.l %d2, -(%sp)
    lea hexdigits(%pc), %a1
    lea hexend(%pc), %a0
    moveq #7, %d1
1:
    moveq #15, %d2
    and.b %d0, %d2
    move.b (%a1,%d2.w), -(%a0)
    lsr.l #4, %d0
    dbf %d1, 1b
    subq.l #2, %a0
    move.l (%sp)+, %d2
    bra print

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

dosname:
    .asciz "dos.library"
textstart:
    .asciz "fudgeboot\n"
textcpu:
    .asciz "cpu: AttnFlags "
text68040:
    .asciz " (68040)\n"
text68060:
    .asciz " (68060)\n"
text68080:
    .asciz " (68080)\n"
textold:
    .asciz " (68030 or older)\n"
textmemory:
    .asciz "memory:\n"
textindent:
    .asciz "  "
textdash:
    .asciz "-"
textspace:
    .asciz " "
textnewline:
    .asciz "\n"
textchipset:
    .asciz "display: DENISEID "
textaga:
    .asciz " (AGA)\n"
textnoaga:
    .asciz " (not AGA: no video, serial only)\n"
textnocpu:
    .asciz "needs a 68040, 68060 or 68080: fudge uses their MMU\n"
textnoram:
    .asciz "needs RAM from "
textto:
    .asciz " to "
textgo:
    .asciz "starting fudge in 5 seconds\n"
textstop:
    .asciz "stopped\n"
hexdigits:
    .ascii "0123456789ABCDEF"
hexstart:
    .ascii "0x"
    .space 8
hexend:
    .byte 0

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

