push 0
push "echo%time%"
si ax
ldr ax
push 10
puts

;main:


int 2 ; RW_INT

; [ 3 ] c0 07 00 00
push 1984
push 1 ; len
push 3 ; addr
puts

; [ 3 ] c0 07 00 00 a8 07 00 00
push 1960
push 3 ; addr
push 4 ; pos
stm

push 3
push 4
ldm ; 1960

; [ 4 ] a8 07 00 00
push 1 ; len
push 4 ; addr
puts 

push 3
push 0
ldm ; 1984


; [ 4 ] a8 07 00 00 c0 07 00 00
push 4
push 4
stm   

main: 

;int 3 ; stackdump
;int 4 ; memdump
;int 5 ; regdump

int 2 

push 4
push 4
ldm ; 1984
pop r1

add r1 1
ldr r1
push 4
push 0
stm 

push 4
push 0
ldm ; 1960
pop r2 

add r2 1
ldr r2
push 4
push 4
stm 

; x + y =
ldr r1 print 
push 32 printc
push '+' printc 
push 32 printc
ldr r2 print 
push 32 printc
push '=' printc
push 32 printc

; add it together
mov r3 0
add r3 r1
add r3 r2

; store at #6
ldr r3
push 1
push 6
puts

int 2 ; RW_INT
push 6
push 0
ldm
print
push 10
printc 

int 1 ; RW_CHAR

push "Timestamp: "
si ax
ldr ax
push 5
puts

push 5
write

; get time
push 10 ; cmd
push 11 ; dst
prc

push 11
write

push 10
printc

jmp main

end: