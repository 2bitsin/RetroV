    org 0x0000

start_here:
    mov         ax,     cs
    mov         ds,     ax
    mov         es,     ax
    mov         fs,     ax
    mov         gs,     ax
    mov         ax,     0x0050
    mov         ss,     ax
    mov         sp,     0x7700
    cld
    mov         si,     strings.hello_world
    call        print_e9
    hlt


print_e9:
    push				si
    push				ax
  .print:
    lodsb
    or          al,     al
    jz          .done
    out         0xE9,   al
    jmp         .print
  .done:
    pop					ax
    pop					si
    ret


strings:
    .hello_world: db "Hello, world!", 0x0D, 0x0A, 0x00    

    times 0xFFF0 - ($ - $$) db 0x00
entry_point:
    jmp short start_here
    times 0x10000 - ($ - $$) db 0x00

