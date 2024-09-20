.set CALL_INDEX_DEBUG,                  0x00
.set CALL_INDEX_PICK,                   0x01
.set CALL_INDEX_PLACE,                  0x02
.set CALL_INDEX_SPAWN,                  0x03
.set CALL_INDEX_DESPAWN,                0x04
.set CALL_INDEX_KILL,                   0x05
.set CALL_INDEX_FIND,                   0x06
.set CALL_INDEX_LOAD,                   0x07
.set CALL_INDEX_UNLOAD,                 0x08

.section .text

.global call_debug
call_debug:
    mov r7, $CALL_INDEX_DEBUG
    swi #0
    bx lr

.global call_despawn
call_despawn:
    mov r7, $CALL_INDEX_DESPAWN
    swi #0
    bx lr

.global call_find
call_find:
    mov r7, $CALL_INDEX_FIND
    swi #0
    bx lr

.global call_kill
call_kill:
    mov r7, $CALL_INDEX_KILL
    swi #0
    bx lr

.global call_load
call_load:
    mov r7, $CALL_INDEX_LOAD
    swi #0
    bx lr

.global call_pick
call_pick:
    mov r7, $CALL_INDEX_PICK
    swi #0
    bx lr

.global call_place
call_place:
    mov r7, $CALL_INDEX_PLACE
    swi #0
    bx lr

.global call_spawn
call_spawn:
    mov r7, $CALL_INDEX_SPAWN
    swi #0
    bx lr

.global call_unload
call_unload:
    mov r7, $CALL_INDEX_UNLOAD
    swi #0
    bx lr

