// ***********************************************************************
// * Copyright 2016-2018 Qualcomm Technologies International, Ltd.
// ***********************************************************************


// *************************************************************************************************
// NOTE ON MEMORY MANAGEMENT
//
// When a function returns a non-NULL ::ka_err pointer, it is the caller's responsibility to call
// ka_free_error() to release the memory associated with it. Calling a function and
// ignoring its error is a potential memory leak.
//
// *************************************************************************************************

#ifndef INCLUDED_KALACCESS_HIGHLEVEL_H
#define INCLUDED_KALACCESS_HIGHLEVEL_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif


/// Execution states of the Kalimba exposed by this API.
typedef enum
{
    /// Processor has hit a PM breakpoint. Removing the breakpoint will allow the processor to run;
    /// to keep the core at the breakpointed location after breakpoint removal, first pause it via 
    /// ka_pause().
    KA_STATE_PM_BREAK,
    /// Unlike PM breakpoints, removing a DM breakpoint does not cause the core to run again. Call 
    /// ka_run() to resume execution.
    KA_STATE_DM_BREAK,
    /// The processor is running (it has been told to run and nothing is stopping it from doing so).
    KA_STATE_RUNNING,
    /// Processor is paused -- distinct from sitting at a breakpointed location.
    KA_STATE_STOPPED,
    /// If memory exceptions have been enabled, and configured to break execution when raised, this
    /// state will be set when an exception is occurs.
    KA_STATE_EXCEPTION_BREAK,
    /// Processor has hit a breakpoint triggered by external source, e.g. another subsystem.
    KA_STATE_EXTERNAL_BREAK,
    /// The processor, or subsystem within which the processor lives, is not powered or clocked.
    KA_STATE_CLOCK_OR_POWER_OFF,
    /// Processor is shallow sleeping. Only supported for csra68100 and later chips.
    KA_STATE_SHALLOW_SLEEP,
} ka_chip_state;

/// \defgroup HighLevelMem High level memory access API
/// @{


KALACCESS_API ka_err *ka_read_pm(kalaccess *ka, unsigned addr, uint32_t *result);

/// Writing to PM will only work for regions that are backed by RAM and which have suitable access
/// control. Writes to other areas, e.g. mapped flash, or access-controlled PM RAM, may or may not 
/// return an error depending on the transport and the firmware configuration, but will not take
/// effect.
KALACCESS_API ka_err *ka_write_pm(kalaccess *ka, unsigned addr, uint32_t value);

KALACCESS_API ka_err *ka_read_pm_block(kalaccess *ka, unsigned start_addr, uint32_t *data, unsigned num_words);

KALACCESS_API ka_err *ka_write_pm_block(kalaccess *ka, unsigned start_addr, uint32_t *data, unsigned num_words);

KALACCESS_API ka_err *ka_read_dm(kalaccess *ka, unsigned addr, uint32_t *result);

KALACCESS_API ka_err *ka_write_dm(kalaccess *ka, unsigned addr, uint32_t val);

KALACCESS_API ka_err *ka_read_dm_block(kalaccess *ka, unsigned start_addr, uint32_t *data, unsigned num_words);

KALACCESS_API ka_err *ka_write_dm_block(kalaccess *ka, unsigned start_addr, uint32_t *data, unsigned num_words);

KALACCESS_API ka_err *ka_read_dm_block8(kalaccess *ka, unsigned start_addr, uint8_t *data, unsigned num_octets);

KALACCESS_API ka_err *ka_read_pm_block8(kalaccess *ka, unsigned start_addr, uint8_t *data, unsigned num_octets);

KALACCESS_API ka_err *ka_write_dm_block8(kalaccess *ka, unsigned start_addr, uint8_t *data, unsigned num_octets);

KALACCESS_API ka_err *ka_write_pm_block8(kalaccess *ka, unsigned start_addr, uint8_t *data, unsigned num_octets);

KALACCESS_API ka_err *ka_read_dm_block16(kalaccess *ka, unsigned start_addr, uint16_t *data, unsigned num_elements);

KALACCESS_API ka_err *ka_read_pm_block16(kalaccess *ka, unsigned start_addr, uint16_t *data, unsigned num_elements);

KALACCESS_API ka_err *ka_write_dm_block16(kalaccess *ka, unsigned start_addr, uint16_t *data, unsigned num_elements);

KALACCESS_API ka_err *ka_write_pm_block16(kalaccess *ka, unsigned start_addr, uint16_t *data, unsigned num_elements);

/// @}

/// \defgroup HighLevelRegs High level register access API
/// @{

KALACCESS_API ka_err *ka_read_register(kalaccess *ka, ka_register_id reg, uint32_t *result);
KALACCESS_API ka_err *ka_write_register(kalaccess *ka, ka_register_id reg, uint32_t val);
/// Attempt to read the specified set of registers in an efficient manner. Will arrange for block
/// reads where possible, reducing latency.
KALACCESS_API ka_err *ka_read_register_set(kalaccess *ka, ka_register_id const *reg_ids, uint32_t *data, int const num_registers);
KALACCESS_API ka_err *ka_write_register_set(kalaccess *ka, ka_register_id const *reg_ids, uint32_t const *data, int const num_registers);

