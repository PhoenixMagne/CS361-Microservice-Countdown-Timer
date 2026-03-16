#include <zmq.h> //if this shows as an error, make sure to use the compile script as noted in the readme and have zmq downloaded.
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

// Logic to process the incoming ZeroMQ message
char* handle_request(const char* json_input) {
    cJSON *root = cJSON_Parse(json_input); // Parse the raw JSON string
    if (!root) return strdup("{\"status\":\"error\",\"msg\":\"Invalid JSON\"}");

    cJSON *response = cJSON_CreateObject(); // Prepare response object
    cJSON *action = cJSON_GetObjectItem(root, "action"); // Get the command
    cJSON *timer_id = cJSON_GetObjectItem(root, "timer_id"); // Get the ID

    if (action && strcmp(action->valuestring, "start") == 0) {
        // --- START LOGIC ---
        cJSON *duration = cJSON_GetObjectItem(root, "duration_seconds");
        // long long end_time = get_timestamp() + (duration->valueint * 1000);
        add_timer(timer_id->valuestring, (long)duration->valueint); // Add timer to linked list
        
        cJSON_AddStringToObject(response, "status", "success");
       // cJSON_AddNumberToObject(response, "end_timestamp", (double)end_time);

    } else if (action && strcmp(action->valuestring, "status") == 0) {
        // status logic
        Timer *curr = timerList;
        int found = 0;
        while (curr) {
            if (strcmp(curr->id, timer_id->valuestring) == 0) {
                long long remaining = curr->end_timestamp - get_timestamp();
                // If remaining is 0 or less, it's done
                cJSON_AddStringToObject(response, "status", remaining <= 0 ? "FINISHED" : "running");
                cJSON_AddNumberToObject(response, "remaining_ms", (double)(remaining > 0 ? remaining : 0));
                found = 1;
                break;
            }
            curr = curr->next;
        }
        if (!found) cJSON_AddStringToObject(response, "status", "not_found");
    }

    char *out = cJSON_PrintUnformatted(response); // Convert response to string
    cJSON_Delete(root);     // Clean up memory
    cJSON_Delete(response); // Clean up memory
    return out; // Return string (to be freed in main)
}

int main() {
    void *context = zmq_ctx_new(); // Init ZeroMQ context
    void *responder = zmq_socket(context, ZMQ_REP); // Create Reply socket
    zmq_bind(responder, "tcp://*:5555"); // Listen on port 5555

    printf("Timer Microservice started on port 5555 (Using cJSON)...\n");

    while (1) {
        char buffer[1024]; // Buffer for incoming message
        int bytes = zmq_recv(responder, buffer, 1024, 0); // Block for request
        if (bytes < 0) continue; 
        buffer[bytes] = '\0'; // Null-terminate received string

        char *reply = handle_request(buffer); // Get JSON reply string
        zmq_send(responder, reply, strlen(reply), 0); // Send it back
        free(reply); // Clean up the string returned by cJSON_Print
    }

    return 0;
}