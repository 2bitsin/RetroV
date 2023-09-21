----|---|----------|------|----------|----------|---|------|--------------------------    
MODE|T/G| COLSxROWS| FONT | RESOL.   | COLORS   | ? | ADDR | ADAPTERS
----|---|----------|------|----------|----------|---|------|--------------------------    
00h | T | 40x25    | 9x16 | 360x400  |   16     | 8 | B800 | VGA
01h | T | 40x25    | 9x16 | 360x400  |   16     | 8 | B800 | VGA
02h | T | 80x25    | 9x16 | 720x400  |   16     | 8 | B800 | VGA
03h | T | 80x50    | 8x8  | 640x400  |   16     | 4 | B800 | VGA [17]
04h | G | 40x25    | 8x8  | 320x200  |    4     | . | B800 | CGA,PCjr,EGA,MCGA,VGA
05h | G | 40x25    | 8x8  | 320x200  |    4     | . | B800 | MCGA,VGA
06h | G | 80x25    | 8x8  | 640x200  |    2     | . | B800 | CGA,PCjr,EGA,MCGA,VGA    
07h | T | 80x25    | 9x16 | 720x400  | mono     | . | B000 | VGA
08h | G | 20x25    | 8x8  | 160x200	 |   16     | . |      | PCjr, Tandy 1000
09h | G | 40x25    | 8x8  | 320x200  |   16     | . |      | PCjr, Tandy 1000
0Ah | G | 80x25    | 8x8  | 640x200  |    4     | . |      | PCjr, Tandy 1000
0Bh |   | reserved |      |          |          |   |      | (EGA BIOS internal use)    
0Ch |   | reserved |      |          |          |   |      | (EGA BIOS internal use)
0Dh | G | 40x25    | 8x8  | 320x200  |   16     | 8 | A000 | EGA,VGA
0Eh | G | 80x25    | 8x8  | 640x200  |   16     | 4 | A000 | EGA,VGA
0Fh | G | 80x25    | 8x14 | 640x350  | mono     | 2 | A000 | EGA,VGA
10h | G | 80x25    | 8x14 | 640x350  |   16     | . | A000 | 256k EGA,VGA
11h | G | 80x30    | 8x16 | 640x480  | mono     | . | A000 | VGA,MCGA,ATI EGA,ATI VIP
12h | G | 80x30    | 8x16 | 640x480  |  16/256K | . | A000 | VGA,ATI VIP
13h | G | 40x25    | 8x8  | 320x200  | 256/256K | . | A000 | VGA,MCGA,ATI VIP