/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Memory Management Unit Processor Port Interface.
 */

#ifndef MMU_PROC_PORT_H
#define MMU_PROC_PORT_H

#define hal_vm_clear_port_cpu_read() \
        hal_set_reg_vm_cpu1_clear_port(VM_CPU1_REQUESTS_READ_MASK)
#define hal_vm_clear_port_cpu_write() \
        hal_set_reg_vm_cpu1_clear_port(VM_CPU1_REQUESTS_WRITE_MASK)

#ifdef CHIP_DEF_REMOTE_VM_MAPPING_BUG_B_178412
/*
 * To add a delay so that the write to the VM remote buffer register
 * can take effect before any following reads or writes, we read back the
 * value we have just set.
 */
#define MMU_REMOTE_WRITE_PORT_MAP_WORKAROUND   \
                    (void)hal_get_reg_vm_request_write_remote_config(),
#define MMU_REMOTE_READ_PORT_MAP_WORKAROUND   \
                    (void)hal_get_reg_vm_request_read_remote_config(),
#else
#define MMU_REMOTE_WRITE_PORT_MAP_WORKAROUND
#define MMU_REMOTE_READ_PORT_MAP_WORKAROUND
#endif

/**
 * MMU Processor Port saved state.
 */
typedef struct mmu_proc_port_state
{
    mmu_handle                  buffer_handle;
} mmu_proc_port_state;

/**
 * Number of bytes guaranteed to be visible within a memory mapped window
 * starting at a particular buffer offset.
 *
 * Note that if the mapped buffer ends within the window then its contents
 * will get wrapped to fill the window (repeatedly if necessary).
 *
 * \sa  mmu_read_port_map_8bit() etc.
 * \sa  mmu_write_port_map_8bit() etc.
 */
#define MMU_VISIBLE_BYTES       (1UL << MMU_WINDOW_ADDR_BITS_PER_BUFFER)
#define MMU_VISIBLE_WORDS       (MMU_VISIBLE_BYTES / sizeof(uint))


#if defined(DESKTOP_TEST_BUILD) && !defined(COV_TEST_BUILD)

extern void mmu_read_port_open(mmu_handle h);
extern void mmu_write_port_open(mmu_handle h);
extern void mmu_read_port_close(void);
extern void mmu_write_port_close(void);
extern void mmu_read_port_save(mmu_proc_port_state *saved_state);
extern void mmu_write_port_save(mmu_proc_port_state *saved_state);
extern void mmu_read_port_restore(const mmu_proc_port_state* saved_state);
extern void mmu_write_port_restore(const mmu_proc_port_state* saved_state);

extern const uint8* mmu_read_port_map_8bit(mmu_handle h, mmu_offset offset);
extern const uint16* mmu_read_port_map_16bit_le(mmu_handle h, mmu_offset offset);
extern const uint16* mmu_read_port_map_16bit_be(mmu_handle h, mmu_offset offset);
extern uint8* mmu_write_port_map_8bit(mmu_handle h, mmu_offset offset);
extern uint16* mmu_write_port_map_16bit_le(mmu_handle h, mmu_offset offset);
extern uint16* mmu_write_port_map_16bit_be(mmu_handle h, mmu_offset offset);


/**
 * Macros to map either a remote or local buffer for reading/writing
 */
#define mmu_read_port_map_remote_or_local_8bit(h,o)  mmu_read_port_map_8bit((h),(o));
#define mmu_read_port_map_remote_or_local_16bit_le(h,o) mmu_read_port_map_16bit_le((h),(o));
#define mmu_write_port_map_remote_or_local_8bit(h,o) mmu_write_port_map_8bit((h),(o));
#define mmu_write_port_map_remote_or_local_16bit_le(h,o) mmu_write_port_map_16bit_le((h),(o));
#define mmu_write_port_map_remote_8bit(h,o) mmu_write_port_map_8bit((h),(o));

#define MMU_INDEX_FROM_ADDRESS_SPACE(ptr)    (0)    /* not yet implemented */

#else /* DESKTOP_TEST_BUILD && !COV_TEST_BUILD */

/**
 * Reserve the MMU Processor Read Port for read-only access to the
 * specified Buffer.
 *
 * To map specific regions of the buffer use \c mmu_read_port_seek() when the
 * port is open.
 *
 * This call shall be matched with a call to \c mmu_read_port_close() when
 * the port is finished with.
 *
 *\design   Open could be defined to map offset 0 and return a ptr. However
 *          no known clients would benefit from this.
 *
 *\design   Open could have been combined with specifying an initial offset
 *          and returning a ptr. This would be convenient in some cases but
 *          would lead to a wider interface as a variant would be required for
 *          each mapping mode (as is unavoidable for the mmu_read_port_map_*()
 *          mapping functions).
 *
 *\pre      mmu_read_port_is_closed(port)
 *\pre      mmu_handle_is_valid(buffer)
 *
 *\sa       mmu_read_port_close()
 *\sa       mmu_read_port_seek()
 */
