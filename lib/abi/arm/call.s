.set CALL_INDEX_DEBUG,                  0x00
.set CALL_INDEX_WALK,                   0x01
.set CALL_INDEX_CREATE,                 0x02
.set CALL_INDEX_DESTROY,                0x03
.set CALL_INDEX_KILL,                   0x04
.set CALL_INDEX_STAT,                   0x05
.set CALL_INDEX_LIST,                   0x06
.set CALL_INDEX_READ,                   0x07
.set CALL_INDEX_LOAD,                   0x0A
.set CALL_INDEX_UNLOAD,                 0x0B
.set CALL_INDEX_SPAWN,                  0x0C
.set CALL_INDEX_DESPAWN,                0x0D
.set CALL_INDEX_PICK,                   0x0E
.set CALL_INDEX_PLACE,                  0x0F
.set CALL_INDEX_LINK,                   0x10
.set CALL_INDEX_UNLINK,                 0x11
.set CALL_INDEX_ANNOUNCE,               0x12

.section .text

.global call_announce
call_announce:
    mov r7, $CALL_INDEX_ANNOUNCE
    swi #0
    bx lr

.global call_create
call_create:
    mov r7, $CALL_INDEX_CREATE
    swi #0
    bx lr

.global call_debug
call_debug:
    mov r7, $CALL_INDEX_DEBUG
    swi #0
    bx lr

.global call_destroy
call_destroy:
    mov r7, $CALL_INDEX_DESTROY
    swi #0
    bx lr

.global call_despawn
call_despawn:
    mov r7, $CALL_INDEX_DESPAWN
    swi #0
    bx lr

.global call_kill
call_kill:
    mov r7, $CALL_INDEX_KILL
    swi #0
    bx lr

.global call_link
call_link:
    mov r7, $CALL_INDEX_LINK
    swi #0
    bx lr

.global call_list
call_list:
    mov r7, $CALL_INDEX_LIST
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

.global call_read
call_read:
    mov r7, $CALL_INDEX_READ
    swi #0
    bx lr

.global call_spawn
call_spawn:
    mov r7, $CALL_INDEX_SPAWN
    swi #0
    bx lr

.global call_stat
call_stat:
    mov r7, $CALL_INDEX_STAT
    swi #0
    bx lr

.global call_unlink
call_unlink:
    mov r7, $CALL_INDEX_UNLINK
    swi #0
    bx lr

.global call_unload
call_unload:
    mov r7, $CALL_INDEX_UNLOAD
    swi #0
    bx lr

.global call_walk
call_walk:
    mov r7, $CALL_INDEX_WALK
    swi #0
    bx lr

