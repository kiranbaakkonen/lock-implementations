#include "stdlib.h"
#include "stdio.h"
#include "lock.h"
#include "stopwatch.h"
#include "string.h"
#include "pthread.h"
#include "statistics.h"

volatile long counter;
StopWatch_t timer;

struct thread_args{
    void * lock;
    int lock_type;
    long limit;
};

struct ret_args{
    long ipt;
};

/*
    Serial counter function
*/
void serial_implementation(long limit){
    while(1){
        if(counter < limit){
            counter++;
        }
        else{
            return;
        }
    }
}

/*
    Thread function that increments counter
*/
void * counter_thread(void * args){

    struct thread_args * t_args = (struct thread_args *)args;
    struct ret_args * r_args = malloc(sizeof(struct ret_args));
    r_args->ipt = 0;
    long ipt = 0;
    if(t_args->lock_type == 0){
        
        // increment counter using tas lock
        struct tas_lock * l = (struct tas_lock *) t_args->lock;
        long limit = t_args->limit;
        while(1){
            lock_tas(l);
            if(counter < limit){
                // printf("%ld\n", counter);
                counter++;
                ipt++;
            }
            else{
                unlock_tas(l);
                r_args->ipt = ipt;
                return (void*) r_args;
                // pthread_exit(NULL);
            }
            unlock_tas(l);
        }
    }
    else if(t_args->lock_type == 1){

        // increment counter using backoff lock
        struct backoff_lock * l = (struct backoff_lock *) t_args->lock;
        long limit = t_args->limit;
        while(1){
            lock_backoff(l);
            if(counter < limit){
                // printf("%ld\n", counter);
                counter++;
                ipt++;
            }
            else{
                unlock_backoff(l);
                r_args->ipt = ipt;
                return (void*) r_args;
                // pthread_exit(NULL);
            }
            unlock_backoff(l);
        }
    }
    else if(t_args->lock_type == 2){

        // increment counter using mutex lock
        pthread_mutex_t * mut = (pthread_mutex_t *) t_args->lock;
        long limit = t_args->limit;
        while(1){
            pthread_mutex_lock(mut);
            if(counter < limit){
                // printf("%ld\n", counter);
                counter++;
                ipt++;
            }
            else{
                pthread_mutex_unlock(mut);
                r_args->ipt = ipt;
                return (void*) r_args;
                // pthread_exit(NULL);
            }
            pthread_mutex_unlock(mut);
        }
    }
    else{

        // increment counter using array lock
        struct array_lock *l = (struct array_lock *) t_args->lock;
        long limit = t_args->limit;
        int slot;
        while(1){
            lock_array(l, &slot);
            if(counter < limit){
                // printf("%ld\n", counter);
                counter++;
                ipt++;
            }
            else{
                unlock_array(l, slot);
                r_args->ipt = ipt;
                return (void*) r_args;
                // pthread_exit(NULL);
            }
            unlock_array(l, slot);
        }
    }

    return (void*) r_args;
}

/*
    Parallel counter implementation
    create n threads and then has them run the counter thread function
*/
void parallel_implementation(int n, long limit, int lock_type, long count[]){

    // check if the args are valid
    if(n < 1){
        printf("Parallel implementation requires at least 1 thread\n");
        return;
    }

    // initialize t_args
    struct thread_args * t_args = malloc(sizeof(struct thread_args));
    t_args->lock_type = lock_type;
    t_args->limit = limit;
    if(lock_type == 0){
        
        // create a test and set lock
        struct tas_lock * l = malloc(sizeof(struct tas_lock));
        init_lock_tas(l);
        t_args->lock = (void*) l;
    }
    else if(lock_type == 1){

        // create a backoff lock;
        // min and max are purely speculative
        struct backoff_lock * l = malloc(sizeof(struct backoff_lock));
        init_lock_backoff(l, 1, 20);
        t_args->lock = (void*) l;
    }
    else if(lock_type == 2){
        
        // create a pthread mutex
        pthread_mutex_t * mut = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(mut, NULL);
        t_args->lock = (void*) mut;
    }
    else if(lock_type == 3){
        
        // create array lock
        struct array_lock * l = malloc(sizeof(struct array_lock));
        init_lock_array(l, n);
        t_args->lock = (void*) l;
    }
    else{
        printf("Parallel implementation requires a lock type 0-3\n");
        return;
    }


    // create threads
    pthread_t threads[n];
    void * r[n];
    startTimer(&timer);
    for(int i = 0; i < n; i++){
        pthread_create(&threads[i], NULL, counter_thread, (void *)t_args);
    }

    for(int i = 0; i < n; i++){
        pthread_join(threads[i], &r[i]);
    }
    stopTimer(&timer);

    for(int i = 0; i < n; i++){
        struct ret_args * res = (struct ret_args *) r[i];
        count[i] = res->ipt;
    }

}


