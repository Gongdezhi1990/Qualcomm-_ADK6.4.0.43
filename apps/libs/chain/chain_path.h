/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.
*/

#ifndef CHAIN_PATH_H_
#define CHAIN_PATH_H_

#include "chain_list.h"

/****************************************************************************
DESCRIPTION
    Get the first valid input terminal in the stream with a role which matches
    input_role
*/
Sink chainPathGetInput(kymera_chain_t *chain, unsigned input_role);

/****************************************************************************
DESCRIPTION
    Get the first valid output terminal in the stream with a role which matches
    output_role
*/
Source chainPathGetOutput(kymera_chain_t *chain, unsigned output_role);

/****************************************************************************
DESCRIPTION
    Connect all streams in chain
*/
void chainPathConnect(kymera_chain_t *chain);

/****************************************************************************
DESCRIPTION
    Connect path specific streams in chain
*/
void chainPathConnectPath(kymera_chain_t *chain, unsigned path_role);

/****************************************************************************
DESCRIPTION
    Get the number of streams in the chain
*/
unsigned chainStreamGetNumberOfStreams(kymera_chain_t *chain);

/****************************************************************************
DESCRIPTION
    Get a role by stream number
*/
unsigned chainStreamGetRoleByStreamNumber(kymera_chain_t *chain, unsigned stream_number);

#endif /* CHAIN_PATH_H_ */
