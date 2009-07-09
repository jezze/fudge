extern idtp

global idt_flush
idt_flush:
	lidt [idtp]
	ret

