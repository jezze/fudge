.set CALL_INTERRUPT,                    0x80
.set CALL_INDEX_OPEN,                   0x01
.set CALL_INDEX_CLOSE,                  0x02
.set CALL_INDEX_READ,                   0x03
.set CALL_INDEX_WRITE,                  0x04
.set CALL_INDEX_MOUNT,                  0x05
.set CALL_INDEX_BIND,                   0x06
.set CALL_INDEX_EXECUTE,                0x07
.set CALL_INDEX_EXIT,                   0x08
.set CALL_INDEX_LOAD,                   0x09
.set CALL_INDEX_UNLOAD,                 0x0A
.set CALL_INDEX_SPAWN,                  0x0B

.global call_bind
call_bind:
    movl $CALL_INDEX_BIND, %eax
    int $CALL_INTERRUPT
    ret

.global call_close
call_close:
    movl $CALL_INDEX_CLOSE, %eax
    int $CALL_INTERRUPT
    ret

.global call_execute
call_execute:
    movl $CALL_INDEX_EXECUTE, %eax
    int $CALL_INTERRUPT
    ret

.global call_exit
call_exit:
    movl $CALL_INDEX_EXIT, %eax
    int $CALL_INTERRUPT
    ret

.global call_load
call_load:
    movl $CALL_INDEX_LOAD, %eax
    int $CALL_INTERRUPT
    ret

.global call_mount
call_mount:
    movl $CALL_INDEX_MOUNT, %eax
    int $CALL_INTERRUPT
    ret

.global call_open
call_open:
    movl $CALL_INDEX_OPEN, %eax
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

.global call_unload
call_unload:
    movl $CALL_INDEX_UNLOAD, %eax
    int $CALL_INTERRUPT
    ret

.global call_write
call_write:
    movl $CALL_INDEX_WRITE, %eax
    int $CALL_INTERRUPT
    ret

