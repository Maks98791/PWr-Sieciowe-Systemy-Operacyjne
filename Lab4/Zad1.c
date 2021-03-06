#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/sem.h>
#include "SemLibrary.h"
#include "SMLibrary.h"

int main(int argc, char * argv[]){ 

    key_t key = KEY_VALUE;
    int shmId = 0, semId;
    double deposit;
    struct shData *data;
    int clients, account, account2, i, iterations;

    if(argc == 2 || argc > 5) {
        printf("Incorrect parameters quantity!\nProgram will close...\n");
        exit(0);
    }
    else if(argc == 3){
        account = atof(argv[1]);    
        iterations = atof(argv[2]);    
    }
    else if(argc == 4) {
        account = atof(argv[1]);    
        deposit = atof(argv[2]); 
        iterations = atof(argv[3]);        
    }
    else if(argc == 5) {
        account = atof(argv[1]);    
        account2 = atof(argv[2]);    
        deposit = atof(argv[3]); 
        iterations = atof(argv[4]);           
    }

    semId = bSemCreate(key, "/tmp/", 'E', bankAccounts); 
    data = createOrGetSM(key, &shmId, semId);

    /* Wait some time for creation all structures */
    sleep(5);
    for(i = 0; i < iterations; i++) {
        if(argc == 3) {
            checkBalance(data, account);
        } 
        else if(argc == 4 ) {
            bSemBlockP(semId, account);
            makeDeposit(account, deposit, data);
            bSemUnblockV(semId, account);
        }
        else if(argc == 5) {
            bSemBlockP(semId, account);
            bSemBlockP(semId, account2);
            makeDeposit(account, (-1) * deposit, data);
            makeDeposit(account2, deposit, data);
            bSemUnblockV(semId, account2);    
            bSemUnblockV(semId, account);    
        }
    }
    /* Wait some time before disconnect Shared Memory */
    sleep(2);
    clients = disconnectSM(data, semId);
    
    /* Wait some time before checking if all clients are disconnected and can remove shared memory and semaphores */
    sleep(5);
    if(clients == 0) {
        removeSM(clients, shmId);        
        bSemDelete(semId, 0);
    }  
    return 1;
}   