#define mmu_read_port_open(buffer_handle)           ((void)buffer_handle)

/**
 * Release the MMU Processor Read Port.
 *
 * This shall be called once the port is finished with to allow its re-use.
 *
 *\pre      mmu_proc_port_is_open(port)
 */
#define mmu_read_port_close(void)                   hal_vm_clear_port_cpu_read()

/**
 * Configure the MMU Processor Read Port to map the buffer region read-only
 * in "8bit" mode.
 *
 *\returns  Memory address mapped to specified buffer offset.
 *          The buffer will appear wrapped multiple times.
 *
 */
#define mmu_read_port_map_8bit(buffer_handle, offset)   \
                            (const uint8 *)(PxD_VM_BUFFER_LOWER + \
                             (mmu_handle_index(buffer_handle) << \
                              MMU_WINDOW_ADDR_BITS_PER_BUFFER) + offset)

/**
 * Configure the MMU Processor Remote Buffer Read Port to map the buffer
 * region read-only in "8bit" mode. Note that only one remote buffer may
 * be mapped for reading at any time. If used from interrupt the read remote
 * config register must be saved and restored around its use so background
 * use isn't disturbed.
 *
 *\returns  Memory address mapped to specified buffer offset.
 *          The buffer will appear wrapped multiple times.
 */
#define mmu_read_port_map_remote_8bit(buffer_handle, offset)   \
    (hal_set_reg_vm_request_read_remote_config(buffer_handle), \
    MMU_REMOTE_READ_PORT_MAP_WORKAROUND                        \
    (const uint8 *)(PxD_VM_BUFFER_LOWER + (offset)))

/**
 * Configure the MMU Processor Read Port to map the buffer region read-only
 * in 16 bit Little-Endian mode.
 *
 * The buffer octets are mapped to 16bit word in memory according to:-
 *
 *      0xXX,0xYY in buffer == 0xYYXX in memory.
 *
 * The offset must be even.
 *
 * \pre     is_even(buffer_octet_offset)
 *
 * \sa      mmu_read_port_map_8bit() for more detail.
 */
#define mmu_read_port_map_16bit_le(buffer_handle, offset)   \
                            (const uint16 *)(PxD_VM_BUFFER_LOWER + \
                             (mmu_handle_index(buffer_handle) << \
                              MMU_WINDOW_ADDR_BITS_PER_BUFFER) + offset)

/**
 * Configure the MMU Processor Remote Buffer Read Port to map the buffer
 * region read-only in 16 bit mode. Note that only one remote buffer may
 * be mapped for reading at any time. If used from interrupt the read remote
 * config register must be saved and restored around its use so background
 * use isn't disturbed.
 * The offset must be even.
 *
 *\returns  Memory address mapped to specified buffer offset.
 *          The buffer will appear wrapped multiple times.
 *
 * \pre     is_even(buffer_octet_offset)
 *
 * \sa      mmu_read_port_map_8bit() for more detail.
 */
#define mmu_read_port_map_remote_16bit(buffer_handle, offset)   \
    (hal_set_reg_vm_request_read_remote_config(buffer_handle), \
    MMU_REMOTE_READ_PORT_MAP_WORKAROUND                        \
    (const uint16 *)(PxD_VM_BUFFER_LOWER + (offset)))

/**
 * Note: 16 bit Big-Endian port mapping mode is not supported by the hardware
 */

/**
 * Return the mmu index given a pointer into the buffer data space area -
 * for example one returned by \c mmu_read_port_map_8bit().
 */
#define MMU_INDEX_FROM_ADDRESS_SPACE(ptr)    \
    (((uint32)(ptr) - PxD_VM_BUFFER_LOWER) >> MMU_WINDOW_ADDR_BITS_PER_BUFFER)

/**
 * Saves the current state of MMU Processor Read Port and leaves it in closed
 * state available for re-opening.
 *
 * This shall be called in a matching pair with mmu_read_port_restore()
 * to save and restore the port state around a nested use, such as within
 * interrupt.
 *
 * \post     mmu_read_port_is_closed()
 *
 * \sa       mmu_read_port_restore()
 */
#define mmu_read_port_save(saved_state)     ((saved_state)->buffer_handle = \
                    (mmu_handle)hal_get_reg_vm_request_read_remote_config())

/**
 * Restores the state of MMU Processor Read Port for previously saved state.
 *
 * This shall be called in a matching pair with mmu_read_port_save_state()
 * to save and restore the port state around a nested use, such as within
 * interrupt.
 *
 * \pre     mmu_read_port_is_closed()
 *
 * \sa      mmu_read_port_save()
 */
#define mmu_read_port_restore(saved_state)      \
        hal_set_reg_vm_request_read_remote_config((saved_state)->buffer_handle)

/**
 * Open the MMU Processor Write Port for write-only access to the
 * specified Buffer.
 *
 * \see     mmu_read_port_open() for more detail.
 */
#define mmu_write_port_open(buffer)             ((void)buffer)

