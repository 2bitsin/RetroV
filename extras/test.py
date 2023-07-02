with open('PVGA_PC4X_EVEN.BIN', 'rb') as fe:
    even = fe.read()

with open('PVGA_PC4X_ODD.BIN', 'rb') as fo:
    odd = fo.read()

with open('combined.bin', 'wb') as fw:
    for e, o in zip(even, odd):
        fw.write(bytes([e]))
        fw.write(bytes([o]))