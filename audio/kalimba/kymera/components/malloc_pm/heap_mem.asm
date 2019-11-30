/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
.MODULE $M.pm_malloc;
.CODESEGMENT CODE_HEAP;
.MAXIM;
$_heap_pm:
/* We can have any dummy instruction here
   but use the pm_heap_size so we can
   see it on a kmap file */
Null = $__pm_heap_size;
.ENDMODULE;

#ifdef INSTALL_DUAL_CORE_SUPPORT
.MODULE $M.pm_malloc_p1;
.CODESEGMENT CODE_HEAP_P1;
.MAXIM;
$_heap_pm_p1:
/* We can have any dummy instruction here
   but use the __pm_p1_heap_size so we can
   see it on a kmap file */
Null = $__pm_p1_heap_size;
.ENDMODULE;
#endif
