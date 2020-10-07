#pragma once
#include "States.h"
#include "StateMachine.h"

typedef void (*fp_t)();

class StateMachine
/*
* State machine class.
* Maximum amount of methods mapped: 128
*
* This object allows for continuous
* polymorphic access to state methods in
* a call stack. It aids to automate the
* transition between different function
* calls after a set of pre-determined set
* of states.
*
* The mainState and mainMethod correlate
* to the "idle" state of the device, and
* will automatically be the default yield
* whenever the previous state has exhausted
* and the method getCurrentState is called
* once again. It will remain the result
* until methods with access mutate the state
* through passing an enum instance for the
* desired state. If however the transitional
* state mismatches with the predefined
* transitional state and a method tries to
* switch to an illegal state, this is prohibited
* and automatically reverts to the main state
* and ignores the attempted transition.
*
* For custom transitions to be allowed, e.g.:
*
    * blinkLed -> runMotor
*
* .. instead of the default, which would be:
*
    * blinkLed -> mainState
*
* .. the StateMachine instance must be provided
* with this breadcrum trail upon defining the
* method in the addMethod call.
*
* It is as easy as adding the optional
* "transition" parameter in the function call,
* defining an enum to the desired transitional
* state. For this to be supported, the transitional
* state must be implemented before the state which
* uses it as a transition.
*/
{
private:

    int stateCount;
    State currentState;
    State mainState;
    fp_t mainMethod;    
    fp_t methods[128];
    State transitionalStates[128];
    State states[128];
    State nextState;
    const fp_t getMethodForState(State state);
    const State getTransitionalStateForState(State state);

public:

    StateMachine(fp_t mainMethod, State mainState);
        
    const State getMainState();
    const fp_t next();

    const State getCurrentState();
    void release();
    void transitionTo(State state);
    void addState(fp_t func, State state, State transition=State::IDLE);
};
