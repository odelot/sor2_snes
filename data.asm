.include "hdr.asm"

.section ".rodata1" superfree

patterns1: .incbin "fase11BG1.pic" 
patterns1_end

patterns2: .incbin "fase11BG2.pic" 
patterns2_end

hud: .incbin "hud.pic" 
hud_end

.ends

.section ".rodata2" superfree

map: .incbin "fase11BG1.map"
map_end:

map2: .incbin "fase11BG2.map"
map2_end:

hud_map: .incbin "hud.map"
hud_map_end:

hud_pal: .incbin "hud.pal"
hud_pal_end:

palette: .incbin "fase11BG1.pal"
palette_end:

palette2: .incbin "fase11BG2.pal"
palette2_end:

caixa: .incbin "caixa.pic"
caixa_end:

caixa_palette: .incbin "caixa.pal"
caixa_palette_end:

.ends


.section ".rodata3" superfree

blaze:
.incbin "blaze.pic"
blaze_end:

blaze_palette:
.incbin "blaze.pal"
blaze_palette_end:




.ends

.section ".rodata4" superfree

signal_tiles:
.incbin "signal.pic"
signal_tiles_end:

signal_palette:
.incbin "signal.pal"
signal_palette_end:




.ends

.section ".rodata5" superfree


galsia_tiles:
.incbin "galsia.pic"
galsia_tiles_end:

galsia_palette:
.incbin "galsia.pal"
galsia_palette_end:

.ends

.section ".rodata6" superfree

socobrr:
.incbin "soco.brr"
socobrrend:


danobrr:
.incbin "dano.brr"
danobrrend:
.ends
