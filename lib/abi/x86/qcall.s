.code32

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
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_DEBUG, %eax
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

.global call_find
call_find:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_FIND, %eax
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

.global call_load
call_load:
    pushl %ecx
    pushl %edx
    movl $CALL_INDEX_LOAD, %eax
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

callreturn:
    popl %edx
    popl %ecx
    ret

