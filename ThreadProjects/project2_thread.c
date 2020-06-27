#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define MAX 1024

int NUM_THREADS; //number of threads
int len;         //length of string which each thread has to compute
int total = 0;
int n1, n2;
char *s1, *s2;
FILE *fp;

//creating mutex objects
pthread_mutex_t m;

//reading file function
int readf(FILE *fp)
{
    if ((fp = fopen("strings.txt", "r")) == NULL)
    {
        printf("ERROR: can't open string.txt\n");
        return 0;
    }
    s1 = (char *)malloc(sizeof(char) * MAX);
    if (s1 == NULL)
    {
        printf("ERROR: Out of memory!\n");
        return -1;
    }
    s2 = (char *)malloc(sizeof(char) * MAX);
    if (s2 == NULL)
    {
        printf("ERROR: Out of memory!\n");
        return -1;
    }
    //read s1 and s2 from the file
    s1 = fgets(s1, MAX, fp);
    s2 = fgets(s2, MAX, fp);
    n1 = strlen(s1) - 1;
    n2 = strlen(s2);
    if (s1 == NULL || s2 == NULL || n1 < n2)
        return -1;
}

void *num_substring(void *arg)
{
    //variables for convinence
    int i, j, k, start, end;

    //thread number
    long offset;
    offset = (long)arg;

    //calculating start and end pointer in n1 string for each thread
    if (offset == 0)
    {
        //printf("thread 0\n");
        start = offset * len;
        end = start + (len - 1);
    }
    else
    {
        //printf("thread more than 0\n");
        start = (offset * len) - 1;
        end = start + len;
    }
    printf("start = %d, end = %d and len=%d for thread=%ld\n", start, end, len, offset);
    int count;
    int val=0;
    int dest = end - (n2 - 1);
    printf("number of loops for thread %ld is %d\n", offset, dest - start);
    //printf("start of loop for thread %ld\n",offset);
    for (i = start; i <= dest; i++)
    {
        printf("thread %ld loop %d\n", offset, i);
        count = 0;
        for (j = i, k = 0; k < n2; j++, k++) //search for next string of size n2
        {
            if (*(s1 + j) != *(s2 + k))
            {
                printf("break loop no match\n");
                break;
            }
            else
            {
                printf("increase count\n");
                count++;
            }
            if (count == n2)
            {
                //since this is global variable we need to use mutex
                pthread_mutex_lock(&m);
                printf("increase total\n");
                val++;
                total++; //find a substring in next step
                pthread_mutex_unlock(&m);
            }
        }
    }
    pthread_exit((void *)val);
}

int main(int argc, char *argv[])
{
    int count;
    int num = 0;
    readf(fp);
    //decleare a pointer to an integer which is the list of possible 
    int *data;
    //now allocate memory

    printf("size of n1 = %d and n2= %d\n", n1, n2); //just to check

    data = malloc(sizeof(int) * n1);
    //allocation is failed if data = 0
    if (!data)
    {
        perror("Error allocation");
        exit(1);
    }
    //initalizing all ints in array with 0
    memset(data, 0, sizeof(int) * n1);

    //we know n1 MOD NUM_THREADS = 0 and n2 < n1/NUM_THREADS
    //calculating NUM_THREADS
    for (int i = 1; i <= n1; i++)
    {
        if (n1 % i == 0)
        {
            data[num] = i;
            num++;
        }
    }

    //check array with print
    for (int i = 0; i <= n1; i++)
    {
        printf("data[%d]=%d\n", i, data[i]);
    }

    //again n2 < n1/NUM_THREADS
    for (int i = 0; i < sizeof(data); i++)
    {
        if (data[i] == 0)
        {
            printf("skipped\n");
            break;
        }
        if ((n2) < (n1 / data[i]))
        {
            //we need to take max number of threads
            NUM_THREADS = data[i];
            printf("NUM_THREADS=%d\n", NUM_THREADS);
        }
    }

    printf("%d\n", NUM_THREADS);

    //calculating length of string n1 to which each thread has to look for
    len = n1 / NUM_THREADS;
    printf("%d", len);

    //Now I know how many threads I needed
    //creating thread objets array
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;
    int rc;
    long t;
    void *status;

    //initialize and set thread detach attribute
    pthread_mutex_init(&m, NULL);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (t = 0; t < NUM_THREADS; t++)
    {
        printf("Main creating thread %ld\n", t);
        rc = pthread_create(&threads[t], &attr, num_substring, (void *)t);
        if (rc)
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    //free attributes and wait for the other threads
    pthread_attr_destroy(&attr);
    for (t = 0; t < NUM_THREADS; t++)
    {
        rc = pthread_join(threads[t], &status);
        if (rc)
        {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
        printf("Main: completed join with thread %ld having a status of %ld\n", t, (long)status);
    }

    printf("The number of substrings is: %d\n", total);
    printf("Main program completed Exiting\n");
    pthread_mutex_destroy(&m);
    pthread_exit(NULL);
}