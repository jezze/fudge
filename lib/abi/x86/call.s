.code32

.set CALL_INTERRUPT,                    0x80
.set CALL_INDEX_DEBUG,                  0x00
.set CALL_INDEX_WALK,                   0x01
.set CALL_INDEX_CREATE,                 0x02
.set CALL_INDEX_DESTROY,                0x03
.set CALL_INDEX_KILL,                   0x04
.set CALL_INDEX_STAT,                   0x05
.set CALL_INDEX_LIST,                   0x06
.set CALL_INDEX_READ,                   0x07
.set CALL_INDEX_WRITE,                  0x08
.set CALL_INDEX_NOTIFY,                 0x09
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
    movl $CALL_INDEX_ANNOUNCE, %eax
    int $CALL_INTERRUPT
    ret

.global call_create
call_create:
    movl $CALL_INDEX_CREATE, %eax
    int $CALL_INTERRUPT
    ret

.global call_debug
call_debug:
    movl $CALL_INDEX_DEBUG, %eax
    int $CALL_INTERRUPT
    ret

.global call_destroy
call_destroy:
    movl $CALL_INDEX_DESTROY, %eax
    int $CALL_INTERRUPT
    ret

.global call_despawn
call_despawn:
    movl $CALL_INDEX_DESPAWN, %eax
    int $CALL_INTERRUPT
    ret

.global call_kill
call_kill:
    movl $CALL_INDEX_KILL, %eax
    int $CALL_INTERRUPT
    ret

.global call_link
call_link:
    movl $CALL_INDEX_LINK, %eax
    int $CALL_INTERRUPT
    ret

.global call_list
call_list:
    movl $CALL_INDEX_LIST, %eax
    int $CALL_INTERRUPT
    ret

.global call_load
call_load:
    movl $CALL_INDEX_LOAD, %eax
    int $CALL_INTERRUPT
    ret

.global call_notify
call_notify:
    movl $CALL_INDEX_NOTIFY, %eax
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

.global call_read
call_read:
    movl $CALL_INDEX_READ, %eax
    int $CALL_INTERRUPT
    ret

.global call_spawn
call_spawn:
    movl $CALL_INDEX_SPAWN, %eax
    int $CALL_INTERRUPT
    ret

.global call_stat
call_stat:
    movl $CALL_INDEX_STAT, %eax
    int $CALL_INTERRUPT
    ret

.global call_unlink
call_unlink:
    movl $CALL_INDEX_UNLINK, %eax
    int $CALL_INTERRUPT
    ret

.global call_unload
call_unload:
    movl $CALL_INDEX_UNLOAD, %eax
    int $CALL_INTERRUPT
    ret

.global call_walk
call_walk:
    movl $CALL_INDEX_WALK, %eax
    int $CALL_INTERRUPT
    ret

.global call_write
call_write:
    movl $CALL_INDEX_WRITE, %eax
    int $CALL_INTERRUPT
    ret

