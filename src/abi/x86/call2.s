.code32

.set CALL2_INDEX_WALK,                  0x01
.set CALL2_INDEX_CREATE,                0x02
.set CALL2_INDEX_DESTROY,               0x03
.set CALL2_INDEX_OPEN,                  0x04
.set CALL2_INDEX_CLOSE,                 0x05
.set CALL2_INDEX_READ,                  0x06
.set CALL2_INDEX_WRITE,                 0x07
.set CALL2_INDEX_AUTH,                  0x08
.set CALL2_INDEX_MOUNT,                 0x09
.set CALL2_INDEX_LOAD,                  0x0A
.set CALL2_INDEX_UNLOAD,                0x0B
.set CALL2_INDEX_SPAWN,                 0x0C
.set CALL2_INDEX_DESPAWN,               0x0D
.set CALL2_INDEX_SEEK,                  0x0E

.global call2_auth
call2_auth:
    movl $CALL2_INDEX_AUTH, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

.global call2_close
call2_close:
    movl $CALL2_INDEX_CLOSE, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

.global call2_create
call2_create:
    movl $CALL2_INDEX_CREATE, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

.global call2_destroy
call2_destroy:
    movl $CALL2_INDEX_DESTROY, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

.global call2_despawn
call2_despawn:
    movl $CALL2_INDEX_DESPAWN, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

.global call2_load
call2_load:
    movl $CALL2_INDEX_LOAD, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

.global call2_mount
call2_mount:
    movl $CALL2_INDEX_MOUNT, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

.global call2_open
call2_open:
    movl $CALL2_INDEX_OPEN, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

.global call2_read
call2_read:
    movl $CALL2_INDEX_READ, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

.global call2_seek
call2_seek:
    movl $CALL2_INDEX_SEEK, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

.global call2_spawn
call2_spawn:
    movl $CALL2_INDEX_SPAWN, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

.global call2_unload
call2_unload:
    movl $CALL2_INDEX_UNLOAD, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

.global call2_walk
call2_walk:
    movl $CALL2_INDEX_WALK, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

.global call2_write
call2_write:
    movl $CALL2_INDEX_WRITE, %eax
    movl %esp, %ecx
    movl $call2_return, %edx
    sysenter

call2_return:
    ret

