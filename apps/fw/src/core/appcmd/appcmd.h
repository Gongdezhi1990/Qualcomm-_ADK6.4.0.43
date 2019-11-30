/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Curator APPCMD command interpreter
 *
 */
#ifndef APPCMD_H
#define APPCMD_H

/*
 * Definitions visible to the other end of the link are in
 * appcmd.xml in the Hydra interface directory.
 */
#include "gen/k32/appcmd_prim.h"
#include "buffer/buffer.h"

/*****************************************************************************
 * Declarations
 ****************************************************************************/

extern volatile uint32 appcmd_cmd;
extern uint32 appcmd_rsp;
extern uint32 appcmd_parameters[];
extern uint32 appcmd_results[];
extern BUFFER_MSG *appcmd_rsp_buf;

extern const uint32 appcmd_protocol_version;

/*****************************************************************************
 * Dependencies.
 ****************************************************************************/

/*****************************************************************************
 * Functions
 ****************************************************************************/

/**
 * Initialise the APPCMD Implementation
 *
 */
extern void appcmd_init(void);

/**
 * Enable or disable interrupts for appcmd events
 *
 */
extern void appcmd_enable_interrupts(bool enable);

/**
 * Handler function type for test commands. Used by modules
 * to install handlers for test commands.
 *  /param command The command that the host issued
 *  /param params The parameters supplied with the command
 *  /param result An array to hold the result of the command
 *   which is filled by the handler function.
 *   /returns The response code e.g. APPCMD_RESPONSE_SUCCESS
 */
typedef APPCMD_RESPONSE appcmd_test_handler_fn(APPCMD_TEST_ID command,
                                               uint32 * params,
                                               uint32 * result);
/**
 * Add a handler for a test.
 *  /param command The command that the handler handles
 *  /param fn The handler function to be called
 *  /returns Whether the handler was successfully added.
 */
extern bool appcmd_add_test_handler(APPCMD_TEST_ID command,
                                    appcmd_test_handler_fn * fn);

/**
 * API to allow tests to send response packets at any time.  Appcmd simply
 * pushes them into a buffer which the host should poll regularly.
 * @param command The test command ID
 * @param rsp     An APPCMD_RESPONSE value
 * @param results An array of results, equivalent to appcmd_results
 * (can be NULL if \p num_results is 0)
 * @param num_results The number of results words in the array
 * @param remote_packet Flag indicating that the packet arrived from the other
 * processor
 */
extern void appcmd_add_test_rsp_packet_source(uint32 command /* test ID */,
                                              uint32 rsp,
                                              const uint32 *results,
                                              uint16f num_results,
                                              bool remote_packet);

/**
 * Add a response packet that was created locally (used by normal clients)
 */
#define appcmd_add_test_rsp_packet(command, rsp, results, num_results) \
   appcmd_add_test_rsp_packet_source(command, rsp, results, num_results, FALSE)

/**
 * Add a response packet that was created remotely (used by IPC only)
 */
#define appcmd_add_remote_test_rsp_packet(command, rsp, results, num_results) \
   appcmd_add_test_rsp_packet_source(command, rsp, results, num_results, TRUE)


#if defined(DESKTOP_TEST_BUILD)
extern void appcmd_background_handler(void);
#endif

/**
 * Empty function that simply provides a symbol that the callback-from-Python
 * mechanism can use as the nominal firmware callback
 */
extern void appcmd_test_cb(void);

#endif /* APPCMD_H */
