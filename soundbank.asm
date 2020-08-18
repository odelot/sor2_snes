;************************************************
; snesmod soundbank data                        *
; total size:      37400 bytes                  *
;************************************************

.include "hdr.asm"

.bank 5
.section "SOUNDBANK0" ; need dedicated bank(s)

__SOUNDBANK__0:
.incbin "soundbank.bnk" read $8000
.ends

.bank 6
.section "SOUNDBANK1" ; need dedicated bank(s)

__SOUNDBANK__1:
.incbin "soundbank.bnk" skip $8000
.ends

