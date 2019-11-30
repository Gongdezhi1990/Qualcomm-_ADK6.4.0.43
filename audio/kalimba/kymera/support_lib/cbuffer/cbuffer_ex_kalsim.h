#ifdef RUNNING_ON_KALSIM
// Use 'new' cbuffer function in the support lib
#define $cbuffer.get_read_address_ex                       $cbuffer.get_read_address_exx
#define $_cbuffer_get_read_address_ex                      $_cbuffer_get_read_address_exx
#define $_cbuffer_get_write_address_ex                     $_cbuffer_get_write_address_exx
#define $cbuffer.calc_amount_data_ex                       $cbuffer.calc_amount_data_exx
#define $_cbuffer_calc_amount_data_ex                      $_cbuffer_calc_amount_data_exx
#define $cbuffer.calc_amount_space_ex                      $cbuffer.calc_amount_space_exx
#define $_cbuffer_calc_amount_space_ex                     $_cbuffer_calc_amount_space_exx
#define $cbuffer.set_read_address_ex                       $cbuffer.set_read_address_exx
#define $_cbuffer_set_read_address_ex                      $_cbuffer_set_read_address_exx
#define $cbuffer.set_write_address_ex                      $cbuffer.set_write_address_exx
#define $_cbuffer_set_write_address_ex                     $_cbuffer_set_write_address_exx
#define $cbuffer.advance_write_ptr_ex                      $cbuffer.advance_write_ptr_exx
#define $_cbuffer_advance_write_ptr_ex                     $_cbuffer_advance_write_ptr_exx
#define $cbuffer.advance_read_ptr_ex                       $cbuffer.advance_read_ptr_exx
#define $_cbuffer_advance_read_ptr_ex                      $_cbuffer_advance_read_ptr_exx
#define $cbuffer.copy_aligned_16bit_be_zero_shift_ex       $cbuffer.copy_aligned_16bit_be_zero_shift_exx
#define $_cbuffer_copy_aligned_16bit_be_zero_shift_ex      $_cbuffer_copy_aligned_16bit_be_zero_shift_exx
#endif

