/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
 *
 *
 *************************************************************************/
/*
 * \file boot.c
 * \brief Startup code
 * MODULE : Boot
 *
 */

/****************************************************************************
Include Files
*/
#if defined(INSTALL_HTOL) || defined(RUNNING_ON_KALSIM)
#include "audio.h"
#endif /* INSTALL_HTOL || RUNNING_ON_KALSIM */
#include "sched_oxygen/sched_oxygen.h"
#include "pmalloc/pl_malloc.h"
#include "platform/pl_error.h"
#include "pl_timers/pl_timers.h"
#include "platform/profiler_c.h"
#include "hal.h"
#include "id/id.h"
#include "capability_database.h"
#ifdef INSTALL_HTOL
#include "htol.h"
#endif
#include "fault/fault.h"
#ifdef INSTALL_PIO_SUPPORT
#include "pio.h"
#endif
#include "platform/pl_interrupt.h"
#ifdef INSTALL_HYDRA
#ifndef RUNNING_ON_KALSIM
#include "submsg/submsg.h"
#include "hydra_sssm/sssm_ss.h"
#else
#ifdef FIRMWARE_TALKING_TO_KALCMD
#include "kalsim_msgif.h"
#endif /* FIRMWARE_TALKING_TO_KALCMD */
#endif /* RUNNING_ON_KALSIM */
#include "hydra_mmu_buff.h"
#include "mib/mib.h"
#endif /* INSTALL_HYDRA */
#ifdef CHIP_BASE_NAPIER
#include "hydra_mmu_buff.h"
#include "patch_loader.h"
#include "audio.h"
#include "comms_interface.h"
extern dsp_shared_window_t comms_shared_window;
#endif /* CHIP_BASE_NAPIER */
#ifdef CHIP_BASE_BC7
#include "bc_comms.h"
#include "kalimba_messages.h"
#endif /* CHIP_BASE_BC7 */
#ifdef CHIP_BASE_A7DA_KAS
#include "kas_dma.h"
#include "kas_audio_endpoint.h"
#include "kas_ipc.h"
#endif /* CHIP_BASE_A7DA_KAS */
#ifdef INSTALL_CAP_DOWNLOAD_MGR
#include "malloc_pm/malloc_pm.h"
#include "cap_download_mgr.h"
#endif
#ifdef INSTALL_ROM_TEST
#include "rom_test.h"
#endif
#ifdef INSTALL_FILE_MGR
#include "file_mgr.h"
#endif /* INSTALL_FILE_MGR */

#ifdef INSTALL_AUDIO_DATA_SERVICE
#include "audio_data_service.h"
#endif

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
#include "kip_mgr/kip_mgr.h"
#endif

#ifndef NONSECURE_PROCESSING
#include "../../lib_private/security/security_library_c.h"
#endif /* NONSECURE_PROCESSING */
#include "sys_events.h"
#include "preserved/preserved.h"

#ifdef INSTALL_AOV
#include "aov.h"
#endif

/****************************************************************************
Private Macro Declarations
*/
/* The HTOL rig has to set TEST_REG_0 to this value in order to put us into
 * HTOL mode. */
#define DO_HTOL_SIGNATURE 0x13579B
/****************************************************************************
Extern declarations
*/
/* This shouldn't really be global so isn't in the public header */
extern uint24 slt_fingerprint;
extern char PM_RAM_PATCH_CODE_START_HEADER;

#ifdef AUDIO_SECOND_CORE
/**
 * \brief sub main function for second cores
 *        hal_init() is done common at main() and rest will be
 *        initialised here.
 *
 */
