
align 16

rept 256 idx
{
  amd_ivt_entry#idx :
    vmmcall 
    db      idx-1
    iret
}

rept 256 idx
{
  intel_ivt_entry#idx :
    vmcall 
    db      idx-1
    iret
}


init_ivt_amd:   
    push    di
    push    es
    push    si
    push    cx

    mov     di,     0
    mov     es,     di

    mov     si,     amd_ivt_entry1
    mov     cx,     256
  .L0:
    mov     [es:di], si
    add     di,     2
    mov     [es:di], word 0xF000
    add     di,     2

    add     si,     amd_ivt_entry2 - amd_ivt_entry1
    loop    .L0
    
    pop     cx
    pop     si
    pop     es
    pop     di
    ret

init_ivt_intel:   
    push    di
    push    es
    push    si
    push    cx

    mov     di,     0
    mov     es,     di

    mov     si,     intel_ivt_entry1
    mov     cx,     256
  .L0:
    mov     [es:di], si
    add     di,     2
    mov     [es:di], word 0xF000
    add     di,     2

    add     si,     intel_ivt_entry2 - intel_ivt_entry1
    loop    .L0
    
    pop     cx
    pop     si
    pop     es
    pop     di
    ret    