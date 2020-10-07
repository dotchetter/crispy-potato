#pragma once
#include "StateMachine.h"

StateMachine::StateMachine(fp_t mainMethod, State mainState)
{
    this->mainMethod = mainMethod;
    this->mainState = mainState;
}


const State StateMachine::getMainState()
{
    return this->mainState;
}


const State StateMachine::getCurrentState()
{
    return this->currentState;
}


const State StateMachine::getTransitionalStateForState(State state)
/*
* Get the next state in line, as per defined
* by the given state the machine is currently
* in at the time of calling. The default is 
* the mainState state as a failsafe fallback,
* if loop exhausts without finding a matching
* transition for the current state. 
*/
{
    for (int i = 0; i < this->stateCount; i++)
    {
        if (this->states[i] == state)
        {
            return this->transitionalStates[i];
        }
    }
    return this->mainState;
}


const fp_t StateMachine::getMethodForState(State state)
/*
* Iterate over the states and return the method
* on the same index position as the given state
* in the linked arrays.
*/
{
    for (int i = 0; i < this->stateCount; i++)
    {
        if (states[i] == state)
        {
            return this->methods[i];
        }
    }return this->mainMethod;
}


void StateMachine::release()
/*
* Mutates the instance's currentState to whichever
* state is defined as the transitional state for 
* the state the machine is in at the given time 
* of call.
*/
{
    this->nextState = this->getTransitionalStateForState(this->currentState);
}

void StateMachine::transitionTo(State state)
/*
* Allows methods to mutate the state of the machine
* to transition to another method in the call stack.
* It is however only allowed if the current state is
* the main state, that is, it cannot interrupt an
* ongoing transition between one secondary state and 
* another. 
*/
{
    if (this->currentState == this->mainState)
    {
        this->nextState = state;
    }
}

const fp_t StateMachine::next()
/*
* Allows for a continuous polling method to be called
* which returns the method for the state the machine
* is in at the time of call. The previous state is cached
* as to reduce lookup time and CPU cycles spent on searching,
* if the state from the previous call is left unchanged.
* 
* Protects the application from an infinite loop where
* the state is left unchanged by misbehaving state methods
* which do not call the transition() method. This validation
* is implemented by not allowing the same state to occur twice
* or more in sequence.
*/
{
    
    if (this->currentState == this->nextState)
    {
        this->nextState = this->mainState;
    }

    this->currentState = this->nextState;
    return this->getMethodForState(this->nextState);
}

void StateMachine::addState(fp_t func, State state, State transition)
{
    if (this->stateCount < sizeof(this->methods) / sizeof(this->methods[0]))
    {
        this->methods[this->stateCount] = func;
        this->states[this->stateCount] = state;
        this->transitionalStates[this->stateCount] = transition;
        this->stateCount++;
    }
}
