/********************************************************************************

                                  sort chain

            a queue that contains the nearest datas you put in, and
        you can get the middle-value(not average-value) from the queue.

********************************************************************************/

#include <stdio.h>
#include <string.h>
#include "sortchain.h"

// the sequence is in [0, MAX_SEQ)
#define MAX_SEQ     (4000000000)     /* the larger the better */

#define THRESHOLD   (handle->thres)

/**
 * judge is a @handle full
 * @param handle: the sort chain handle
 * @return SCH_TRUE : full
 *         SCH_FALSE : not full
 */
static char is_full(schh_t *handle)
{
    return handle->full_flag;
}

/**
 * find an empty node from @handle
 * @param handle: the sort chain handle
 * @param node: the pointer variable stores the empty node's address
 * @return: see @schres_t
 */
static schres_t find_emptynode(schh_t *handle, schnode_t **node)
{
    unsigned int i;

    for(i = 0; i < THRESHOLD; i++)
    {
        if(handle->nodes[i].hasdata_flag == SCH_FALSE)
        {
            *node = &handle->nodes[i];
            return SCHRES_OK;
        }
    }

    return SCHRES_ERR;
}

/**
 * delete the oldest node of @handle
 * @param handle: the sort chain handle
 */
static void delete_oldestdata(schh_t *handle)
{
    schnode_t *anynode, *deletenode;

    // if the first one is the oldest one
    if(handle->head->seq == handle->oldestseq)
    {
        anynode = handle->head;
        handle->head = handle->head->next;
        anynode->next = NULL;
        anynode->hasdata_flag = SCH_FALSE;
        anynode->data = 0;
        handle->oldestseq++;
        anynode->seq = 0;
        return;
    }

    // if the oldest one isn't in the first place
    for(anynode = handle->head; anynode != NULL; anynode = anynode->next)
    {
        if((anynode->next->seq == handle->oldestseq))
        {
            deletenode = anynode->next;
            anynode->next = deletenode->next;
            deletenode->next = NULL;
            deletenode->hasdata_flag = SCH_FALSE;
            handle->oldestseq++;
            deletenode->seq = 0;
            deletenode->data = 0;
            return;
        }
    }
}

