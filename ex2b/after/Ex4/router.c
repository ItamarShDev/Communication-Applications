#include "router.h"

int trials; //How many times to try to establish connection
Table *table; // Hash table for saving the data - vertexes and their neighbors
void *sourceName; // Saving the name of the source
int finished, changed;
//------------initialization for threads -------------------------------
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t conditionSender = PTHREAD_COND_INITIALIZER;
pthread_cond_t conditionCalculator = PTHREAD_COND_INITIALIZER;

void prepare(char *name, int tr, Table *t)
{
    int err, counter = 0, i = 0;
    pthread_t *send_t = NULL, *receive_t = NULL;
    pthread_t calc_t;
    Object *obj = NULL;
    sourceName = name;
    trials = tr;
    table = t;

    relax();  //First time update your neighbors weight
    finished = 0; //The program not finished


    obj = search(table, sourceName);

    if (obj == NULL)
    {
        printf("No such source\n");
        pthread_exit(NULL);
    }

    //counting neighbors
    while (obj->next != NULL)
    {
        counter++;
        obj = obj->next;
    }

    obj = search(table, sourceName);
    obj->sent = 0;

    //Create pthread arrays
    send_t = malloc(sizeof(pthread_t) * counter);
    receive_t = malloc(sizeof(pthread_t) * counter);

    while (obj->next != NULL)
    {
        obj = obj->next;
        obj->sent = -1;
    }

    obj = search(table, sourceName);

    //For each neighbor of source create thread for sender and thread for receiver
    while (obj->next != NULL)
    {
        obj = obj->next;
        obj->sent = -1;
        err = pthread_create(&send_t[i], NULL, sender, (void *)obj);

        if (err != 0)
        {
            printf("Error threading\n");
            pthread_exit(NULL);
        }

        err = pthread_create(&receive_t[i], NULL, receiver, (void *)obj);

        if (err != 0)
        {
            printf("Error threading\n");
            pthread_exit(NULL);
        }

        i++;
    }
    //Create thread for calculator
    err = pthread_create(&calc_t, NULL, calculator, (void *)obj);
    if (err != 0)
    {
        printf("Error threading\n");
        exit(1);
    }
    //Join all threads
    for (i = 0; i < counter; i++)
    {
        pthread_join(send_t[i], NULL);
        pthread_join(receive_t[i], NULL);
    }
    pthread_join(calc_t, NULL);

    free(send_t);
    free(receive_t);
}

void *sender(void *nei)
{
    struct sockaddr_in serv_addr; //struct for server address
    int portnum; //port number for socket connection
    int sockfd, rw_error;
    struct hostent *server;
    int cc = 0; //conector counter
    int i, sum = 0, j = 0;
    char *name = (char *)sourceName;
    int arr[num + 1];
    Object *neighbor = (Object *)nei;

    portnum = neighbor->port;

    for (i = 0; i < strlen(name); i++)
        sum += (int)name[i];

    portnum += sum;

    server = gethostbyname(neighbor->ip);

    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such server\n");
        return NULL;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portnum);


    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("ERROR opening socket");
        return NULL;
    }

    while (1) //Try several times to connect to server
    {
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            if (cc == trials)
            {

                pthread_exit(NULL);
            }

            printf("Trying to connect to neighbor \n");
            cc++;
            sleep(1);
        }
        else
        {
            break; //connection established
        }
    }
    printf("connection established \n");
    //--------------First time send anyway -----------
    pthread_mutex_lock(&lock);
    arr[0] = 1;
    j++;

    for (i = 0; i < table->currentSize; i++)
    {
        if (table->arr[i] != NULL)
        {
            arr[j] = table->arr[i]->weight;
            j++;
        }
    }

    pthread_mutex_unlock(&lock);
    rw_error = write(sockfd, arr, (num + 1) * sizeof(int));

    if (rw_error < 0)
    {
        printf("ERROR writing to socket\n");
        return NULL;
    }

    pthread_mutex_lock(&lock);

    while (finished == 0)
    {
        pthread_cond_wait(&conditionSender, &lock);
        j = 1;

        for (i = 0; i < table->currentSize; i++)
        {
            if (table->arr[i] != NULL)
            {
                arr[j] = table->arr[i]->weight;
                j++;
            }
        }

        if (changed == 1) //DV changed
        {
            arr[0] = 1;
        }
        else
        {
            arr[0] = 0;
        }

        pthread_mutex_unlock(&lock);

        rw_error = write(sockfd, arr, (num + 1) * sizeof(int));

        if (rw_error < 0)
        {
            printf("ERROR writing to socket\n");
            return NULL;
        }

        if (rw_error == 0)
        {
            printf("No one want to talk to me  =( \n");
            return NULL;
        }
    }

    close(sockfd);
    pthread_mutex_unlock(&lock);
    return NULL;
}

