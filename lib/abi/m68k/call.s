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

# The kernel reads the arguments from the user stack as {return address, arguments...}, which is how the
# caller left them. The index goes in d0 and so does the result.
.macro call name, index
.global call_\name
call_\name:
    moveq #\index, %d0
    trap #0
    rts
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

