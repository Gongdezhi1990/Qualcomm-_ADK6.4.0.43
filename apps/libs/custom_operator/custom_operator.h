/****************************************************************************
Copyright (c) 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    custom_operator.h

DESCRIPTION
    The purpose of this library is to manage the creation and destruction of operators.
    It sits on top of the operators library and manages any DSP capability bundle file load/unload required.
    Additionally, it acts as a medium to override capability IDs at a low level, by specifying that you want
    some capability ID to be replaced with a different one before creating an operator for it.
*/

#ifndef LIBS_CUSTOM_OPERATOR_H_
#define LIBS_CUSTOM_OPERATOR_H_

#include "operators.h"

/*!
 * The processors on which a downloadable capabilities bundle is available
 */
typedef enum
{
    capability_bundle_available_p0 = 0,
    capability_bundle_available_p0_and_p1 = 1,
    FORCE_ENUM_TO_MIN_16BIT(capability_bundle_processor_availability_t)
} capability_bundle_processor_availability_t;

/*!
 * @brief Create an operator and send a sequence of configuration messages as defined by the setup parameter.
 *        The capability ID will be overridden if that has been specified using CustomOperatorsReplaceCapabilityId().
 *        DSP capability bundle files will be loaded as required.
 *
 * @param cap_id The hard-coded DSP capability ID that can be overridden.
 * @param processor_id The processor for which to create the operator.
 * @param priority The operators priority.
 * @param setup Items to be configured when operator is created. Can be NULL if no configuration is required.
 *
 * @return Operator created.
 */
Operator CustomOperatorCreate(capability_id_t cap_id, operator_processor_id_t processor_id, operator_priority_t priority, const operator_setup_t* setup);

/*!
 * @brief Destroys all the operators passed as input and unloads DSP capability bundle files as required.
 *        The operators must be stopped before they can be destroyed.
 *
 * @param operators Pointer to an array with the operators to be destroyed.
 * @param number_of_operators The length of operators array.
 */
void CustomOperatorDestroy(Operator *operators, unsigned number_of_operators);

/*!
 * @brief Sets a new capability ID to replace the original/hard-coded capability ID before creating an operator for it.
 *
 * @param original_cap_id The capability ID we want to replace.
 * @param new_cap_id The new capability ID we want to replace the original one.
 *
 * @return Returns TRUE for success and FALSE for error.
 */
bool CustomOperatorReplaceCapabilityId(capability_id_t original_cap_id, capability_id_t new_cap_id);

/*!
 * @brief Sets the processor ID to replace the default processor for a given original capability ID.
 *
 * @param cap_id The capability ID with which we will be associating the processor ID replacement.
 * @param new_proc_id The processor ID we want to use instead of the default.
 *
 * @return void
 */
void CustomOperatorReplaceProcessorId(capability_id_t cap_id, operator_processor_id_t replacement_proc_id);


#ifdef HOSTED_TEST_ENVIRONMENT
/* Used to reset the state of the lists in custom_operator_bundle_files.c */
void CustomOperatorBundleFilesTestReset(void);
/* Used to reset the state of the lists in custom_operator_capability_id_translator.c */
void CustomOperatorCapabilityIdTranslatorTestReset(void);
/* Used to reset the array of processor Id mappings in custom_operator_processor_id_translator.c */
void CustomOperatorProcessorIdTranslatorTestReset(void);
#endif

#endif /* LIBS_CUSTOM_OPERATOR_H_ */
