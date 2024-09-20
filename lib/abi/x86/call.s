.code32

.set CALL_INTERRUPT,                    0x80
.set CALL_INDEX_DEBUG,                  0x00
.set CALL_INDEX_PICK,                   0x01
.set CALL_INDEX_PLACE,                  0x02
.set CALL_INDEX_SPAWN,                  0x03
.set CALL_INDEX_DESPAWN,                0x04
.set CALL_INDEX_KILL,                   0x05
.set CALL_INDEX_FIND,                   0x06
.set CALL_INDEX_LOAD,                   0x0A
.set CALL_INDEX_UNLOAD,                 0x0B

.section .text

.global call_debug
call_debug:
    movl $CALL_INDEX_DEBUG, %eax
    int $CALL_INTERRUPT
    ret

.global call_despawn
call_despawn:
    movl $CALL_INDEX_DESPAWN, %eax
    int $CALL_INTERRUPT
    ret

.global call_find
call_find:
    movl $CALL_INDEX_FIND, %eax
    int $CALL_INTERRUPT
    ret

.global call_kill
call_kill:
    movl $CALL_INDEX_KILL, %eax
    int $CALL_INTERRUPT
    ret

.global call_load
call_load:
    movl $CALL_INDEX_LOAD, %eax
    int $CALL_INTERRUPT
    ret

.global call_pick
call_pick:
    movl $CALL_INDEX_PICK, %eax
    int $CALL_INTERRUPT
    ret

.global call_place
call_place:
    movl $CALL_INDEX_PLACE, %eax
    int $CALL_INTERRUPT
    ret

.global call_spawn
call_spawn:
    movl $CALL_INDEX_SPAWN, %eax
    int $CALL_INTERRUPT
    ret

.global call_unload
call_unload:
    movl $CALL_INDEX_UNLOAD, %eax
    int $CALL_INTERRUPT
    ret

