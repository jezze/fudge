.code32

.set CALL_INTERRUPT,                    0x80
.set CALL_INDEX_AUTH,                   0x00
.set CALL_INDEX_WALK,                   0x01
.set CALL_INDEX_CREATE,                 0x02
.set CALL_INDEX_DESTROY,                0x03
.set CALL_INDEX_OPEN,                   0x04
.set CALL_INDEX_CLOSE,                  0x05
.set CALL_INDEX_READ,                   0x06
.set CALL_INDEX_WRITE,                  0x07
.set CALL_INDEX_MOUNT,                  0x08
.set CALL_INDEX_BIND,                   0x09
.set CALL_INDEX_LOAD,                   0x0A
.set CALL_INDEX_UNLOAD,                 0x0B
.set CALL_INDEX_SPAWN,                  0x0C
.set CALL_INDEX_DESPAWN,                0x0D

.global call_auth
call_auth:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_AUTH, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global call_bind
call_bind:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_BIND, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global call_close
call_close:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_CLOSE, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global call_create
call_create:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_CREATE, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global call_destroy
call_destroy:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_DESTROY, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global call_despawn
call_despawn:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_DESPAWN, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global call_load
call_load:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_LOAD, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global call_mount
call_mount:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_MOUNT, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global call_open
call_open:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_OPEN, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global call_read
call_read:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_READ, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global call_spawn
call_spawn:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_SPAWN, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global call_unload
call_unload:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_UNLOAD, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global call_walk
call_walk:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_WALK, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global call_write
call_write:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
    movl $CALL_INDEX_WRITE, %eax
    int $CALL_INTERRUPT
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

