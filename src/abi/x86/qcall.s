.code32

.set CALL_INDEX_WALK,                   0x01
.set CALL_INDEX_CREATE,                 0x02
.set CALL_INDEX_DESTROY,                0x03
.set CALL_INDEX_OPEN,                   0x04
.set CALL_INDEX_CLOSE,                  0x05
.set CALL_INDEX_READ,                   0x06
.set CALL_INDEX_WRITE,                  0x07
.set CALL_INDEX_AUTH,                   0x08
.set CALL_INDEX_MOUNT,                  0x09
.set CALL_INDEX_LOAD,                   0x0A
.set CALL_INDEX_UNLOAD,                 0x0B
.set CALL_INDEX_SPAWN,                  0x0C
.set CALL_INDEX_DESPAWN,                0x0D
.set CALL_INDEX_SEEK,                   0x0E

.global call_auth
call_auth:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_AUTH, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_close
call_close:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_CLOSE, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_create
call_create:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_CREATE, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_destroy
call_destroy:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_DESTROY, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_despawn
call_despawn:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_DESPAWN, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_load
call_load:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_LOAD, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_mount
call_mount:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_MOUNT, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_open
call_open:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_OPEN, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_read
call_read:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_READ, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_seek
call_seek:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_SEEK, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_spawn
call_spawn:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_SPAWN, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_unload
call_unload:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_UNLOAD, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_walk
call_walk:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_WALK, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_write
call_write:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_WRITE, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

callreturn:
    popl %edx
    popl %ecx
    ret

