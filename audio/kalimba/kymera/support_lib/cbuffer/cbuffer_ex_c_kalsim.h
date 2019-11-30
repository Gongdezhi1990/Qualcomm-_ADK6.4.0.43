#ifdef RUNNING_ON_KALSIM
// Use 'new' cbuffer function in the support lib
#define get_read_address_ex                      get_read_address_exx
#define get_write_address_ex                     get_write_address_exx
#define calc_amount_data_ex                      calc_amount_data_exx
#define calc_amount_space_ex                     calc_amount_space_exx
#define set_read_address_ex                      set_read_address_exx
#define set_write_address_ex                     set_write_address_exx
#define advance_write_ptr_ex                     advance_write_ptr_exx
#define advance_read_ptr_ex                      advance_read_ptr_exx
#define copy_aligned_16bit_be_zero_shift_ex      copy_aligned_16bit_be_zero_shift_exx
#endif

