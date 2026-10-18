.set CALL_INDEX_DEBUG,                  0x00
.set CALL_INDEX_PICK,                   0x01
.set CALL_INDEX_PLACE,                  0x02
.set CALL_INDEX_SPAWN,                  0x03
.set CALL_INDEX_DESPAWN,                0x04
.set CALL_INDEX_KILL,                   0x05
.set CALL_INDEX_FIND,                   0x06
.set CALL_INDEX_LOAD,                   0x07
.set CALL_INDEX_UNLOAD,                 0x08
.set CALL_INDEX_ANNOUNCE,               0x09
.set CALL_FRAME,                        32

# The kernel reads the arguments from the user stack in the same layout as on x86_64:
# {return address slot, arguments...} with each argument at its natural alignment.
.macro callindex index
    li a7, \index
    ecall
    addi sp, sp, CALL_FRAME
    ret
.endm

.section .text

.global call_debug
call_debug:
    addi sp, sp, -CALL_FRAME
    sd a0, 8(sp)
    callindex CALL_INDEX_DEBUG

.global call_despawn
call_despawn:
    addi sp, sp, -CALL_FRAME
    sw a0, 8(sp)
    callindex CALL_INDEX_DESPAWN

.global call_find
call_find:
    addi sp, sp, -CALL_FRAME
    sw a0, 8(sp)
    sd a1, 16(sp)
    sw a2, 24(sp)
    callindex CALL_INDEX_FIND

.global call_kill
call_kill:
    addi sp, sp, -CALL_FRAME
    sw a0, 8(sp)
    callindex CALL_INDEX_KILL

.global call_load
call_load:
    addi sp, sp, -CALL_FRAME
    sw a0, 8(sp)
    sw a1, 12(sp)
    callindex CALL_INDEX_LOAD

.global call_pick
call_pick:
    addi sp, sp, -CALL_FRAME
    sw a0, 8(sp)
    sd a1, 16(sp)
    callindex CALL_INDEX_PICK

.global call_place
call_place:
    addi sp, sp, -CALL_FRAME
    sw a0, 8(sp)
    sw a1, 12(sp)
    sw a2, 16(sp)
    sw a3, 20(sp)
    sd a4, 24(sp)
    callindex CALL_INDEX_PLACE

.global call_spawn
call_spawn:
    addi sp, sp, -CALL_FRAME
    sw a0, 8(sp)
    sw a1, 12(sp)
    sw a2, 16(sp)
    callindex CALL_INDEX_SPAWN

.global call_unload
call_unload:
    addi sp, sp, -CALL_FRAME
    sw a0, 8(sp)
    sw a1, 12(sp)
    callindex CALL_INDEX_UNLOAD

.global call_announce
call_announce:
    addi sp, sp, -CALL_FRAME
    sw a0, 8(sp)
    sw a1, 12(sp)
    sd a2, 16(sp)
    callindex CALL_INDEX_ANNOUNCE
