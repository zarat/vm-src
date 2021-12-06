push 0
push "echo %time%"
si ax
ldr ax
push 5 ; addr
puts

push 0
push "Timestamp:"
si ax
ldr ax
push 4 ; addr
puts

setup:

    int 10 ; ARITH_INT
    int 2 ; RW_INT
    
    ; [ 1 ] 1984
    push 1
    push 1 ; len
    push 1 ; addr
    puts
    
    ; [ 1 ] 1984 1960
    push 1
    push 1 ; addr
    push 4 ; pos
    stm
    
    push 1
    push 4
    ldm ; 1960
    
    ; [ 2 ] 1960
    push 1 ; len
    push 2 ; addr
    puts 
    
    push 1
    push 0
    ldm ; 1984

    ; [ 2 ] 1960 1984
    push 2
    push 4
    stm   

main: 

    int 2 

    push 2
    push 4
    ldm ; 1984
    pop r1
    
    add r1 1
    ldr r1
    push 2
    push 4
    stm 
    
    push 2
    push 0
    ldm ; 1960
    pop r2 
    
    add r2 1
    ldr r2
    push 2
    push 0
    stm 
    
    ; x + y =
    ldr r1 push 'd' print 
    push 32 printc
    push '+' printc 
    push 32 printc
    ldr r2 push 'd' print 
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
    push 3
    puts
    
    int 2 ; RW_INT
    push 3
    push 0
    ldm
    push 'd'
    print
    push 10
    printc 
    
    int 1 ; RW_CHAR
    
    ; Timestamp
    push 4
    write
    
    ; get time
    push 5 ; cmd
    push 6 ; dst
    prc
    
    push 6
    write
    
    ;push 10 printc
    
    jmp main

end: