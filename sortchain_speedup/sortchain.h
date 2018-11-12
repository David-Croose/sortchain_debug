/********************************************************************************

                                  sort chain

            a queue that contains the nearest datas you put in, and
        you can get the middle-value(not average-value) from the queue.

********************************************************************************/

#ifndef _SORTCHAIN_H_
#define _SORTCHAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

// user configure here
typedef float schdat_t;       /* sort chain data */
#define SCH_NODES_TOTAL     (3999)

#if ((SCH_NODES_TOTAL / 2 * 2) == SCH_NODES_TOTAL)
#   error the @SCH_NODES_TOTAL must be odd!
#endif

#if (SCH_NODES_TOTAL == 1)
#   error the @SCH_NODES_TOTAL can not be 1!
#endif

#define SCH_TRUE    (1)
#define SCH_FALSE   (0)

typedef struct SCHNODE {
    schdat_t data;
    unsigned int seq;
    char hasdata_flag;
    struct SCHNODE *next;
} schnode_t;   // sort chain node

typedef struct {
    schnode_t nodes[SCH_NODES_TOTAL];   // TODO  should be configurable here
    char full_flag;
    unsigned int oldestseq;     // the oldest sequence
    unsigned int newestseq;     // the newest sequence, note that the variable
                                // represents the new coming data seq, the real
                                // newest seq in sort chain is @newestseq - 1
    schnode_t *head;
    unsigned int thres;         // threshold
} schh_t;

typedef enum {
    SCHRES_OK,         // ok
    SCHRES_NOTREADY,   // not ready
    SCHRES_ERR,        // error
} schres_t;            // sort chain result

schres_t sortchain_init(schh_t *handle, unsigned int thres);
schres_t sortchain_add(schh_t *handle, schdat_t data, schdat_t *mid);

#ifdef __cplusplus
}
#endif

#endif
