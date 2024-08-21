.code32

.set CALL_INDEX_DEBUG,                  0x00
.set CALL_INDEX_WALK,                   0x01
.set CALL_INDEX_CREATE,                 0x02
.set CALL_INDEX_DESTROY,                0x03
.set CALL_INDEX_KILL,                   0x04
.set CALL_INDEX_STAT,                   0x05
.set CALL_INDEX_LIST,                   0x06
.set CALL_INDEX_READ,                   0x07
.set CALL_INDEX_ANNOUNCE,               0x08
.set CALL_INDEX_NOTIFY,                 0x09
.set CALL_INDEX_LOAD,                   0x0A
.set CALL_INDEX_UNLOAD,                 0x0B
.set CALL_INDEX_SPAWN,                  0x0C
.set CALL_INDEX_DESPAWN,                0x0D
.set CALL_INDEX_PICK,                   0x0E
.set CALL_INDEX_PLACE,                  0x0F
.set CALL_INDEX_LINK,                   0x10
.set CALL_INDEX_UNLINK,                 0x11

.section .text

.global call_announce
call_announce:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_ANNOUNCE, %eax
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

.global call_debug
call_debug:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_DEBUG, %eax
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

.global call_kill
call_kill:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_KILL, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_link
call_link:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_LINK, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_list
call_list:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_LIST, %eax
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

.global call_notify
call_notify:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_NOTIFY, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_pick
call_pick:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_PICK, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_place
call_place:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_PLACE, %eax
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

.global call_spawn
call_spawn:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_SPAWN, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_stat
call_stat:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_STAT, %eax
    movl %esp, %ecx
    movl $callreturn, %edx
    sysenter

.global call_unlink
call_unlink:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_UNLINK, %eax
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

callreturn:
    popl %edx
    popl %ecx
    ret