static int cpux_main(void)
{
    /* P1 does not care about the reservedSize argument
     * Patching does not happen on P1, therefore the call
     * to init_malloc_pm ought to happen from boot.c (whether
     * or not INSTALL_PM_HEAP_SHARE is defined)
     */
#ifdef INSTALL_CAP_DOWNLOAD_MGR
    init_malloc_pm(0);
#endif

    /* initialise KiP mgr. The kip mgr would be
     * calling ipc_register_comms and registering the
     * processor with ipc.
     */
    kip_init();

    /* Enable error handlers for Kalimba libs.
     * These all end up in panic so probably not useful to
     * do this any earlier. */
    error_enable_exception_handlers(TRUE);

    /**
     * Initialise the fault system here because on P0 it is initialised by Hydra
     * SSSM which does not run other on P1.
     */
    init_fault();

#ifndef NONSECURE_PROCESSING
    /* On Px, all security requests will be forwarded to P0 */
    security_init();
#endif /* NONSECURE_PROCESSING */

    /**
     * Profiler initialisation creates a scheduled_event to update the profiler
     * every sec.
     */
     PROFILER_INIT();

    /** Run Scheduler */
    sched(RUNLEVEL_BOOT);

    return 0;

}
#endif /* AUDIO_SECOND_CORE */


/**
 * \brief  Startup entry function, which initialises & runs platform scheduler
 *
 * \return Ideally scheduler should run for ever(!).
 *
 * Initialises IRQ and scheduler and runs the scheduler.
 */
