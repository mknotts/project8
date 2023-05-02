#include<unistd.h>
#include <stdio.h>

#include "server_functions.h"

int datastore[NUMKEYS];

void idle(int time){
    sleep(time);
}

int get(int key){
    printf("getting index %d\n", key);
    if(key > NUMKEYS || key < 0){
        return -1;
    }
    return datastore[key];
}

int put(int key, int value){
    printf("adding %d to index %d\n", value, key);
    if(key > NUMKEYS || key < 0){
        return -1;
    }
    datastore[key] = value;
    return 0;
}