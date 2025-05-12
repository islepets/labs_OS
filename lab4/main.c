#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_THREADS 10
#define QUEUE_SIZE 30

typedef struct {
    void (*function)(void* p);
    void* data;
} task_t;

typedef struct {
    task_t queue[QUEUE_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    sem_t sem; 
    int shutdown_flag; 
} thread_pool_t;

thread_pool_t pool;
pthread_t threads[MAX_THREADS];
int thread_count = 0;


int enqueue(task_t t) {
    pthread_mutex_lock(&pool.mutex);
    if (pool.count == QUEUE_SIZE) {
        pthread_mutex_unlock(&pool.mutex);
        return 1; 
    }

    pool.queue[pool.tail] = t;
    pool.tail = (pool.tail + 1) % QUEUE_SIZE;
    pool.count++;
    pthread_mutex_unlock(&pool.mutex);
    sem_post(&pool.sem);
    return 0;
}

int dequeue(task_t *t) {
    pthread_mutex_lock(&pool.mutex);
    if (pool.count == 0) {
        pthread_mutex_unlock(&pool.mutex);
        return 1; 
    }

    *t = pool.queue[pool.head];
    pool.head = (pool.head + 1) % QUEUE_SIZE;
    pool.count--;
    pthread_mutex_unlock(&pool.mutex);
    return 0;
}


void* worker(void* arg) {
    while (1) {
        sem_wait(&pool.sem);

        pthread_mutex_lock(&pool.mutex);
        if (pool.shutdown_flag && pool.count == 0) {
            pthread_mutex_unlock(&pool.mutex);
            break;
        }
        pthread_mutex_unlock(&pool.mutex);

        task_t task;
        if (dequeue(&task) == 0) {
            task.function(task.data);
        }
    }
    return NULL;
}

int pool_init() {
    int i;
    pool.head = 0;
    pool.tail = 0;
    pool.count = 0;
    pool.shutdown_flag = 0;

    if (pthread_mutex_init(&pool.mutex, NULL) != 0) {
        return 1;
    }

    if (sem_init(&pool.sem, 0, 0) != 0) {
        pthread_mutex_destroy(&pool.mutex);
        return 1;
    }

    thread_count = MAX_THREADS;
    for (i = 0; i < thread_count; i++) {
        if (pthread_create(&threads[i], NULL, worker, NULL) != 0) {
            for (int j = 0; j < i; j++) {
                pthread_cancel(threads[j]);
                pthread_join(threads[j], NULL);
            }
            pthread_mutex_destroy(&pool.mutex);
            sem_destroy(&pool.sem);
            return 1;
        }
    }
    return 0;
}

int pool_submit(void (*func)(void* p), void* p) {
    task_t task;
    task.function = func;
    task.data = p;

    if (enqueue(task) != 0) {
        return 1; 
    }

    sem_post(&pool.sem);
    return 0;
}

void pool_shutdown() {
    pthread_mutex_lock(&pool.mutex);
    pool.shutdown_flag = 1;


    for (int i = 0; i < thread_count; i++) {
        sem_post(&pool.sem);
    }
    pthread_mutex_unlock(&pool.mutex);

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&pool.mutex);
    sem_destroy(&pool.sem);
}


void my_task(void* arg) {
    int task_id = *(int*)arg;
    printf("Поток %lu: Выполнение задачи %d\n", pthread_self(), task_id);
    sleep(1); 
}


int main() {
    int num_tasks = 50;

    if (pool_init() != 0) {
        return 1;
    }

    int* task_ids = malloc(num_tasks * sizeof(int));
    if (task_ids == NULL) {
        pool_shutdown();
        return 1;
    }

    for (int i = 0; i < num_tasks; i++) {
        task_ids[i] = i;
        if (pool_submit(my_task, &task_ids[i]) != 0) {
            fprintf(stderr, "Неудалось отправить задачу %d\n", i);
        }
    }

    sleep(5);

    pool_shutdown();

    free(task_ids);
    return 0;
}
