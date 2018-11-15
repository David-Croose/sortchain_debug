/********************************************************************************

                                  sort chain

            a queue that contains the nearest datas you put in, and
        you can get the middle-value(not average-value) from the queue.

********************************************************************************/

#include <stdio.h>
#include <string.h>
#include "sortchain.h"

// the sequence is in [0, MAX_SEQ) nomally, if @THRESHOLD > @MAX_SEQ, the
// sequence is in [0, THRESHOLD), in other words, the routine can adapt
// the @MAX_SEQ to @THRESHOLD automatically, so you don't need to change it.
#define MAX_SEQ     (4000000000)

#define THRESHOLD   (handle->thres)

/**
 * is the @handle full
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

    if(handle->sparenode != NULL)
    {
        *node = handle->sparenode;
        return SCHRES_OK;
    }
    else
    {
        for(i = 0; i < THRESHOLD; i++)
        {
            if(handle->nodes[i].hasdata_flag == SCH_FALSE)
            {
                *node = &handle->nodes[i];
                return SCHRES_OK;
            }
        }
    }

    return SCHRES_ERR;
}

/**
 * delete the oldest node from @handle
 * @param handle: the sort chain handle
 */
static void delete_oldestdata(schh_t *handle)
{
    schnode_t *delnode;     // delete node

    // get the oldest node
    fifo_od(&handle->odqh, &delnode);

    // delete the oldest node
    if(delnode->next == NULL)
    {
        // if @delnode is in the end
        delnode->prev->next = NULL;
    }
    else if(delnode == handle->head)
    {
        // if @delnode is in the head
        handle->head = delnode->next;
        delnode->next->prev = handle->head;
    }
    else
    {
        delnode->prev->next = delnode->next;
        delnode->next->prev = delnode->prev;
    }
    memset(delnode, 0, sizeof(schnode_t));

    // update @oldestseq
    handle->oldestseq++;

    // update @sparenode
    handle->sparenode = delnode;
}

///////////////////////////////////////
static void print_all(schh_t *handle)
{
#if 0
    schnode_t *anynode;
    static int aa;

    if(handle->full_flag == 1 && aa == 0)
    {
        aa = 1;
        printf("-------------------------------------\n");
    }

    for(anynode = handle->head; anynode != NULL; anynode = anynode->next)
    {
        printf("%d ", anynode->data);
    }
    printf("  ---  mid=%d\n", handle->mid);
#endif
}
///////////////////////////////////////

static schres_t insert_newestdata(schh_t *handle, schdat_t data)
{
    unsigned int i;
    schnode_t *node;
    schnode_t *anynode, *endnode;

    // find an empty node to fill
    if(find_emptynode(handle, &node) != SCHRES_OK)
    {
        return SCHRES_ERR;
    }

    // updating @sec
    // TODO  is it right that updating whole @sec?
    for(anynode = handle->head, i = 0; anynode != NULL; anynode = anynode->next, i++)
    {
        if(i % handle->secsz == 0)
        {
            handle->sec[i / handle->secsz].minimum = anynode->data;
            handle->sec[i / handle->secsz].addr = anynode;
        }
    }

    // fill the node
    (*node).data = data;
    (*node).seq = handle->newestseq++;
    (*node).hasdata_flag = SCH_TRUE;
    (*node).next = NULL;
    (*node).prev = NULL;
    if(handle->head == NULL)
    {
        // if there is only one data in sort chain now
        handle->head = node;
        (*node).prev = handle->head;
        goto after_inserting;
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
                        anynode->next->prev = node;
                        anynode->next = node;
                        (*node).prev = anynode;
                        break;
                    }
                    else
                    {
                        // if the @data > anynode.next, push the anynode to next
                    }
                }
                else
                {
                    // if anynode <= data < anynode.next==NULL, then the data
                    // would be in the end position
                    (*node).next = NULL;
                    (*node).prev = anynode;
                    anynode->next = node;
                    break;
                }
            }
            else
            {
                // data < head, we place it in the left of the head, and make
                // it be the head
                (*node).next = handle->head;
                handle->head->prev = node;
                handle->head = node;
                (*node).prev = handle->head;
                break;
            }
        }
        break;
    }
    if(i >= handle->sectot)     // the routine should never reach here
    {
        return SCHRES_ERR;
    }

after_inserting:

    // update the @full_flag
    if(handle->full_flag == SCH_FALSE && (handle->newestseq - handle->oldestseq) >= handle->thres)
    {
        handle->full_flag = SCH_TRUE;
    }

    // get mid-value
    // TODO  polling whole chain to get mid-value, not good
    for(anynode = handle->head, i = 0; anynode != NULL; anynode = anynode->next, i++)
    {
        if(i == THRESHOLD / 2)
        {
            handle->mid = anynode->data;
            break;
        }
    }

    // push the current node address to fifo
    fifo_in(&handle->odqh, &node);

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
    *mid = handle->mid;
}

/**
 * sort chain initialization
 * @param handle: the sort chain handle
 * @param thres: threshhold. the sortchain will pop out oldest data when
 *              the number of data reaches @thres
 * @param sectot: section total. this parameter has no concerns with @mid,
 *                but the speed of function @sortchain_add. it is suggested
 *                to make it equal to the square root of @thres to let
 *                @sortchain_add act fastest
 * @return: see @schres_t
 */
schres_t sortchain_init(schh_t *handle, unsigned int thres, unsigned int sectot)
{
    if(!handle || !thres || thres > SCH_NODES_TOTAL)
    {
        return SCHRES_ERR;
    }
    if(!sectot || thres % sectot > 0)
    {
        return SCHRES_ERR;
    }

    memset(handle, 0, sizeof(schh_t));
    handle->thres = thres;
    handle->sectot = sectot;
    handle->secsz = handle->thres / handle->sectot;
    fifo_init(&handle->odqh, handle->odqb, sizeof(schnode_t *), handle->thres + 1);
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

        print_all(handle);
    }
    else
    {
        if(insert_newestdata(handle, data) != SCHRES_OK)
        {
            return SCHRES_ERR;
        }

        print_all(handle);

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

int sortchain_demo_main(void)
{
    schres_t res;
    schh_t datalib;
    schdat_t mid;
    unsigned int i;
    schdat_t dat[] = {
        5, 10, 9, 17, 17, -13, 1, 8, 12, -6, 18, 2, 0, 11, 3, 15,
        19, 4, -7, 16, 0, 8, 4, 5, 13, 9, -14, 16, 7, 7, 3, -5, 11,
    };

    if(sortchain_init(&datalib, 8, 1) != SCHRES_OK)
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
