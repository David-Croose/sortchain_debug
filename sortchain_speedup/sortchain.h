/********************************************************************************

                                  sort chain

            a queue that contains the nearest datas you put in, and
        you can get the middle-value(not average-value) from the queue.

********************************************************************************/

#ifndef _SORTCHAIN_H_
#define _SORTCHAIN_H_

#include "fifo.h"

#ifdef __cplusplus
extern "C" {
#endif

// user configure here
typedef float schdat_t;       /* sort chain data */
#define SCH_NODES_TOTAL     (3999)

typedef enum {
    DELP_LEFT,
    DELP_RIGHT,
    DELP_MID,
} delp_t;                   // delete position

#define SCH_TRUE    (1)
#define SCH_FALSE   (0)

typedef struct SCHNODE {
    schdat_t data;
    unsigned int seq;
    char hasdata_flag;
    struct SCHNODE *next;                  // next node
    struct SCHNODE *prev;                  // previous node
} schnode_t;   // sort chain node

typedef struct {
    schnode_t nodes[SCH_NODES_TOTAL];      // TODO  should be configurable here
    char full_flag;
    unsigned int oldestseq;                // the oldest sequence
    unsigned int newestseq;                // the newest sequence, note that the variable
                                           // represents the new coming data seq, the real
                                           // newest seq in sort chain is @newestseq - 1
    schnode_t *head;
    unsigned int thres;                    // threshold, the window size of the this handle
    unsigned int sectot;                   // section-total
    unsigned int secsz;                    // section-size, equals to @thres / @sectot
    struct SEC {
        schnode_t *addr;                   // each section's minimal node address
        schdat_t minimum;                  // each section's minimal data
    } sec[SCH_NODES_TOTAL];                // section. a sort-chain will be divided into
                                           // @sectot sections, each of them has it's own
                                           // minimal data konwn as @minimum, and the @addr
                                           // is pointed to the minimal node of the sort-
                                           // chain.
    schnode_t *midnode;
    schnode_t *ldelnode;                   // left-delete-node. the node in the left of the
                                           // deleted node
    delp_t del_flag;                       // the deleted node position refering to midnode

    struct NODE odqh;                      // oldest data queue handle
    schnode_t *odqb[SCH_NODES_TOTAL];      // oldest data queue buffer

    schnode_t *sparenode;                  // when a node had been deleted from a sortchain, it
                                           // becomes a spare-node
} schh_t;                                  // sort-chain handle

typedef enum {
    SCHRES_OK,         // ok
    SCHRES_NOTREADY,   // not ready
    SCHRES_ERR,        // error
} schres_t;            // sort chain result

schres_t sortchain_init(schh_t *handle, unsigned int thres, unsigned int sectot);
schres_t sortchain_add(schh_t *handle, schdat_t data, schdat_t *mid);

#ifdef __cplusplus
}
#endif

#endif
