#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/sem.h"
#include "string.h"

//define the bool
#define BOOL int
#define TRUE 1
#define FALSE 0

#define SIZE1 10
//#define SIZE 999999
#define SIZE 1005000
#define SIZE2 10000

int prime_numbers[168]={2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,
                        127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,
                        211,223,227,229,233,239,241,251,257,263,269,271,277,281,
                        283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,
                        383,389,397,401,409,419,421,431,433,439,443,449,457,461,463,
                        467,479,487,491,499,503,509,521,523,541,547,557,563,569,571,
                        577,587,593,599,601,607,613,617,619,631,641,643,647,653,659,
                        661,673,677,683,691,701,709,719,727,733,739,743,751,757,761,
                        769,773,787,797,809,811,821,823,827,829,839,853,857,859,863,
                        877,881,883,887,907,911,919,929,937,941,947,953,967,971,977,
                        983,991,997};

int main(int argc, char *argv[]) {
    int nprocesses,upperlimit,lowlimit,mnprocesses,lnprocesses ;
    int n =1;
    pid_t mypid,fatherpid;
    int cnumber = 0;
    int numre = 0;
    int shmid,shmid2,shmid3;
    char *shmaddr;
    char *shmaddr2;
    char *shmaddr3;
    struct shmid_ds buf ;
    int flag = 0 ;

    int sem_id = 0;

    char message[20];

    BOOL isprime = FALSE;
    union semun sem_unioni;
    union semun sem_uniond;

    struct sembuf sem_bp;
    sem_bp.sem_num = 0;
    sem_bp.sem_op = -1;//P()
    sem_bp.sem_flg = SEM_UNDO;
    semop(sem_id, &sem_bp, 1);

    struct sembuf sem_bv;
    sem_bv.sem_num = 0;
    sem_bv.sem_op = 1;//V()
    sem_bv.sem_flg = SEM_UNDO;

    printf("Test Begin!\n");
    if(argv[1]){
        nprocesses = atoi(argv[1]);
        if(nprocesses < 1){
            printf("n-procs must be in [1, 10].\n");
            return -5;
        }
        else if(nprocesses > 10){
            printf("n-procs must be in [1, 10].\n");
            return -6;
        }
       // printf("You want %d processes!\n",nprocesses);
    }else{
        printf("Warning! You need to input the number of processes!\n");
        return -1;
    }
    if(argv[2]){
        lowlimit = atoi(argv[2]);
        if(lowlimit < 1001){
            printf("low must be in [1001, 999999].\n");
            return -7;
        }
        else if(lowlimit > 999999){
            printf("low must be in [1001, 999999].\n");
            return -8;
        }
      //  printf("The low limit is %d.\n",lowlimit);
    }else{
        printf("Warning! You need to input the low limit!\n");
        return -2;
    }
    if(argv[3]){
        upperlimit = atoi(argv[3]);
        if(upperlimit < 1001){
            //cout<<"Warning! should Larger than 1001"<<endl;
            printf("up must be in [1001, 999999].\n");
            return -9;
        }
        else if(upperlimit > 999999){
            // cout<<"Warning! should smaller than 999999!"<<endl;
            printf("up must be in [1001, 999999].\n");
            return -10;
        }
       // printf("The upper limit is %d.\n",upperlimit);
    }else{
        printf("Warning! You need to input the upper limit!\n");
        return -3;
    }
    if(upperlimit < lowlimit){
        printf("Warning! The upper limit should larger than the lower limit! \n");
        return -4;
    }

//init in the parent process
    //record the father pid to do the deinit
    fatherpid =getpid();

    //generate the semaphore
    sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);
    //init the semaphore
    sem_unioni.val = 1;
    semctl(sem_id, 0, SETVAL, sem_unioni);

    //generate the shared memory
    shmid = shmget(IPC_PRIVATE, SIZE1, IPC_CREAT|0600 ) ;
    shmaddr = (char *)shmat( shmid, NULL, 0 ) ;
    strcpy( shmaddr, "2") ;
    //set init value of cnumber to 2
    cnumber = atoi(shmaddr);
    shmdt( shmaddr ) ;

    //generate the shared memory2 rto store the f result
    shmid2 = shmget(IPC_PRIVATE, SIZE, IPC_CREAT|0600 ) ;

    //generate the shared memory3 to store primy number in the low2upper range
    shmid3 = shmget(IPC_PRIVATE, SIZE2, IPC_CREAT|0600 ) ;

    //fork nprocesses children process
    //totally nprocesses+1 processes
    for(int i=0;i<nprocesses;i++) {
        mypid = fork();
        if (mypid == 0) {
            break;
        }
    }

    //in all the children process do the calculation
    while(1) {
        //wait signal
        semop(sem_id, &sem_bp, 1);
        //attach to the shared memory
        shmaddr = (char *) shmat(shmid, NULL, 0);
        cnumber = atoi(shmaddr);
        if (cnumber > 999) {
            //deattach
            shmdt(shmaddr);
            //release
            semop(sem_id, &sem_bv, 1);
            break;
        }
        if (getpid() != fatherpid) {
            //write the calculation code here
            numre=0;
            for(int i =0 ;i<168;i++){
                if((cnumber % prime_numbers[i]) == 0){
                    numre++;
                }
            }


            if(cnumber >= lowlimit && cnumber <= upperlimit){
                isprime = TRUE;
                if(numre>0){
                    //have a factor,not prime number
                    isprime = FALSE;
                }
                else if(cnumber % 2 ==0 || cnumber % 3 ==0 || cnumber % 5 ==0 || cnumber % 7 ==0){
                    //have a factor 2,not prime number
                    isprime =  FALSE;
                }
                else{
                    for(int j = 5; j*j <= cnumber;j++){
                        if(cnumber % j ==0){
                            isprime =  FALSE;
                            break;
                        }
                    }
                }
                if(isprime){
                    shmaddr3 = (char *)shmat( shmid3, NULL, 0 ) ;
                    sprintf(message, "%d", cnumber);
                    strcat(shmaddr3,message);
                    strcat(shmaddr3," ");
                    shmdt(shmaddr3);
                }
            }


            shmaddr2 = (char *)shmat( shmid2, NULL, 0 ) ;
            sprintf(message, "%d", numre);
            strcat(shmaddr2,message);
            shmdt(shmaddr2);

            cnumber++;
        }
        sprintf(message, "%d", cnumber);
        strcpy(shmaddr, message);
        //deattach
        shmdt(shmaddr);
        //release signal
        semop(sem_id, &sem_bv, 1);

    }

    // cout<<"Result ------------------ END!"<<endl;
    //in the father process
    if(getpid() == fatherpid){
        //in the father process
        sleep(5);
        //wait(NULL);
        //cout<<"hhh"<<endl;
        printf("---------------------------\n");
        shmaddr2 = (char *)shmat( shmid2, NULL, 0 ) ;
        for(int i =0 ;i<998;i++){
            printf("f[%d]=%c\n",i+2,shmaddr2[i]);
           // cout<<"f["<<i+2<<"]="<<shmaddr2[i]<<endl;
        }
        shmdt(shmaddr2);
        printf("---------------------------\n");
        //cout<<"---------------------------"<<endl;
        printf("Primes in the range [%d,%d]:\n",lowlimit,upperlimit);
        //cout<<"Primes in the range ["<<lowlimit<<","<<upperlimit<<"]:"<<endl;

        shmaddr3 = (char *)shmat( shmid3, NULL, 0 ) ;
        printf("%s\n",shmaddr3);
        //cout<<shmaddr3<<endl;
        shmdt(shmaddr3);

        //delete the semaphore
        semctl(sem_id, 0, IPC_RMID, sem_uniond);
        //delete the shared memory
        shmctl(shmid, IPC_RMID, NULL) ;
        shmctl(shmid2, IPC_RMID, NULL) ;
        shmctl(shmid3, IPC_RMID, NULL) ;
    }

    return 0;
}