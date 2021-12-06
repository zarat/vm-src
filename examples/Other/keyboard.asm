main:

    int 6 ; was a key pressed?
    pop r1 ; store in r1
    
    ; if no key was pressed, we jump back to main
    ldr r1
    push 0
    eq
    jz main
    
    ; if a key was pressed, get in onto the stack and store it in r2
    int 7
    pop r2
    
    ; LINE FEED
    ldr r2
    push 13
    eq
    jz newline
    
    ; BACKSPACE
    ldr r2    
    push 8
    eq
    jz backspace
    
    ; ESCAPE
    ldr r2
    push 27
    eq
    jz end
    
    ; otherwise print it as a character
    call print_char
    
    jmp main
    
newline:
    push 10
    printc
    jmp main
    
backspace:
    ; step back
    push 8
    printc
    ; overwrite deleted character with empty space
    push 32 
    printc 
    ; step back again to overwrite the deleted character on next keypress
    push 8 
    printc
    jmp main
    
print_char:
    ldr r2
    printc
    ret

end:
    push "Shutting down.."
    si ax
    ldr ax
    push 100
    puts
    push 100
    write