/*
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

*/
/**
 * \file
 * This is the public header file for the P1 main module.
 * 
 * * \mainpage
  * The Application Subsystem Processor 1 Software Documentation.
 * 
 * \defgroup core core
 * \section core_overview Overview
 * This is the component where all Qualcomm core modules live.
 * 
 * \defgroup customer customer
 * \section customer_overview Overview
 * This is the component where all customer subcomponents live.
 * 
 * \defgroup customer_core core
 * \ingroup customer
 * \section customer_core_overview Overview
 * This is the subcomponent where all customer core modules live.
 *
 * \defgroup vm_libs vm_libs
 * \ingroup customer
 * \section vm_libs_overview Overview
 * This is the subcomponent where all vm_lib modules live.
 *
 * \defgroup bluestack_prims bluestack_prims
 * \ingroup customer
 * \section bluestack_prims_overview Overview
 * This is the subcomponent where all bluestack prims live.
 * 
 * \defgroup includes includes
 * \ingroup core
 *
 * \defgroup hydra_includes hydra
 * \ingroup core
 *
 * \defgroup main main
 * \ingroup customer_core
 * 
 * \section main_intro INTRODUCTION
 * This subsystem provides a version of main() suitable for the Application
 * Subsystem P1. It mostly initialises other modules.
 * 
 */
/*!
 * \defgroup sink_app Sink Application
 * \ingroup sample_apps
 * \brief  Application for use in all devices that play audio or calls.
 * 
 * \section sink_app_into INTRODUCTION
 *      The sink application is a multi-purpose application that can be
 * configured for use in playing audio, allowing music streams carried 
 * over A2DP to be played, whilst providing control of the source device
 * using standard Bluetooth protocols for media control (AVRCP) and
 * call control (HFP).
 *
 * \defgroup sample_apps Sample Applications
 * \ingroup customer
 *      A number of sample application with the Audio 
 *  Development Kit (ADK). All the applications can be modified by
 *  customers.
 *
 *      The two types of application are
 *  - Single-Purpose examples
 *      None provided at present
 *  - Fully configurable applications
 *      -# \ref sink_app
 * 
 */