/// @}

/// \defgroup HighLevelExec High level execution control API
/// @{

KALACCESS_API ka_err *ka_get_instruction_set_from_pc(kalaccess *ka, ka_instruction_set *insn_set);

/// \brief Run the processor.
/// Use of this function is incompatible with an in-progress ka_run_to() or ka_step_over() operation.
/// Those operations should be completed via ka_pause() or cancelled via 
/// ka_abandon_run_control_operation() before ka_run() is used again.
/// The ka_instruction_set parameter 'insn_set' is no longer consumed by ka_run. Callers may safely pass any value.
KALACCESS_API ka_err *ka_run(kalaccess *ka, ka_instruction_set /*insn_set DEPRECATED, IGNORED*/, bool *run_called);

/// \brief Run to the specified address.
/// This function inserts a breakpoint at the desired address and sets the core running. When the
/// breakpoins is hit, ka_pause() should be called to remove the temporary breakpoint used for this
/// operation. See also ka_step_over().
KALACCESS_API ka_err *ka_run_to(kalaccess *ka, unsigned addr, ka_instruction_set insn_set);

/// Pause the processor.
/// Use this to either halt execution, or to stop the processor carrying on when a PM breakpoint is
/// removed; see ::KA_STATE_PM_BREAK.
KALACCESS_API ka_err *ka_pause(kalaccess *ka);

/// Step a single instruction, including any prefixes. Steps "in" to calls.
/// \param[in] insn_set supply the instruction set of the code at the current execution location.
/// If not known, use ka_get_instruction_set_from_pc().
KALACCESS_API ka_err *ka_step(kalaccess *ka, ka_instruction_set insn_set);

/// \brief Step over an instruction, stepping over calls.
/// Step over an instruction, stepping over calls. This function may therefore insert a breakpoint
/// at the instruction after the call and set the core running. In this case, the step-over will
/// complete when the breakpoint is hit. When this occurs, ka_pause() should be called to 
/// remove the temporary breakpoint used for this operation.
///
/// If there is no call to step over, the step may be achieved without this measure, so the
/// operation will be completed upon return. These cases may be distinguished by the value of
/// \p run_called upon return.
///
/// Note that if the core is set running, it may hit a previously-set breakpoint before the
/// step-over target is reached. In this case, the operation can be continued by running the
/// processor again, or abandoned by calling ka_abandon_run_control_operation().
///
/// \param[in] ka a kalaccess session.
/// \param[out] run_called upon return, will be \c false is the step completed synchronously, or
/// \c true if it will complete later (see above).
/// \param[in] insn_set the instruction set of the code at the current PC.
KALACCESS_API ka_err *ka_step_over(kalaccess *ka, bool *run_called, ka_instruction_set insn_set);

/// Determine if the processor has hit a temporary run control breakpoint installed by kalaccess
/// for ka_step_over() or ka_run_to().
/// The processor should be paused when this function is called.
KALACCESS_API ka_err *ka_was_run_control_break_hit(
    kalaccess *ka,
    uint32_t current_location,
    bool *result);

/// \brief Abandons an in-progress ka_run_to() or ka_step_over() operation.
/// This removes any temporary breakpoint which was installed for these functions. It does nothing
/// if there is no breakpoint to remove.
/// Note that ka_pause() also removes this breakpoint, if it is set, but unlike ka_pause(), this
/// function does not alter the processor's run state (obtainable via ka_get_chip_state()).
KALACCESS_API ka_err *ka_abandon_run_control_operation(kalaccess *ka);


/// Get the processor's execution state.
/// \see ::ka_chip_state.
KALACCESS_API ka_err *ka_get_chip_state(kalaccess *ka, ka_chip_state *result);


/// @}

/// \defgroup HighLevelBps High level breakpoint API
/// @{

/// \brief This function removes ALL hardware breakpoints, irrespective of who set them, and enables
/// break instructions. This function does not need to be called before using the other breakpoint
/// functions in this API; it is intended as an optional way to get a "clean slate".
/// \param[in] ka an active kalaccess session
KALACCESS_API ka_err *ka_init_bp_system(kalaccess *ka);

typedef struct
{
    unsigned actual_addr;            ///< the address the BP is installed at on the hardware.
    bool     set_by_calling_session; ///< whether this BP was set by the kalaccess session which 
                                     ///< made the call to ka_list_pm_breaks
} ka_pm_breakpoint;

