#ifndef __SHM_H__
#define __SHM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdlib.h"
#include "string.h"

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY_S2M                 9527
#define SHM_MEM_SIZE_S2M            1024 * 10
#define SHM_BUFFER_SIZE_S2M         SHM_MEM_SIZE_S2M/4

#define SHM_KEY_M2S                 9800
#define SHM_MEM_SIZE_M2S            4096
#define SHM_BUFFER_SIZE_M2S         SHM_MEM_SIZE_M2S/4

#define SHM_SEPARATOR               7777.7777
#define SHM_SEPARATOR_BGW           6666.6666
#define SHM_FINISH                  9999.9999

#define SHM_CMD_FINISH_0            19191919
#define SHM_CMD_FINISH_1            91919191



typedef enum
{
    XK_SHM_IDX_S2M_NUM_OF_DEV = 0,
    XK_SHM_IDX_S2M_LATEST_CMD_CNT,
    XK_SHM_IDX_S2M_RESERVED_2,
    XK_SHM_IDX_S2M_RESERVED_3,
    XK_SHM_IDX_S2M_RESERVED_4,
    XK_SHM_IDX_S2M_RESERVED_5,
    XK_SHM_IDX_S2M_RESERVED_6,
    XK_SHM_IDX_S2M_RESERVED_7,
    XK_SHM_IDX_S2M_RESERVED_8,
    XK_SHM_IDX_S2M_RESERVED_9,
    XK_SHM_IDX_S2M_SEPARATOR,
    XK_SHM_IDX_S2M_DATA_START,

} XK_SHM_IDX_S2M;


typedef enum
{
    XK_SHM_IDX_M2S_CHK_CNT = 0,
    XK_SHM_IDX_M2S_PORT,
    XK_SHM_IDX_M2S_CMD_START,

} XK_SHM_IDX_M2S;


int XK_ShmGet(key_t key, int size);
void *XK_ShmAttach(int shm_id);








#ifdef __cplusplus
}
#endif


#endif

