# CS361-Microservice-Countdown-Timer
Microservice for a simple countdown timer. 

To compile, run 
gcc countdownMicroservice.c cJSON.c -o countdownMicroservice -I$(brew --prefix)/include -L$(brew --prefix)/lib -lzmq -lm
in your terminal. 

you need to have zmq downloaded as well as the cJSON already in the repo, so I could only get this to run on local and not the engineering servers. 

run ./timer_service to start the microservice
create a timer:
request:
{
  "action": "start",
  "timer_id": "baking_cake_1",
  "duration_seconds": 600
}


response:
{ "status": "success" }

check timer status:
{
  "action": "status",
  "timer_id": "baking_cake_1"
}

response if running:
{
  "status": "running",
  "remaining_ms": 450000
}

response if finished:
{
  "status": "FINISHED",
  "remaining_ms": 0
}

uses port tcp://*:5555