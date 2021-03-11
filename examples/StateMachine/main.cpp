// Copyright (C) 2021 Piotr Skorupa

#include "StateMachine.hpp"

#include <iostream>
#include <list>

// StateMachine is header only library.
// So you can just move StateMachine.hpp to your project and start using it!

// In this exmaple I present you simple state machine with few states and actions
// Our state machine will be WASHING machine with WASHING and DRYING functions.

// First you need to implement your states and specify what data you want to share between them.
// If you do not need to share any data between states, just specify empty struct
struct WashingData
{
    bool isWaterInDrum{false};
};

constexpr auto IDLE = "IDLE";
constexpr auto WASHING = "WASHING";
constexpr auto DRYING = "DRYING";

// Each state needs to inherit from scorpion::State. As template args put this state(CRTP) and data struct
struct Idle : public scorpion::State<Idle, WashingData>
{
    // Each state needs to define below functions
    std::string description() const override { return IDLE; }
    void beforeTransitionImpl() { /*Will be invoked before transition to the next state.*/}
    void afterTransitionImpl() { /*Will be invoked after transition to this state*/}
    bool validateImpl(const WashingData& data)
    {
        // Will be invoked during transition to this state, to check if given data will be valid
        return data.isWaterInDrum == false;
    }
};

struct Washing : public scorpion::State<Washing, WashingData>
{
    Washing(int /* x */) { /*Constructor with int, just for example */ }
    std::string description() const override { return WASHING; }
    void beforeTransitionImpl()
    {
        // After WASHING set water in drum to false
        setData(WashingData{false});
    }
    void afterTransitionImpl()
    {
        // Before WASHING set water in drum to true
        setData(WashingData{true});
    }
    bool validateImpl(const WashingData& data) { return true; }
};

struct Drying : public scorpion::State<Drying, WashingData>
{
    std::string description() const override { return DRYING; }
    void beforeTransitionImpl() {}
    void afterTransitionImpl() {}
    bool validateImpl(const WashingData& data) { return data.isWaterInDrum == false; }
};

// Define your actions
enum class WashingAction
{
    BUTTON_WASH_PRESSED,
    BUTTON_DRY_PRESSED,
    BUTTON_FINISH_PRESSED
};

int main()
{
    scorpion::StateMachine<WashingAction, WashingData> washingMachine;

    // Register all states
    washingMachine.registerState<Idle>(IDLE);
    washingMachine.registerState<Washing>(WASHING, 2); // pass args for constructor after coma
    washingMachine.registerState<Drying>(DRYING);

    // After registration, set starting state
    washingMachine.setStartingState(IDLE);

    // Now you must add all possible transitions -> function: addTransition(OnAction, From, To)
    // If there is no transition it also should be mentioned -> use: Transition::NO
    using scorpion::Transition;
    washingMachine.addTransition(WashingAction::BUTTON_WASH_PRESSED, IDLE, WASHING);
    washingMachine.addTransition(WashingAction::BUTTON_WASH_PRESSED, WASHING, Transition::NO);
    washingMachine.addTransition(WashingAction::BUTTON_WASH_PRESSED, DRYING, Transition::NO);

    washingMachine.addTransition(WashingAction::BUTTON_DRY_PRESSED, IDLE, DRYING);
    washingMachine.addTransition(WashingAction::BUTTON_DRY_PRESSED, WASHING, Transition::NO);
    washingMachine.addTransition(WashingAction::BUTTON_DRY_PRESSED, DRYING, Transition::NO);

    washingMachine.addTransition(WashingAction::BUTTON_FINISH_PRESSED, IDLE, Transition::NO);
    washingMachine.addTransition(WashingAction::BUTTON_FINISH_PRESSED, WASHING, IDLE);
    washingMachine.addTransition(WashingAction::BUTTON_FINISH_PRESSED, DRYING, IDLE);

    // Action queue for presentation of actions handling
    std::list<WashingAction> actionQueue =
        {
            WashingAction::BUTTON_WASH_PRESSED,
            WashingAction::BUTTON_FINISH_PRESSED,
            WashingAction::BUTTON_DRY_PRESSED,
            WashingAction::BUTTON_FINISH_PRESSED
        };

    std::cout << "Washing machine starting with state: " <<
                washingMachine.getCurrentState()->description() << std::endl;

    for (const auto & action : actionQueue)
    {
        try
        {
            washingMachine.handleAction(action);

            std::cout << "Washing machine is in state: " <<
                washingMachine.getCurrentState()->description() << std::endl;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}