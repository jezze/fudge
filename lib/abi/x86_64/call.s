.code64

.set CALL_INTERRUPT,                    0x80
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

.macro callindex index
    movq $\index, %rax
    int $CALL_INTERRUPT
    addq $CALL_FRAME, %rsp
    ret
.endm

.section .text

.global call_debug
call_debug:
    subq $CALL_FRAME, %rsp
    movq %rdi, 8(%rsp)
    callindex CALL_INDEX_DEBUG

.global call_despawn
call_despawn:
    subq $CALL_FRAME, %rsp
    movl %edi, 8(%rsp)
    callindex CALL_INDEX_DESPAWN

.global call_find
call_find:
    subq $CALL_FRAME, %rsp
    movl %edi, 8(%rsp)
    movq %rsi, 16(%rsp)
    movl %edx, 24(%rsp)
    callindex CALL_INDEX_FIND

.global call_kill
call_kill:
    subq $CALL_FRAME, %rsp
    movl %edi, 8(%rsp)
    callindex CALL_INDEX_KILL

.global call_load
call_load:
    subq $CALL_FRAME, %rsp
    movl %edi, 8(%rsp)
    callindex CALL_INDEX_LOAD

.global call_pick
call_pick:
    subq $CALL_FRAME, %rsp
    movl %edi, 8(%rsp)
    movq %rsi, 16(%rsp)
    callindex CALL_INDEX_PICK

.global call_place
call_place:
    subq $CALL_FRAME, %rsp
    movl %edi, 8(%rsp)
    movl %esi, 12(%rsp)
    movl %edx, 16(%rsp)
    movl %ecx, 20(%rsp)
    movq %r8, 24(%rsp)
    callindex CALL_INDEX_PLACE

.global call_spawn
call_spawn:
    subq $CALL_FRAME, %rsp
    movl %edi, 8(%rsp)
    movl %esi, 12(%rsp)
    callindex CALL_INDEX_SPAWN

.global call_unload
call_unload:
    subq $CALL_FRAME, %rsp
    movl %edi, 8(%rsp)
    callindex CALL_INDEX_UNLOAD

.global call_announce
call_announce:
    subq $CALL_FRAME, %rsp
    movl %edi, 8(%rsp)
    movl %esi, 12(%rsp)
    movq %rdx, 16(%rsp)
    callindex CALL_INDEX_ANNOUNCE
