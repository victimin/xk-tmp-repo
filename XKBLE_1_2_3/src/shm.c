#include "shm.h"

int XK_ShmGet(key_t key, int size){
    int shm_id;
    if ( -1 == ( shm_id = shmget( (key_t)key, size, IPC_CREAT|0666))){
        printf("Filed to create SHM\n");
        return -1;
    }
    return shm_id;
}

void *XK_ShmAttach(int shm_id){
    void *tmpAddr;
    if ( ( void *)-1 == ( tmpAddr = shmat( shm_id, ( void *)0, 0)))
    {
        printf( "Filed to attach SHM\n");
        return -1;
    }
    return tmpAddr;

}
