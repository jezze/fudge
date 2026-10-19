.set CALL_INDEX_DEBUG,                  0x00
.set CALL_INDEX_PICK,                   0x01
.set CALL_INDEX_PLACE,                  0x02
.set CALL_INDEX_SPAWN,                  0x03
.set CALL_INDEX_DESPAWN,                0x04
.set CALL_INDEX_KILL,                   0x05
.set CALL_INDEX_FIND,                   0x06
.set CALL_INDEX_LOAD,                   0x07
.set CALL_INDEX_UNLOAD,                 0x08
.set CALL_INDEX_ANNOUNCE,               0x09

# The kernel reads the arguments from the user stack as {return address, arguments...}, like on
# x86: r0-r3 are pushed in front of the caller's stacked arguments, then lr. The index goes in r12.
.macro call name, index
.global call_\name
.type call_\name, %function
call_\name:
    push {r0-r3}
    push {lr}
    mov r12, #\index
    swi #0
    pop {lr}
    add sp, sp, #16
    bx lr
.endm

.section .text

call debug, CALL_INDEX_DEBUG
call despawn, CALL_INDEX_DESPAWN
call find, CALL_INDEX_FIND
call kill, CALL_INDEX_KILL
call load, CALL_INDEX_LOAD
call pick, CALL_INDEX_PICK
call place, CALL_INDEX_PLACE
call spawn, CALL_INDEX_SPAWN
call unload, CALL_INDEX_UNLOAD
call announce, CALL_INDEX_ANNOUNCE

