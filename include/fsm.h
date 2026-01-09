#ifndef FSM_H
#define FSM_H

enum State
{
    IDLE,
    SCAN,
    PROCESS
};

extern State currentState;

void setupFSM();
void runFSM();

#endif // !FSM_H