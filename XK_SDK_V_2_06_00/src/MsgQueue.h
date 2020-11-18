#ifndef __MESSEAGE_QUEUES_H__
#define __MESSEAGE_QUEUES_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "XK_CommonDefineSet.h"

// #include <AlgoThread.h>



void QueueInit(void);
void QueueReset(void);
stMsgHandler QueuePut(stMsgHandler QueueValue);
stMsgHandler QueueGet(void);



#ifdef __cplusplus
}
#endif


#endif