/// \brief This function retrieves the currently installed hardware PM breakpoints on the
/// device. This includes those not set by the current (or indeed any) kalaccess client session, but 
/// excludes any breakpoints reserved by kalaccess for implementing run-control operations
/// (e.g. step over).
/// The address information is thus in terms of actual BP install addresses, rather than requested
/// addresses, as there is no way of knowing the latter, for BPs not installed by this session.
/// \param[in] ka an active kalaccess session
/// \param[out] installed_bps receives an array of ka_pm_breakpoint structures, with information about
/// installed breakpoints.
/// This list should be freed using ka_free_pm_break_list.
/// \param[out] num_installed_bps received the number of elements in the array installed_bps
KALACCESS_API ka_err *ka_list_pm_breaks(kalaccess *ka, ka_pm_breakpoint **installed_bps, int *num_installed_bps);

/// \brief Free the list returned by ka_list_pm_breaks
KALACCESS_API void ka_free_pm_break_list(ka_pm_breakpoint const *installed_bps);

/// \brief Returns whether the given program address is a valid breakpoint location.
/// \param[in] insn_set the instruction set of the code present at addr
/// set at the address should be supplied.
KALACCESS_API ka_err *ka_is_valid_pm_break_location(kalaccess *ka, unsigned addr, bool *result, ka_instruction_set insn_set);

/// \brief Set a PM breakpoint. See also ka_set_pm_break_ret_actual_addr.
/// \param[in] addr the address the caller would like to set the breakpoint
/// \param[in] insn_set the instruction set of the code present at addr
KALACCESS_API ka_err *ka_set_pm_break(kalaccess *ka, unsigned addr, ka_instruction_set insn_set);

/// \brief Set a PM breakpoint. This function behaves exactly like ka_set_pm_break, but also returns
/// the actual address at which the breakpoint was installed.
/// \param[in] addr the address the caller would like to set the breakpoint
/// \param[in] insn_set the instruction set of the code present at addr
/// \param[out] actual_addr receives the address at which the breakpoint was actually installed. This
/// be set when this function returns with no error, or with KA_ADDRESS_ALREADY_CONTAINS_BREAK.
KALACCESS_API ka_err *ka_set_pm_break_ret_actual_addr(kalaccess *ka, unsigned addr, ka_instruction_set insn_set, 
                                                      unsigned *actual_addr);

/// \brief Clear a PM breakpoint set by ka_set_pm_break or ka_set_pm_break_ret_actual_addr.
/// \param[in] addr the address requested when ka_set_pm_break or ka_set_pm_break_ret_actual_addr was called.
KALACCESS_API ka_err *ka_clear_pm_break(kalaccess *ka, unsigned addr);

/// \brief This function removes ALL hardware PM breakpoints, irrespective of who set them.
KALACCESS_API ka_err *ka_clear_all_pm_breaks(kalaccess *ka);

/// \brief Set a data breakpoint.
/// \param[in] start_addr the first address for the data breakpoint to trigger on.
/// \param[in] end_addr the last address for the data breakpoint to trigger on (inclusive), so the
/// range is [start_addr, end_addr].
/// \param[in] on_read true if the breakpoint should trigger on read accesses to [start_addr, end_addr]
/// \param[in] on_read true if the breakpoint should trigger on write accesses to [start_addr, end_addr]
KALACCESS_API ka_err *ka_set_dm_break(kalaccess *ka, unsigned start_addr, unsigned end_addr, bool on_read, bool on_write);

/// \brief Clear a data breakpoint set via ka_set_dm_break. The parameters must take the same values
/// that were passed to ka_set_dm_break.
KALACCESS_API ka_err *ka_clear_dm_break(kalaccess *ka, unsigned start_addr, unsigned end_addr, bool on_read, bool on_write);

/// \brief This function removes ALL hardware DM breakpoints, irrespective of who set them.
KALACCESS_API ka_err *ka_clear_all_dm_breaks(kalaccess *ka);

typedef struct
{
    unsigned start_addr;
    unsigned last_addr;
    bool     trigger_on_read;
    bool     trigger_on_write;
    bool     set_by_calling_session; ///< whether this BP was set by the kalaccess session which 
                                     ///< made the call to ka_list_dm_breaks
} ka_dm_breakpoint;

/// \brief This function retrieves the currently set / installed hardware DM breakpoints on the
/// device. This includes those not set by the current (or indeed any) kalaccess session.
/// \param[in] ka an active kalaccess session
/// \param[out] installed_bps receives an array of ka_dm_breakpoint structures, with information about
/// installed breakpoints.
/// This list should be freed using ka_free_dm_break_list.
/// \param[out] num_installed_bps received the number of elements in the array installed_bps
KALACCESS_API ka_err *ka_list_dm_breaks(kalaccess *ka, ka_dm_breakpoint **installed_bps, int *num_installed_bps);

/// \brief Free the list returned by ka_list_dm_breaks
KALACCESS_API void ka_free_dm_break_list(ka_dm_breakpoint const *installed_bps);

/// @}

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_KALACCESS_HIGHLEVEL_H
