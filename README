# FUDGE OPERATING SYSTEM

http://www.fudgeos.com/

## OVERVIEW

Fudge is a small and elegant operating system. It started as a project to see if
it would be possible to create an operating system that came as close as
possible to be fully deterministic. Today, operating systems often show
unpredictable behaviour because they implement technical solutions that are not
determinstic in nature such as task scheduling, caching and dynamic memory
allocation. Fudge tries to find alternative solutions to these problems and has
so far been found to work quite well.

Determinism in the term of computers could mean many different things. It could
mean that a program, given input, should always return a conclusive result.
However unlikely that sounds it is actually quite common because of unwanted
side-effects such as time delays or incorrectly handled input. Determinism could
also mean that every time you execute a program it will always take the same
amount of time to complete given the same input and the same environment in
which it operates.

Fudge focus is on the latter where each programs should complete in the same
amount of time or as close as possible. This is not to be confused with a
real-time operating system that is only concerned about making sure each task is
guaranteed a certain amount of time. Here instead we are concerned with the
actual execution time. In order to achieve this Fudge will only run one task at
a time and will not interrupt the running task unless an interrupt occurs from
hardware or software. This does not mean that Fudge is limited to only run one
task at a time because tasks can delay there own execution until such an
interrupt occurs and take presedence if the interrupt is of the type it was
waiting for. While Fudge operates in kernel mode it can not be interrupted at
all so the kernel in that sense is fully deterministic. The kernel also does not
keep track of things in a dynamic fashion so each interrupt of the same type
should always take the exact same time as any previous. The kernel is not even
aware of the fact there can be many tasks running because it can only see the
current running one.

## BUILDING FUDGE

Please see the building instructions on the website.

## LICENCE

Please see the LICENCE file included in the source.

## COMMUNITY

Come talk to us on irc.

Server: irc.freenode.net
Channel: #fudge
