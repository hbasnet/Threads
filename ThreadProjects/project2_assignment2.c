#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//global variables
#define N 5 //queue size
int itemCount = 0;

pthread_mutex_t obj_mutex;         //creating mutex object
pthread_cond_t cond_con, cond_pro; //conditional variables for producer and consumer
char ch;
FILE *filepointer; //file pointer

//circular queue
char items[N];
int front = -1, rear = -1;

//check if queue is empty
int isEmpty()
{
    if (front == -1 && rear == -1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//insert element in a queue
void enQueue(char ch)
{
    if ((rear + 1) % N == front)
    {
        printf("Queue is full\n");
        return;
    }
    else if (isEmpty())
    {
        front = rear = 0;
    }
    else
    {
        rear = (rear + 1) % N;
    }
    items[rear] = ch;
    itemCount++;
    printf("item count %d\n", itemCount);
}

//remove element from a queue and save it in element variable
char deQueue()
{
    char ch;
    if (isEmpty())
    {
        printf("Queue is empty\n");
        return -1;
    }
    else if (front == rear)
    {
        ch = items[front];
        front = rear = -1;
    }
    else
    {
        ch = items[front];
        front = (front + 1) % N;
    }

    itemCount--;
    printf("item count %d\n", itemCount);
    return ch;
}

//producer function which enqueue character from file to queue
void *produce(void *arg)
{
    //loop till end of file
    while ((ch = fgetc(filepointer)) != EOF)
    {
        printf("producer \n");
        //use mutex lock to protect queue
        pthread_mutex_lock(&obj_mutex);
        //sleep if queue is full
        while ((rear + 1) % N == front)
        {
            printf("producer wait\n");
            pthread_cond_wait(&cond_pro, &obj_mutex); //while sleep it will unlock mutex itself
        }
        printf("%c produced\n", ch);
        enQueue(ch); //insert a character in a queue
        //once you have sth in queue its ok to wake up consumer
        pthread_cond_signal(&cond_con);
        pthread_mutex_unlock(&obj_mutex);
    }
    pthread_exit(NULL);
}

//consumer function dqueue each character from front of queue
void *consume(void *arg)
{
    while (1)
    {
        printf("consumer \n");
        pthread_mutex_lock(&obj_mutex);
        //if queue is empty then there nothing to consume so will wait for producer
        while (isEmpty())
        {

            if (feof(filepointer) && isEmpty())
            {
                printf("completed producing and consuming\n");
                exit(1);
            }
            printf("consumer wait\n");
            pthread_cond_wait(&cond_con, &obj_mutex);
        }
        //dequeue will return a character
        printf("%c consumed\n", deQueue());
        pthread_cond_signal(&cond_pro);   // once the character is consumed its ok to wake up producer
        pthread_mutex_unlock(&obj_mutex); //release the queue
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    pthread_t producer;
    pthread_t consumer; //creating two threads for producer and consumer

    //opening firle in read mode
    filepointer = fopen("message.txt", "r");
    if (filepointer == 0)
    {
        printf("cannot open a file\n");
    }

    //initialize mutex and conditional variables
    pthread_mutex_init(&obj_mutex, NULL);
    pthread_cond_init(&cond_con, NULL);
    pthread_cond_init(&cond_pro, NULL);

    //create threads
    pthread_create(&producer, NULL, produce, NULL);
    pthread_create(&consumer, NULL, consume, NULL);

    //wait for the thread to complete otherwise main will complete before thread complete
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    //cleanup
    pthread_mutex_destroy(&obj_mutex);
    pthread_cond_destroy(&cond_pro);
    pthread_cond_destroy(&cond_con);

    //file close
    fclose(filepointer);
}
