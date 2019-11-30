/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/

#include "pmalloc/pl_malloc.h"
#include "op_channel_list.h"

#include "patch/patch.h"

#ifdef INSTALL_METADATA
bool op_channel_list_connect(OP_CHANNEL_LIST** list, tCbuffer *pterminal_buf, unsigned index, unsigned sink_flag, tCbuffer **metadata_buff)
#else
bool op_channel_list_connect(OP_CHANNEL_LIST** list, tCbuffer *pterminal_buf, unsigned index, unsigned sink_flag)
#endif /* INSTALL_METADATA */
{
    OP_CHANNEL_LIST *temp;
    
    patch_fn(op_channel_list_connect);    
    
    /* check valid parameters */
    if( (list == NULL) || (pterminal_buf == NULL) || (sink_flag > 1) )
    {
        return FALSE;
    }
    
    while(*list)
    {
        temp = *list;
        
        /* check if a list entry already exists for this channel index */
        if(temp->index == index)
        {
            if(temp->pterminal_buf[sink_flag] == NULL)
            {
#ifdef INSTALL_METADATA
                if(*metadata_buff == NULL)
                {
                    if (buff_has_metadata(pterminal_buf))
                    {
                        *metadata_buff = pterminal_buf;
                    }
                }
#endif /* INSTALL_METADATA */
                /* not connected yet, so connect the terminal */
                temp->pterminal_buf[sink_flag] = pterminal_buf;
                return TRUE;
            }
            else
            {
                /* can't connect an already connected terminal */
                return FALSE;
            }
        }
        
        list = &(temp->next);
    }
    
    /* reached end of list: create a new channel entry for this index */
    temp = (OP_CHANNEL_LIST*) xzpmalloc(sizeof(OP_CHANNEL_LIST));
    if(!temp)
    {
        return FALSE;
    }
#ifdef INSTALL_METADATA
    if(*metadata_buff == NULL)
    {
        if (buff_has_metadata(pterminal_buf))
        {
            *metadata_buff = pterminal_buf;
        }
    }
#endif /* INSTALL_METADATA */
    temp->pterminal_buf[sink_flag] = pterminal_buf;
    temp->index = index;
    *list = temp;
    
    return TRUE;
}
#ifdef INSTALL_METADATA
bool op_channel_list_disconnect(OP_CHANNEL_LIST** list, unsigned index, unsigned sink_flag, tCbuffer **metadata_buff)
#else
bool op_channel_list_disconnect(OP_CHANNEL_LIST** list, unsigned index, unsigned sink_flag)
#endif /* INSTALL_METADATA */
{
    OP_CHANNEL_LIST *temp;
#ifdef INSTALL_METADATA
    OP_CHANNEL_LIST *head = *list;
#endif /* INSTALL_METADATA */

    patch_fn(op_channel_list_disconnect);

    
    /* check valid parameters */
    if( (list == NULL) || (sink_flag > 1) )
    {
        return FALSE;
    }

    while(*list)
    {
        temp = *list;

        /* check if a list entry exists for this channel index */
        if(temp->index == index)
        {
            if(temp->pterminal_buf[sink_flag] != NULL)
            {
#ifdef INSTALL_METADATA
                if(*metadata_buff == temp->pterminal_buf[sink_flag])
                {
                    bool found_alternative = FALSE;
                    while (head != NULL)
                    {
                        if (head->index != index)
                        {
                            if (head->pterminal_buf[sink_flag] != NULL &&
                                    buff_has_metadata(head->pterminal_buf[sink_flag]))
                            {
                                *metadata_buff = head->pterminal_buf[sink_flag];
                                found_alternative = TRUE;
                                break;
                            }
                        }
                        head = head->next;
                    }
                    if (!found_alternative)
                    {
                        *metadata_buff = NULL;
                    }
                }
#endif /* INSTALL_METADATA */
                /* terminal has been connected, so disconnect it */
                temp->pterminal_buf[sink_flag] = NULL;
            }
            else
            {
                /* can't disconnect an already disconnected terminal */
                return FALSE;
            }
            
            /* check if both sink and source terminal are disconnected */
            if( (temp->pterminal_buf[OP_CHANNEL_LIST_SOURCE] == NULL) && 
                (temp->pterminal_buf[OP_CHANNEL_LIST_SINK] == NULL) )
            {
                /* if no connections remain, free the list entry */
                *list = temp->next;
                pfree(temp);
            }
            
            /* terminal disconnected successfully */
            return TRUE;
        }
        
        list = &(temp->next);
    }
    
    /* reached end of list without finding a channel entry for this index 
     * which should mean that it was never connected */
    return FALSE;
}

int op_channel_list_num_connected(OP_CHANNEL_LIST** list)
{
    OP_CHANNEL_LIST *temp;
    int num_channels = 0;
    
    patch_fn(op_channel_list_num_connected);
    
    
    while(*list)
    {
        temp = *list;
        
        /* check if either sink or source terminal is disconnected */
        if( (temp->pterminal_buf[OP_CHANNEL_LIST_SOURCE] == NULL) || 
            (temp->pterminal_buf[OP_CHANNEL_LIST_SINK] == NULL) )
        {
            /* connection is missing, so return failure */
            return OP_CHANNEL_LIST_CONNECTION_INVALID;
        }
        else
        {
            /* channel is active */
            num_channels++;
            list = &(temp->next);
        }
    }
    
    return num_channels;
}

void op_channel_list_free_list(OP_CHANNEL_LIST** list)
{
    OP_CHANNEL_LIST *temp;
    
    patch_fn(op_channel_list_free_list);
    
    while(*list)
    {
        temp = *list;
        list = &(temp->next);
        pfree(temp);
    }
}
