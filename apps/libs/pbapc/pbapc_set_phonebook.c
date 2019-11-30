/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    pbapc_set_phonebook.c
    
DESCRIPTION
   phone book state machine

*/

#include <stdlib.h>
#include "pbapc_extern.h"
#include "pbapc_private.h"
#include "pbapc_util.h"

 /***************************************************************************
 * NAME    
 *  pbapcPbNextState
 *
 * DESCRIPTION
 *  Kick off the Phone book state machine.
 ***************************************************************************/
pbapcSetPhonebookState pbapcPbNextState( pbapcPb pb )
{
    pbapcSetPhonebookState pbState;

    if( pb.targetRepos == pb.currRepos )
    {
        /* Target and current repositories the same */
        if( pb.currPb == pbap_root )
        {
            pbState = pbapc_spb_goto_telecom;
        }
        else
        {
            pbState = ( pb.currPb == pbap_telecom)?
                        pbapc_spb_goto_phonebook:
                        pbapc_spb_goto_parent;
        }
    }
    else if ( pb.currRepos != pbap_any  )
    {
        /* Target and current repositories are not same */
        pbState = ( ( pb.targetRepos == pbap_sim1 ) &&
                    ( pb.currPb == pbap_root) )?
                    pbapc_spb_goto_sim1:
                    pbapc_spb_goto_root;
    }

    else
    {
         /* unknown repository */
          pbState = pbapc_spb_goto_root;
    }

    return pbState;
}

/***************************************************************************
 * NAME    
 *  pbapcSetPb
 *
 * DESCRIPTION
 * 
 ***************************************************************************/
void pbapcSetPb(pbapcState *state)
{
    const uint8 *folder = NULL;
    uint16 len = 0;
    ObexFolderPath flag= obex_folder_in;
    
    switch (state->setPbState)
    {
    case pbapc_spb_goto_root:
        flag = obex_folder_root;
        break;
    case pbapc_spb_goto_parent:
        flag = obex_folder_out;
        break;
    case pbapc_spb_goto_sim1:
        folder = pbapcGetSimName( &len );
        break;
    case pbapc_spb_goto_telecom:
        folder = pbapcGetPbNameFromID(pbap_telecom, &len);
        break;
    case pbapc_spb_goto_phonebook:
        folder = pbapcGetPbNameFromID(state->pb.targetPb, &len);
        break;
    default:
        PBAPC_DEBUG(("Invalid SetPhonebook State\n"));
        break;
    }

    ObexSetPathRequest( state->handle, flag, len, folder );    
}


 /***************************************************************************
 * NAME    
 *  pbapcPbCompleteState
 *
 * DESCRIPTION
 *  Set the current phonebook after completing the setpath
 ***************************************************************************/
bool pbapcPbCompleteState( pbapcState *state )
{
    bool ret = FALSE;
    pbapcPb pb = state->pb;
    
    switch (state->setPbState)
    {
    case pbapc_spb_goto_root:
        pb.currRepos = pbap_local;
        pb.currPb  = pbap_root;
        break;
    case pbapc_spb_goto_parent:
        pb.currPb =  pbap_telecom;
        break;
    case pbapc_spb_goto_sim1:
        pb.currRepos = pbap_sim1;
        pb.currPb  = pbap_root;
        break;
    case pbapc_spb_goto_telecom:
        pb.currPb = pbap_telecom;
        pb.currRepos = pb.targetRepos;
        break;
    case pbapc_spb_goto_phonebook:
        pb.currPb = pb.targetPb;
        pb.currRepos = pb.targetRepos;
        ret = TRUE;
        break;
    default:
        PBAPC_DEBUG(("PBAC - Set Phonebook - invalid complete state\n"));
        break;
    }
    
    state->pb = pb;
    return ret;
}

