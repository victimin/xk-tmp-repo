#include "MsgQueue.h"

#define MAX_QUEUE_SIZE  (30)

stMsgHandler g_stQueueBuffer[MAX_QUEUE_SIZE];
int g_nQueFront;
int g_nQueRear;

void QueueInit (void) { 
   g_nQueFront = g_nQueRear = 0;
}

void QueueReset (void) {
   g_nQueFront = g_nQueRear;

}

stMsgHandler QueuePut(stMsgHandler QueueValue) 
{
    stMsgHandler RetureQueueValue;
    if ((g_nQueRear+1) % MAX_QUEUE_SIZE == g_nQueFront) {  
        // printf ("    Queue Overflow.\n");
        RetureQueueValue.m_nStatus = -1;
        return RetureQueueValue;
    }

    g_stQueueBuffer[g_nQueRear] = QueueValue;                   

    g_nQueRear = ++g_nQueRear % MAX_QUEUE_SIZE;         
    RetureQueueValue.m_nStatus = 1;

    return RetureQueueValue;
}

 

stMsgHandler QueueGet(void) 
{
    stMsgHandler RetureQueueValue;

    if (g_nQueFront == g_nQueRear) {             
    //    printf ("    Queue is empty.\n");
        RetureQueueValue.m_nStatus = -1;
        return RetureQueueValue;
    }

    RetureQueueValue = g_stQueueBuffer[g_nQueFront];  

 //   printf("RetureQueueValue.m_nCommand : %d, m_nAlgorithmCore : %d, m_nMetaDataAddr :%d \n", 
 //       RetureQueueValue.m_nCommand,RetureQueueValue.m_nAlgorithmCore, RetureQueueValue.m_nMetaDataAddr);

    RetureQueueValue.m_nStatus = 1;

    
    g_nQueFront = ++g_nQueFront%MAX_QUEUE_SIZE;  
    return RetureQueueValue;
}