/*
    main function usage is as follows
    proper usage is as follows: ./counter [option] [number of threads] [counter limit] [type of lock]
    options:
    -s = run serial counter
    -p = run parallel counter
    -ct = run the lock correcteness test
    -wbct = run the workbasedcounter test
    
    type of lock
    0 - TAS lock
    1 - pthread lock
    2 - array lock
    3 - backoff lock
*/
int main(int argc, char* argv[]){
    if(argc != 5){
        printf("proper usage is as follows: ./counter [option] [number of threads] [counter limit] [type of lock]\n");
        printf("options:\n");
        printf("-s = run serial counter\n");
        printf("-p = run parallel counter\n");
        printf("-a = run all version of the counter\n");
        printf("-ct = run the lock correcteness test (check that counter is correct and that total number of increments is correct)\n");
        printf("-wbct = run the work-based-counter test\n");
        printf("-ft = fairness test (return stddev of number of increments across threads)\n");
        printf("\ntype of lock\n");
        printf("0 - TAS lock\n");
        printf("1 - pthread lock\n");
        printf("2 - array lock\n");
        printf("3 - backoff lock\n");
        return -1;
    }

    int n = atoi(argv[2]);
    long counter_limit = atoi(argv[3]);
    int lock_type = atoi(argv[4]);

    if(strcmp(argv[1], "-s") == 0){
        printf("~~~~~~~~~~~~~~~~~Running serial counter~~~~~~~~~~~~~~~~~\n");
        counter = 0;
        startTimer(&timer);
        serial_implementation(counter_limit);
        stopTimer(&timer);
        printf("Time elapsed = %f\n", getElapsedTime(&timer));
        printf("Counter = %ld\n", counter);
        return 1;
    }
    else if(strcmp(argv[1], "-p") == 0){
        printf("~~~~~~~~~~~~~~~~~Running parallel counter~~~~~~~~~~~~~~~~~\n");
        counter = 0;
        long count[n];
        parallel_implementation(n, counter_limit, lock_type, count);
        printf("Time elapsed = %f\n", getElapsedTime(&timer));
        printf("Counter = %ld\n", counter);
        
        printf("Work done per thread\n");
        long sum = 0;
        for(int i = 0; i < n; i++){
            printf("%ld ", count[i]);
            sum += count[i];
        }
        printf("\n");
        printf("total = %ld\n", sum);
        printf("stdev = %f\n", getStdDev(count, n));

        return 1;
    }
    else if(strcmp(argv[1], "-a") == 0){
        printf("~~~~~~~~~~~~~~~~~Running parallel counter~~~~~~~~~~~~~~~~~\n");
        
        // serial version
        counter = 0;
        startTimer(&timer);
        serial_implementation(counter_limit);
        stopTimer(&timer);
        printf("Serial Time elapsed = %f\n", getElapsedTime(&timer));

        printf("~~~~~~~~~~~~~~~~~~~\n");

        long count[n];

        // tas lock
        counter = 0;
        parallel_implementation(n, counter_limit, 0, count);
        printf("TAS time elapsed = %f\n", getElapsedTime(&timer));

        
        printf("Work done per thread\n");
        long sum = 0;
        for(int i = 0; i < n; i++){
            printf("%ld ", count[i]);
            sum += count[i];
        }
        printf("\n");
        printf("total = %ld\n", sum);
        printf("stdev = %f\n", getStdDev(count, n));
        printf("~~~~~~~~~~~~~~~~~~~\n");


        // backoff lock
        counter = 0;
        parallel_implementation(n, counter_limit, 1, count);
        printf("Backoff time elapsed = %f\n", getElapsedTime(&timer));

        
        printf("Work done per thread\n");
        sum = 0;
        for(int i = 0; i < n; i++){
            printf("%ld ", count[i]);
            sum += count[i];
        }
        printf("\n");
        printf("total = %ld\n", sum);
        printf("stdev = %f\n", getStdDev(count, n));
        printf("~~~~~~~~~~~~~~~~~~~\n");


        // mutex lock
        counter = 0;
        parallel_implementation(n, counter_limit, 2, count);
        printf("Mutex time elapsed = %f\n", getElapsedTime(&timer));

        
        printf("Work done per thread\n");
        sum = 0;
        for(int i = 0; i < n; i++){
            printf("%ld ", count[i]);
            sum += count[i];
        }
        printf("\n");
        printf("total = %ld\n", sum);
        printf("stdev = %f\n", getStdDev(count, n));
        printf("~~~~~~~~~~~~~~~~~~~\n");


        // array lock
        counter = 0;
        parallel_implementation(n, counter_limit, 3, count);
        printf("Array time elapsed = %f\n", getElapsedTime(&timer));

        
        printf("Work done per thread\n");
        sum = 0;
        for(int i = 0; i < n; i++){
            printf("%ld ", count[i]);
            sum += count[i];
        }
        printf("\n");
        printf("total = %ld\n", sum);
        printf("stdev = %f\n", getStdDev(count, n));
        printf("~~~~~~~~~~~~~~~~~~~\n");


        return 1;
    }
    else if(strcmp(argv[1], "-ct") == 0){
        int retval = 1;
        printf("~~~~~~~~~~~~~~~~~Running correctness test~~~~~~~~~~~~~~~~~\n");
        printf("Runs the parallel implementation with each lock and keeps track of how many\n");
        printf("time each thread increments the counter (sum should be equal to the counter limit)\n");
        printf("~~~~~~~~~~~~~~~~~~~\n");
        long count[n];

        // tas lock
        counter = 0;
        parallel_implementation(n, counter_limit, 0, count);
        long sum = 0;
        for(int i = 0; i < n; i++){
            sum += count[i];
        }

        if(sum == counter && counter == counter_limit){
            printf("tas_lock pass\n");
        }
        else{
            printf("FAIL: tas_lock (sum = %ld, counter = %ld)\n", sum, counter);
            retval = -1;
        }


        // backoff lock
        counter = 0;
        parallel_implementation(n, counter_limit, 1, count);
        sum = 0;
        for(int i = 0; i < n; i++){
            sum += count[i];
        }
        
        if(sum == counter && counter == counter_limit){
            printf("backoff_lock pass\n");
        }
        else{
            printf("FAIL: backoff_lock (sum = %ld, counter = %ld)\n", sum, counter);
            retval = -1;
        }


        // mutex lock
        counter = 0;
        parallel_implementation(n, counter_limit, 2, count);
        sum = 0;
        for(int i = 0; i < n; i++){
            sum += count[i];
        }
        if(sum == counter && counter == counter_limit){
            printf("mutex_lock pass\n");
        }
        else{
            printf("FAIL: mutex_lock (sum = %ld, counter = %ld)\n", sum, counter);
            retval = -1;
        }


        // array lock
        counter = 0;
        parallel_implementation(n, counter_limit, 3, count);
        sum = 0;
        for(int i = 0; i < n; i++){
            sum += count[i];
        }
        if(sum == counter && counter == counter_limit){
            printf("array_lock pass\n");
        }
        else{
            printf("FAIL: array_lock (sum = %ld, counter = %ld)\n", sum, counter);
            retval = -1;
        }


        return retval;
    }
    else if(strcmp("-wbct", argv[1]) == 0){
        double time[5];
        long count[n];

        printf("~~~~~~~~~~~~~~~~~Running work based counter test~~~~~~~~~~~~~~~~~\n");


        // serial implementation
        counter = 0; 
        startTimer(&timer);
        serial_implementation(counter_limit);
        stopTimer(&timer);
        time[0] = getElapsedTime(&timer);

        // tas lock
        counter = 0;
        parallel_implementation(n, counter_limit, 0, count);
        time[1] = getElapsedTime(&timer);

        // backoff lock
        counter = 0;
        parallel_implementation(n, counter_limit, 1, count);
        time[2] = getElapsedTime(&timer);


        // mutex lock
        counter = 0;
        parallel_implementation(n, counter_limit, 2, count);
        time[3] = getElapsedTime(&timer);


        // array lock
        counter = 0;
        parallel_implementation(n, counter_limit, 3, count);
        time[4] = getElapsedTime(&timer);

        for(int i = 0; i < 5; i++){
            printf("%f\n", time[i]);
        }

        FILE * fp;
        fp = fopen("results.txt", "w+");
        fprintf(fp, "%f\n%f\n%f\n%f\n%f\n", time[0], time[1], time[2], time[3], time[4]);
        fclose(fp);

        return 1;
    }
    else if(strcmp("-ft", argv[1]) == 0){
        double stddev[4];
        long count[n];

        printf("~~~~~~~~~~~~~~~~~Running fairness test~~~~~~~~~~~~~~~~~\n");


        // tas lock
        counter = 0;
        parallel_implementation(n, counter_limit, 0, count);
        stddev[0] = getStdDev(count, n);

        // backoff lock
        counter = 0;
        parallel_implementation(n, counter_limit, 1, count);
        stddev[1] = getStdDev(count, n);


        // mutex lock
        counter = 0;
        parallel_implementation(n, counter_limit, 2, count);
        stddev[2] = getStdDev(count, n);


        // array lock
        counter = 0;
        parallel_implementation(n, counter_limit, 3, count);
        stddev[3] = getStdDev(count, n);

        for(int i = 0; i < 4; i++){
            printf("%f\n", stddev[i]);
        }

        FILE * fp;
        fp = fopen("results.txt", "w+");
        fprintf(fp, "%f\n%f\n%f\n%f\n", stddev[0], stddev[1], stddev[2], stddev[3]);
        fclose(fp);

        return 1;
    }
    else{
        printf("see usage\n");
        return -1;
    }

}