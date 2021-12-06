int 2 ; RW_INT

push 12
push 1
push 1
puts ; addr 1:0

push 23
push 1
push 4
stm ; addr 1:4

push 1
push 0
ldm
pop r7 ; laod 1:0 into r7

push 1
push 4
ldm
pop r8 ; load 1:4 into r8

int 3
int 4
int 5

inc r7; increment r7
inc r8; increment r8

int 3
int 4
int 5

ldr r7
push 1
push 0
stm ; load r7 and store into 1:0

ldr r8
push 1
push 4
stm ; load r8 and store into 1:4

int 3
int 4
int 5