;
; Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
;
; FILE
;   panic_on_assert_very_brief.s

.text

#include "macros.s"

;
; void panic_on_assert_very_brief(uint16 line_num) /* no return */
;
; \pre  x = return address = call address + 1; al = line_num
;
panic_on_assert_very_brief:
.global panic_on_assert_very_brief
    ;
    ; Derive the calling address from X and pass on to a C function
    ; to do any logging and panic.
    ;
    enterl  #3              ; XH, XL & line num
    st      al, @(0,Y)      ; line_num -> stack[0]
    sub     x, #1           ; call addr = return addr - 1
    ld      al, @$XAP_UXL   ; AL <- call addr L
    ld      ah, @$XAP_UXH   ; AH <- call addr H
    bsr     panic_on_assert_very_brief_impl
