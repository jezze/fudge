.set CALL_INDEX_DEBUG,                  0x00
.set CALL_INDEX_PICK,                   0x01
.set CALL_INDEX_PLACE,                  0x02
.set CALL_INDEX_SPAWN,                  0x03
.set CALL_INDEX_DESPAWN,                0x04
.set CALL_INDEX_KILL,                   0x05
.set CALL_INDEX_RAISE,                  0x06
.set CALL_INDEX_LOAD,                   0x07
.set CALL_INDEX_UNLOAD,                 0x08
.set CALL_INDEX_ANNOUNCE,               0x09
.set CALL_INDEX_COLLECT,                0x0A
.set CALL_FRAME,                        48

# The kernel reads the arguments from the user stack in the same layout as on x86_64:
# {return address slot, arguments...} with each argument at its natural alignment.
.macro callindex index
    mov x8, #\index
    svc #0
    add sp, sp, #CALL_FRAME
    ret
.endm

.section .text

.global call_debug
call_debug:
    sub sp, sp, #CALL_FRAME
    str x0, [sp, #8]
    callindex CALL_INDEX_DEBUG

.global call_despawn
call_despawn:
    sub sp, sp, #CALL_FRAME
    callindex CALL_INDEX_DESPAWN

.global call_kill
call_kill:
    sub sp, sp, #CALL_FRAME
    str w0, [sp, #8]
    callindex CALL_INDEX_KILL

.global call_load
call_load:
    sub sp, sp, #CALL_FRAME
    str w0, [sp, #8]
    str w1, [sp, #12]
    callindex CALL_INDEX_LOAD

.global call_pick
call_pick:
    sub sp, sp, #CALL_FRAME
    str w0, [sp, #8]
    str x1, [sp, #16]
    str w2, [sp, #24]
    str w3, [sp, #28]
    callindex CALL_INDEX_PICK

.global call_place
call_place:
    sub sp, sp, #CALL_FRAME
    str w0, [sp, #8]
    str w1, [sp, #12]
    str w2, [sp, #16]
    str w3, [sp, #20]
    str x4, [sp, #24]
    callindex CALL_INDEX_PLACE

.global call_spawn
call_spawn:
    sub sp, sp, #CALL_FRAME
    str w0, [sp, #8]
    str w1, [sp, #12]
    callindex CALL_INDEX_SPAWN

.global call_unload
call_unload:
    sub sp, sp, #CALL_FRAME
    str w0, [sp, #8]
    str w1, [sp, #12]
    callindex CALL_INDEX_UNLOAD

.global call_announce
call_announce:
    sub sp, sp, #CALL_FRAME
    str w0, [sp, #8]
    str w1, [sp, #12]
    str x2, [sp, #16]
    callindex CALL_INDEX_ANNOUNCE

.global call_collect
call_collect:
    sub sp, sp, #CALL_FRAME
    callindex CALL_INDEX_COLLECT

.global call_raise
call_raise:
    sub sp, sp, #CALL_FRAME
    str w0, [sp, #8]
    str w1, [sp, #12]
    callindex CALL_INDEX_RAISE