int main(void)
{
    /* Common hardware initialisation */
    hal_init();

    /* Initialise IRQs and the scheduler */
    interrupt_initialise();

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
    /* 
    *  This must be called before pmalloc_init to
    *  initialise the IPC LUT. The pmalloc uses LUT
    *  to share heap information between processors
    */
    ipc_init();
#endif

    /*initialise malloc*/
    init_pmalloc();

    /* Init scheduler */
    init_sched();
    init_pl_timers();
    init_preserved();

#ifdef INSTALL_CAP_DOWNLOAD_MGR
#if !defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)
    #error "Capability download only supports dual-core on single image Kymera builds"
#endif /* !defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE) */
    cap_download_mgr_init();
    capability_database_init_download_list();
#endif /* INSTALL_CAP_DOWNLOAD_MGR */

#ifdef AUDIO_SECOND_CORE
    if( hal_get_reg_processor_id() != 0 )
    {
        return cpux_main();
    }
#endif /* AUDIO_SECOND_CORE */
    /* All code below executes only on the primary */
#if !defined(INSTALL_PM_HEAP_SHARE) && defined(INSTALL_CAP_DOWNLOAD_MGR)
    /* All PM can be used by the PM heap */
    init_malloc_pm(0);
#endif

#ifdef INSTALL_PIO_SUPPORT
    /* Initialise PIO system - done early incase the
       rest of the boot depends on pio state */
    init_pio();
#endif

#if  defined(CHIP_BASE_NAPIER)
	/*
	* Check for patches pre-loaded on PRAM
	* This function must be called prior to pram alloc
	*/
	uint32 patchStartAddress = comms_shared_window.patch_start;
	if (patchStartAddress != 0){
		initialize_patches(patchStartAddress); // (unsigned)(&PM_RAM_PATCH_CODE_START_HEADER) from linker script
	}
#endif

#ifndef NONSECURE_PROCESSING
    security_init();
#endif /* NONSECURE_PROCESSING */


    init_system_event();

#if defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)
    /* Initialise the MMU hardware */
    mmu_buffer_initialise();
#endif /* CHIP_BASE_HYDRA || CHIP_BASE_NAPIER*/

// for BC7 we need some XAP <-> DSP messaging in order to be able to handle any ports comms, use asm for now
#ifdef CHIP_BASE_BC7
   // initialise the message library
   bc_message_initialise();
#endif /* CHIP_BASE_BC7 */

#ifdef CHIP_BASE_A7DA_KAS
   /* initialise the DMA services */
   kas_dma_initialise();
   kas_audio_dma_initialise();
   /* Clear all ipc interrupts */
   kas_ipc_initialise();
#endif /* CHIP_BASE_A7DA_KAS */
#if  defined(CHIP_BASE_NAPIER)

#ifdef INSTALL_DUAL_CORE_SUPPORT
    kip_init();
#endif /* INSTALL_DUAL_CORE_SUPPORT */

    /* configure pmalloc */
    config_pmalloc();

    /*
     * Initialise the audio subsystem
     * (on hydra systems this is done by a service which we don't have on Napier)
     */
    init_audio();
#endif

#ifdef INSTALL_HTOL
    /* Should we do HTOL instead of running normally?
     * We figure this out by comparing TEST_REG_0 with the HTOL signature.
     * The register is zeroed (by hardware) when the CPU is reset, so in order
     * for this test to pass, we must have had some external influence.
     * TODO: In future, we will start HTOL under direct instruction from the
     * Curator (probably via a service request); this mechanism is only temporary. */
    if (hal_get_reg_test_reg_0() == DO_HTOL_SIGNATURE)
    {
        /* We need to initialise the audio hardware here
         * In a normal boot this happens later, after any config file download
         * but we don't get that far if doing HTOL
         */
        init_audio();
        hal_set_reg_test_reg_0(0);
        put_message(HTOL_QUEUE_ID, MSG_HTOL_START, NULL);
    }
    else
#endif /* INSTALL_HTOL */
    {
#ifdef INSTALL_HYDRA

#ifdef RUNNING_ON_KALSIM
        /* Initialise the MIB. */
        mib_start_configuring();
        mibinitialised();
        /*
         * Initialise the audio subsystem
         */
        init_audio();
#ifdef FIRMWARE_TALKING_TO_KALCMD
        kalcmd_configure_communication();
#endif /* FIRMWARE_TALKING_TO_KALCMD */
#else
        /* Normal Hydra subsystem initialisation */
        submsg_init();

        /* This just queues a message to start the SSSM process off.
         * Nothing substantial happens (e.g., patch download) until the
         * scheduler actually starts running (sched() below). */
        sssm_init();
#endif /* RUNNING_ON_KALSIM */
#ifdef INSTALL_AUDIO_DATA_SERVICE
        audio_data_service_init();
#endif
#endif /* INSTALL_HYDRA */
    }

#ifdef INSTALL_FILE_MGR
    file_mgr_init();
#endif

#ifdef INSTALL_AOV
    aov_init();
#endif

#ifndef TODO_CRESCENDO_CHIPVER_CHECKING
    /* Check that we're running on the hardware this code was built for
     * We don't bother doing this on ROM builds - we really should get it right
     * and if not there's not much we can do about it once it's in a ROM.
     */
#ifdef INSTALL_LPC_SUPPORT
    {
        int minor_version = hal_get_chip_version_minor();

        if (minor_version != CHIP_BUILD_VER)
        {
            /* For builds targetting "latest" hardware, assume it's OK
             * and don't fault if the version is later than expected
             * (if it isn't OK, there should be a new "latest" build).
             */
            if ((CHIP_BUILD_VER != CHIP_BUILD_VER_LATEST) ||
                (minor_version < CHIP_BUILD_VER))
            {
                fault_diatribe(FAULT_AUDIO_WRONG_HARDWARE_VERSION,
                               hal_get_reg_sub_sys_chip_version());
            }
        }
    }
#endif /* INSTALL_LPC_SUPPORT */
#endif /* TODO_CRESCENDO_CHIPVER_CHECKING */

    /* Enable error handlers for Kalimba libs.
     * These all end up in panic so probably not useful to do this any earlier. */
    error_enable_exception_handlers(TRUE);

    /* Profiler initialisation creates a scheduled_event to update the profiler every sec. */
    PROFILER_INIT();

    /* Run scheduler. We're still not ready until the SSSM has finished, though.
     * Any init code that needs to communicate with other subsystems should be
     * called from sssm_init_operational(). */
    sched(RUNLEVEL_BOOT);

    /* Scheduler isn't expected to return... */

#ifdef INSTALL_ROM_TEST
    /* Shouldn't get here, but we need to reference the ROM performance
     * test code to get it linked in */
    rom_performance_test();
#endif

    /* TODO: This is a shortcut to make sure we reference the SLT.*/
    return(slt_fingerprint);

}
