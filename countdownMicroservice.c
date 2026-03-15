#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "cJSON.h"   

//create and store timers in a linked list to allow for multiple timers to be tracked at once

typedef struct Timer{
    char id[64]; //tracking ID
    long long end_timestamp; //time when the timer should end, in milliseconds since epoch
    struct Timer *next; //next timer in linked list
} Timer;

Timer *timerList = NULL; //head of the linked list

long long get_timestamp() { //return time in milliseconds since epoch
    return (long long)time(NULL) * 1000;
}


//function to add a timer to the linked list
void add_timer(const char *id, long num_seconds) {
    Timer *newTimer = (Timer *)malloc(sizeof(Timer));
    strncpy(newTimer->id, id, sizeof(newTimer->id) - 1);
    newTimer->id[sizeof(newTimer->id) - 1] = '\0'; // Ensure null-termination
    newTimer->end_timestamp = get_timestamp() + num_seconds * 1000; //calculate end timestamp
    newTimer->next = timerList; //add to the front of the list
    timerList = newTimer; //update head of the list
}

void remove_timer(const char *id) { //iterate through the linked list of timers and remove the matching timer when found. 
    Timer *current = timerList;
    Timer *previous = NULL;

    while (current != NULL) {
        if (strcmp(current->id, id) == 0) { //found the timer to remove
            if (previous == NULL) { //removing head of the list
                timerList = current->next;
            } else {
                previous->next = current->next; //bypass the current timer
            }
            free(current); //free memory
            return;
        }
        previous = current;
        current = current->next;
    }
}

char* handle_request(const char* json_input) {
    json_error_t error;

}