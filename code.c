#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define CONSUMER_COUNT 2
#define PRODUCER_COUNT 3
#define QUALITY_MULTIPLIER 2

pthread_mutex_t mutexConsumption = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexProduction = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t conditionConsumption = PTHREAD_COND_INITIALIZER;
pthread_cond_t conditionProduction = PTHREAD_COND_INITIALIZER;

int product = 0;
int productQualities[PRODUCER_COUNT];
int consumerQualities[CONSUMER_COUNT][PRODUCER_COUNT];

int isMyProduct();
void *consumer(void *);
void *producer(void *);
void assignProductQualities();
void createProductQualities();
void destroyMutexes();
void initMutexes();
void printConsumerQualities();
void printProductQualities();
void runThreads();

int main() {
    srand(time(NULL));
    createProductQualities();
    assignProductQualities();
    initMutexes();
    runThreads();
    destroyMutexes();
}

void initMutexes() {
    pthread_mutex_init(&mutexProduction, NULL);
    pthread_mutex_init(&mutexConsumption, NULL);
}

void destroyMutexes() {
    pthread_mutex_destroy(&mutexProduction);
    pthread_mutex_destroy(&mutexConsumption);
}

void runThreads() {
    pthread_t consumerThreads[CONSUMER_COUNT];
    pthread_t producerThreads[PRODUCER_COUNT];

    for (int i = 0; i < PRODUCER_COUNT; i++) {
        int *index = malloc(sizeof(int));
        *index = i;
        pthread_create(&producerThreads[i], NULL, &producer, index);
    }

    for (int i = 0; i < CONSUMER_COUNT; i++) {
        int *index = malloc(sizeof(int));
        *index = i;
        pthread_create(&consumerThreads[i], NULL, &consumer, index);
    }

    for (int i = 0; i < PRODUCER_COUNT; i++) {
        pthread_join(producerThreads[i], NULL);
    }

    for (int i = 0; i < CONSUMER_COUNT; i++) {
        pthread_join(consumerThreads[i], NULL);
    }
}

void *producer(void *j) {
    int i = *(int *)j;

    while (1) {
        int productionTime = rand() % productQualities[i] + 1;
        sleep(productionTime);

        printf("P%d (-) %d (%ds)\n", i, productQualities[i], productionTime);

        pthread_mutex_lock(&mutexProduction);

        if (product != 0) {
            pthread_cond_wait(&conditionProduction, &mutexProduction);
        }

        product = productQualities[i];

        printf("P%d --> %d\n", i, product);

        pthread_cond_signal(&conditionConsumption);
        pthread_mutex_unlock(&mutexConsumption);
    }

    free(j);
}

void *consumer(void *j) {
    int i = *(int *)j;

    while (1) {
        pthread_mutex_lock(&mutexConsumption);

        if (product == 0) {
            pthread_cond_wait(&conditionConsumption, &mutexConsumption);
        }

        if (isMyProduct(i, product) == 0) {
            pthread_cond_signal(&conditionConsumption);
            pthread_mutex_unlock(&mutexConsumption);
            continue;
        }

        int consumptionTime = rand() % (product / QUALITY_MULTIPLIER) + 1;
        sleep(consumptionTime);

        printf("C%d <-- %d (%ds)\n", i, product, consumptionTime);

        product = 0;

        pthread_cond_signal(&conditionProduction);
        pthread_mutex_unlock(&mutexProduction);
    }

    free(j);
}

void createProductQualities() {
    for (int i = 0; i < PRODUCER_COUNT; i++) {
        productQualities[i] = (i + 1) * QUALITY_MULTIPLIER;
    }

    printProductQualities();
}

void assignProductQualities() {
    int i = 0;
    int j = 0;
    int k = 0;

    while (k < PRODUCER_COUNT) {
        while (i < CONSUMER_COUNT && j < PRODUCER_COUNT && k < PRODUCER_COUNT) {
            consumerQualities[i++][j] = productQualities[k++];
        }
        i = 0;
        j++;
    }

    printConsumerQualities();
}

int isMyProduct(int i, int product) {
    for (int j = 0; j < PRODUCER_COUNT; j++) {
        if (consumerQualities[i][j] == product) {
            return 1;
        }
    }
    return 0;
}

void printProductQualities() {
    printf("Production assignment:\n");
    for (int i = 0; i < PRODUCER_COUNT; i++) {
        printf("P%d: %d\n", i, productQualities[i]);
    }
}

void printConsumerQualities() {
    printf("Consumption assignment:\n");
    for (int i = 0; i < CONSUMER_COUNT; i++) {
        printf("C%d: ", i);
        for (int j = 0; j < PRODUCER_COUNT; j++) {
            if (consumerQualities[i][j] != 0) {
                printf("%d ", consumerQualities[i][j]);
            }
        }
        printf("\n");
    }
    printf("\n");
}
