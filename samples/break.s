	.global _start
	.global _exit

_start:
	mov	r0, #0
	mov r1, #1

loop:
	cmp	r0, #4
	bhs	end
	add	r1, r1, r0
	add	r0, r0, #1
	b	loop
end:

_exit:
	b	_exit