/**
 * Close the MMU Processor Write Port.
 *
 * \see     mmu_read_port_close() for more detail.
 */
#define mmu_write_port_close()                  hal_vm_clear_port_cpu_write()

/**
 * Configure the MMU Processor Write Port to map the buffer region write-only
 * in "8bit" mode.
 *
 * \see     mmu_read_port_map_8bit() for more detail.
 *
 * \sa      mmu_write_port_map_16bit_le()
 * \sa      mmu_write_port_map_16bit_be()
 */
#define mmu_write_port_map_8bit(buffer_handle, offset)   \
                            (uint8 *)(PxD_VM_BUFFER_LOWER + \
                             (mmu_handle_index(buffer_handle) << \
                              MMU_WINDOW_ADDR_BITS_PER_BUFFER) + offset)

/**
 * Configure the MMU Processor Remote Buffer Write Port to map the buffer
 * region read-only in 8 bit mode. Note that only one remote buffer may
 * be mapped for writing at any time. If used from interrupt the write remote
 * config register must be saved and restored around its use so background
 * use isn't disturbed.
 *
 * \see     mmu_write_port_map_8bit() for more detail.
 *
 * \sa      mmu_write_port_map_remote_16bit()
 */
#define mmu_write_port_map_remote_8bit(buffer_handle, offset)   \
    (hal_set_reg_vm_request_write_remote_config(buffer_handle), \
    MMU_REMOTE_WRITE_PORT_MAP_WORKAROUND                        \
    (uint8 *)(PxD_VM_BUFFER_LOWER + (offset)))

/**
 * Configure the MMU Processor Write Port to map the buffer region write-only
 * in 16 bit Little-Endian mode.
 *
 * \sa      mmu_write_port_map_16bit_le() for more detail.
 *
 * \pre     is_even(buffer_octet_offset)
 *
 */
#define mmu_write_port_map_16bit_le(buffer_handle, offset)   \
                            (uint16 *)(PxD_VM_BUFFER_LOWER + \
                             (mmu_handle_index(buffer_handle) << \
                              MMU_WINDOW_ADDR_BITS_PER_BUFFER) + offset)

/**
 * Configure the MMU Processor Remote Buffer Write Port to map the buffer
 * region read-only in 16 bit mode. Note that only one remote buffer may
 * be mapped for writing at any time. If used from interrupt the write remote
 * config register must be saved and restored around its use so background
 * use isn't disturbed.
 *
 * \sa      mmu_write_port_map_16bit_le() for more detail.
 *
 * \pre     is_even(buffer_octet_offset)
 *
 */
#define mmu_write_port_map_remote_16bit(buffer_handle, offset)   \
    (hal_set_reg_vm_request_write_remote_config(buffer_handle), \
    MMU_REMOTE_WRITE_PORT_MAP_WORKAROUND                        \
    (uint16 *)(PxD_VM_BUFFER_LOWER + (offset)))

/**
 * Note: 16 bit Big-Endian port mapping mode is not supported by the hardware
 */

/**
 * Macros to map either a remote or local buffer for reading/writing
 */
#define mmu_read_port_map_remote_or_local_8bit(h,o)     \
        (mmu_handle_is_local(h) ? mmu_read_port_map_8bit((h),(o)) : \
                                 mmu_read_port_map_remote_8bit((h),(o)))
#define mmu_read_port_map_remote_or_local_16bit_le(h,o)     \
        (mmu_handle_is_local(h) ? mmu_read_port_map_16bit_le((h),(o)) : \
                                 mmu_read_port_map_remote_16bit((h),(o)))
#define mmu_write_port_map_remote_or_local_8bit(h,o)     \
        (mmu_handle_is_local(h) ? mmu_write_port_map_8bit((h),(o)) : \
                                 mmu_write_port_map_remote_8bit((h),(o)))
#define mmu_write_port_map_remote_or_local_16bit_le(h,o)     \
        (mmu_handle_is_local(h) ? mmu_write_port_map_16bit_le((h),(o)) : \
                                 mmu_write_port_map_remote_16bit((h),(o)))

/**
 * Saves the current state of MMU Processor Write Port and leaves it in closed
 * state available for re-opening.
 *
 * \sa      mmu_read_port_save() for more detail.
 *
 * \sa      mmu_write_port_restore()
 *
 * \post    mmu_write_port_is_closed()
 *
 */
#define mmu_write_port_save(saved_state)    ((saved_state)->buffer_handle = \
                    (mmu_handle)hal_get_reg_vm_request_write_remote_config())

/**
 * Restores the state of MMU Processor Write Port for previously saved state.
 *
 * \sa      mmu_read_port_restore() for more detail.
 * \sa      mmu_write_port_save()
 *
 * \post    mmu_write_port_is_closed()
 *
 */
#define mmu_write_port_restore(saved_state)     \
        hal_set_reg_vm_request_write_remote_config((saved_state)->buffer_handle)

#endif /* DESKTOP_TEST_BUILD && !COV_TEST_BUILD */

#endif  /* MMU_PROC_PORT_H */