void *receiver(void *nei)
{
    struct sockaddr_in serv_addr, cli_addr;
    int sockfd, newsockfd;
    int portnum, rw_error;
    int i = 0, sum = 0;
    socklen_t clilen;
    int arr[num + 1];
    Object *obj;
    Object *neighbor = (Object *)nei;
    char *name = (char *)neighbor->vertex;

    memset(arr, 0, (num + 1)*sizeof(int));

    obj = search(table, sourceName);
    portnum = obj->port;

    for (i = 0; i < strlen(name); i++)
        sum += (int)name[i];

    portnum += sum;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portnum);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR on binding\n");
        close(sockfd);
        return NULL;
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

    if (newsockfd < 0)
    {
        printf("ERROR on accept");
        close(newsockfd);
        close(sockfd);
        return NULL;
    }

    obj->socketfd = sockfd;
    neighbor->socketfd = newsockfd;
    pthread_mutex_lock(&lock);
    while (finished == 0)
    {
        pthread_mutex_unlock(&lock);
        rw_error = read(newsockfd, arr, (num + 1) * sizeof(int));

        if (rw_error < 0)
        {
            printf("ERROR reading from socket\n");
            close(sockfd);
            close(newsockfd);
            return NULL;
        }
        else if (rw_error == 0)
        {
            close(sockfd);
            close(newsockfd);
            return NULL;
        }

        pthread_mutex_lock(&lock);

        obj = search(table, sourceName);

        if (obj == NULL)
            printf("something wrong\n");

        obj = obj->next;

        //neighbor sent 0 -> no changes
        if (arr[0] == 0)
        {
            while (obj != NULL)
            {
                if (strcmp(obj->vertex, neighbor->vertex) == 0)
                    obj->sent = 0;

                obj = obj->next;
            }
        }
        else // need to update DV
        {
            while (obj != NULL)
            {
                if (strcmp(obj->vertex, neighbor->vertex) == 0)
                    obj->sent = 1;

                obj = obj->next;
            }
            update(arr, neighbor->vertex);
        }

        if (checkAllSent() == 1 || check() == 1)//if got zero from all neighbors or DV then wake up calculator
        {
            pthread_cond_signal(&conditionCalculator);
        }

        pthread_mutex_unlock(&lock);
    }

    pthread_mutex_unlock(&lock);
    close(sockfd);
    close(newsockfd);
    return NULL;

}


//Thread calculator check if all sent, then call to relax, if all sent 0 finishing the program
void *calculator(void *nei)
{
    int chk = 0;
    Object *obj;

    pthread_mutex_lock(&lock);
    while (finished == 0)
    {
        pthread_cond_wait(&conditionCalculator, &lock);

        chk = check();
        relax();
        pthread_cond_broadcast(&conditionSender);

        if (chk == 1)
        {
            printTable(table);
            pthread_cond_broadcast(&conditionSender);
            finished = 1; //If everyone sent 0 finished=1
        }

        if (!finished) // If finished, no need to do it - update again to nobody sent
        {
            obj = search(table, sourceName);

            while (obj->next != NULL)
            {
                obj = obj->next;
                obj->sent = -1;
            }
        }

        pthread_mutex_unlock(&lock);

    }
    pthread_mutex_unlock(&lock);
    obj = search(table, sourceName);

    while (obj != NULL)
    {
        close(obj->socketfd);
        obj = obj->next;
    }

    return NULL;
}

//This function recalculating the weight to each vertex in graph
void relax()
{
    Object *temp, *node, *obj;
    changed = 0;
    int i;

    for (i = 0; i < table->currentSize; i++) //for all vertexes in graph do:
    {
        node = table->arr[i];

        if (node)
        {
            temp = node->next;

            while (temp)
            {
                obj = search(table, temp->vertex);

                if ((obj->weight) > (node->weight + temp->weight)) //'Relax' function- update weight if there cheaper way
                {
                    table->arr[obj->indx]->weight = node->weight + temp->weight;
                    table->arr[obj->indx]->neighbor = node->vertex;
                    changed = 1;
                }

                temp = temp->next;
            }
        }
    }

}

//This function checking for each neighbor if sent 0. return 1 if all sent 0
int check()
{
    Object *obj;
    obj = search(table, sourceName);

    if (obj == NULL)
        printf("something wrong\n");

    obj = obj->next;

    while (obj != NULL)
    {
        if (obj->sent != 0)
            return 0;

        obj = obj->next;
    }

    return 1;

}

//This function checking for each neighbor if sent something. return 1 if all sent
int checkAllSent()
{
    Object *obj;
    obj = search(table, sourceName);

    if (obj == NULL)
        printf("something wrong\n");

    obj = obj->next;

    while (obj != NULL)
    {
        if (obj->sent == -1)
            return 0;

        obj = obj->next;
    }

    return 1;
}


//This function updates the DV for neighbor who sent his dv
void update(int arr[], char *neighbor)
{
    Object *neigh, *temp;
    char *name = "a";
    int found = 0, i;

    neigh = search(table, neighbor);

    if (neigh == NULL)
        printf("something wrong\n");

    for (i = 1; i < num + 1 ; i++)
    {
        found = 0;
        temp = neigh->next;
        name = find(i);

        while (temp != NULL)
        {
            if (strcmp((char *)temp->vertex, name) == 0)
            {
                found = 1;
                temp->weight = arr[i];
            }

            temp = temp->next;
        }

        if (strcmp(neighbor, name) != 0)
        {
            if (found == 0)
            {
                add(table, neighbor, name, arr[i], NULL, NULL, 0);
                add(table, name, neighbor, arr[i], NULL, NULL, 0);
            }
        }
    }

    return;

}

//This function get place in array and need to find the suitable vertex in table
char *find(int n)
{
    int i, k = 1;

    for (i = 0; i < table->currentSize; i++)
    {
        if (table->arr[i] != NULL)
        {
            if (k == n)
                return (char *)table->arr[i]->vertex;

            k++;
        }
    }

    return NULL;
}