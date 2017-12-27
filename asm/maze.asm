start:  LD      I, #21e
        RND     V2, #1
        SE      V2, #1
        LD      I, spr
        DRW     V0, V1, #4
        ADD     V0, #4
        SE      V0, #40
        JP      start
        LD      V0, #0
        ADD     V1, #4
        SE      V1, #20
        JP      start
        JP      #218            ; what??

spr:    DW      #8040
        DW      #2010
        DW      #2040
        DW      #8010
