.section .text

.globl buffer_copy
buffer_copy:
    cbz w2, buffer_copy_done
    ldrb w3, [x1], #1
    strb w3, [x0], #1
    sub w2, w2, #1
    b buffer_copy

buffer_copy_done:
    ret

.globl buffer_clear
buffer_clear:
    cbz w1, buffer_clear_done
    strb wzr, [x0], #1
    sub w1, w1, #1
    b buffer_clear

buffer_clear_done:
    ret