static schres_t insert_newestdata(schh_t *handle, schdat_t data)
{
    unsigned int i;
    unsigned int secsz;     // section size
    schnode_t *node;
    schnode_t *anynode, *endnode;

    // find an empty node to fill
    if(find_emptynode(handle, &node) != SCHRES_OK)
    {
        return SCHRES_ERR;
    }

    // fill the node
    (*node).data = data;
    (*node).seq = handle->newestseq++;
    (*node).hasdata_flag = SCH_TRUE;
    (*node).next = NULL;
    if(handle->head == NULL)
    {
        // if there is only one data in sort chain now
        handle->head = node;
        goto update;
    }

    // searching all sections to know where the @data should lay in
    for(i = 0; i < handle->sectot; i++)
    {
        // if @data should lay in the @ith section
        if(handle->sec[i].minimum <= data)
        {
            if(handle->sec[i + 1].addr != NULL)
            {
                if(data < handle->sec[i + 1].minimum)
                {
                    endnode = handle->sec[i + 1].addr;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                endnode = NULL;
            }
        }
        else
        {
            endnode = NULL;
        }

        // entering the @ith section to find the real place of @data
        for(anynode = handle->sec[i].addr; anynode != endnode; anynode = anynode->next)
        {
            // if data >= head, make it compare to the latter datas one by one until
            // we get it's position
            if(anynode->data <= data)
            {
                // if anynode <= data < anynode.next, then the data could lay
                // between them
                if(anynode->next != NULL)
                {
                    if(data < anynode->next->data)
                    {
                        (*node).next = anynode->next;
                        anynode->next = node;
                        break;
                    }
                }
                else
                {
                    // if anynode <= data < anynode.next==NULL, then the data
                    // would be in the end position
                    anynode->next = node;
                    (*node).next = NULL;
                    break;
                }
            }
            else
            {
                // data < head, we place it in the left of the head, and make
                // it be the head
                (*node).next = handle->head;
                handle->head = node;
                break;
            }
        }
        break;
    }
    if(i >= handle->sectot - 1)     // the routine should never reach here
    {
        return SCHRES_ERR;      // TODO  is that right to ignore the @update
    }

update:    // after inserting the @data, the @sec should be updated
    // TODO  is it right that update whole @sec?
    secsz = handle->thres / handle->sectot;
    for(anynode = head, i = 0; anynode != NULL; anynode = anynode->next, i++)
    {
        if(i == THRESHOLD / 2)
        {
            handle->mid = anynode->data;
        }
        if(i % secsz == 0)
        {
            handle->sec[i / secsz].minimum = anynode->data;
            handle->sec[i / secsz].addr = anynode;
        }
    }

    return SCHRES_OK;
}

/**
 * is the sequence number goes to the top(@MAX_SEQ)
 * @param handle: the sort chain handle
 * @return SCH_TRUE: the sequence had reached to the top
 *         SCH_FALSE: the sequence hadn't reached to the top yet
 */
static char is_seqtop(schh_t *handle)
{
    if(handle->newestseq - 1 >= MAX_SEQ)
    {
        return SCH_TRUE;
    }
    return SCH_FALSE;
}

/**
 * reset all nodes' sequence
 * @param handle: the sort chain handle
 */
static void reset_all_seq(schh_t *handle)
{
    unsigned int i;

    for(i = 0; i < THRESHOLD; i++)
    {
        handle->nodes[i].seq -= handle->oldestseq;
    }
    handle->newestseq -= handle->oldestseq;
    handle->oldestseq -= handle->oldestseq;
}

/**
 * get the middle-value of the @handle
 * @param handle: the sort chain handle
 * @param mid: the pointer variable stores the middle-value
 */
static void get_mid(schh_t *handle, schdat_t *mid)
{
    unsigned int i;
    schnode_t *anynode = handle->head;

    for(i = 0; i < THRESHOLD / 2; i++)
    {
        anynode = anynode->next;
    }
    *mid = anynode->data;
}

/**
 * sort chain initialization
 * @param handle: the sort chain handle
 * @param thres: threshhold. the sortchain will pop out oldest data when
 *              the number of data reaches @thres
 * @param sectot: section total
 * @return: see @schres_t
 */
schres_t sortchain_init(schh_t *handle, unsigned int thres, unsigned int sectot)
{
    if(!handle || !thres)
    {
        return SCHRES_ERR;
    }
    if(thres / 2 * 2 == thres)  // thres can not be even
    {
        return SCHRES_ERR;
    }
    if(thres > SCH_NODES_TOTAL)
    {
        thres = SCH_NODES_TOTAL;
    }
    if(!sectot || sectot > thres || thres % sectot > 0)
    {
        return SCHRES_ERR;
    }

    memset(handle, 0, sizeof(schh_t));
    handle->thres = thres;
    return SCHRES_OK;
}

/**
 * add a data to @handle, and if it returns SCHRES_OK, you can get the
 * middle-value
 * @param handle: the sort chain handle
 * @param data: the data to be added in
 * @param mid: the pointer variable stores the middle-value
 * @return: see @schres_t
 */
schres_t sortchain_add(schh_t *handle, schdat_t data, schdat_t *mid)
{
    if(!handle || !mid)
    {
        return SCHRES_ERR;
    }

    if(is_full(handle) == SCH_TRUE)
    {
        delete_oldestdata(handle);
        if(insert_newestdata(handle, data) != SCHRES_OK)
        {
            return SCHRES_ERR;
        }
        if(is_seqtop(handle) == SCH_TRUE)
        {
            reset_all_seq(handle);
        }
        get_mid(handle, mid);
    }
    else
    {
        if(insert_newestdata(handle, data) != SCHRES_OK)
        {
            return SCHRES_ERR;
        }
        if(is_full(handle) == SCH_TRUE)
        {
            if(is_seqtop(handle) == SCH_TRUE)
            {
                reset_all_seq(handle);
            }
            get_mid(handle, mid);
            return SCHRES_OK;
        }

        return SCHRES_NOTREADY;
    }

    return SCHRES_OK;
}

int demo_main(void)
{
    schres_t res;
    schh_t datalib;
    schdat_t mid;
    /// schdat_t dat[] = {5, 9, 1, 8, 6, 2, 0, 3, 4, 7};
    schdat_t dat[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    unsigned int i;

    if(sortchain_init(&datalib, 6, 2) != SCHRES_OK)
    {
        printf("init error!\n");
        return -1;
    }

    for(i = 0; i < sizeof(dat) / sizeof(dat[0]); i++)
    {
        res = sortchain_add(&datalib, dat[i], &mid);
        if(res == SCHRES_OK)
        {
            printf("%d ", mid);
        }
        else if(res == SCHRES_NOTREADY)
        {
            printf("-");
        }
        else
        {
            printf("run error!\n");
            return -2;
        }
    }
    return 0;
}
