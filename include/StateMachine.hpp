// Copyright (C) 2021 Piotr Skorupa

#pragma once

#include <algorithm>
#include <list>
#include <memory>
#include <type_traits>
#include <unordered_map>

// DEFAULT STATE DEFINITION READY TO COPY, UNCOMMENT AND USE !!!
//**********************************************************************/
// struct DefaultData{};
// struct DefaultSate : public scorpion::State<DefaultSate, DefaultData>
// {
//     std::string description() const override
//     {
//         // String for state identification
//         return "DefaultSate";
//     }
//     void beforeTransitionImpl()
//     {
//         // Will be invoked before transition to the next state.
//         // You can for example set data for next state here:
//         // setData(DefaultData{/*data*/});
//     }
//     void afterTransitionImpl()
//     {
//         // Will be invoked after transition to this state
//         // You can for example make some notification here
//     }
//     bool validateImpl(const DefaultData& data)
//     {
//         // Will be invoked during transition to this state.
//         // You can check if data are valid for this state.
//         return true;
//     }
// };
// enum class DefaultAction{ ACTION_1 };
//**********************************************************************/

namespace scorpion
{

template <typename DataPack>
struct IState
{
    virtual ~IState() = default;

    virtual void setData(const DataPack& data) = 0;
    virtual DataPack getData() const = 0;

    virtual std::string description() const = 0; // NEED TO BE OVERRIDE

protected:
    virtual void beforeTransition() = 0;
    virtual void afterTransition() = 0;
    virtual bool validate(const DataPack& data) = 0;

    template <typename T, typename U> friend class StateMachine;
};

template <typename T, typename DataPack>
struct State : public IState<DataPack>
{
    virtual ~State() = default;

    void setData(const DataPack& data)
    {
       data_ = data;
    }

    DataPack getData() const
    {
        return data_;
    }

    bool validate(const DataPack& data) override
    {
        return static_cast<T*>(this)->validateImpl(data);
    }

    void beforeTransition() override
    {
        static_cast<T*>(this)->beforeTransitionImpl();
    }

    void afterTransition() override
    {
        static_cast<T*>(this)->afterTransitionImpl();
    }

protected:
    DataPack data_;
};

struct Transition
{
    constexpr static auto NO{"no"};
    std::string from{NO};
    std::string to{NO};
};

template <typename ActionType, typename DataPack>
class StateMachine
{
public:
    StateMachine(): currentState_(nullptr)
    {}

    void setStartingState(const std::string& stateName) throw()
    {
        if (statesMap_.find(stateName) == statesMap_.end())
        {
            throw std::runtime_error("State must be registered first! Use function: "
                "registerState<StateType>(stateName, args...)");
        }

        currentState_ = statesMap_[stateName];
    }

    template <typename StateType, typename ...Args>
    void registerState(const std::string& stateName, Args... args)
    {
        statesMap_[stateName] = std::make_shared<StateType>(args...);
    }

    void addTransition(const ActionType& action, const std::string& from, const std::string& to)
    {
        transitionMap_[action].push_back(Transition{from, to});
    }

    bool handleAction(const ActionType& action) throw()
    {
        if (not currentState_)
        {
            throw std::runtime_error("Set starting state before handleAction()! "
                "Use function: setStartingState(stateName)");
        }

        auto transitions = transitionMap_[action];
        if (transitions.empty())
        {
            throw std::runtime_error("No transition has been registered for this action!");
        }

        auto currentStatePair = std::find_if(statesMap_.begin(), statesMap_.end(),
            [this](const auto& state) {
                return state.second == currentState_;
            });
        if (currentStatePair == statesMap_.end())
        {
            throw std::runtime_error("Current state has not been registered! State: " + currentState_->description());
        }

        auto transition = std::find_if(transitions.begin(), transitions.end(),
            [&currentStatePair](const auto& transition) {
                return currentStatePair->first == transition.from;
            });

        if (transition == transitions.end())
        {
            throw std::runtime_error("Current state has no registered transition for this action!");
        }

        if ((transition->to == Transition::NO) or (transition->to == transition->from))
        {
            // No transition
            return false;
        }

        currentState_->beforeTransition();
        auto data = currentState_->getData();
        auto nextState = statesMap_[transition->to];
        if (nextState == nullptr)
        {
            throw std::runtime_error("State: " + transition->to + " is not registered!");
        }
        if (not nextState->validate(data))
        {
            throw std::runtime_error("Given data are not valid for state: " + nextState->description());
        }
        nextState->setData(data);
        currentState_ = nextState;
        currentState_->afterTransition();
        return true;
    }

    std::shared_ptr<IState<DataPack>> getCurrentState() const
    {
        if (currentState_ == nullptr)
        {
            throw std::runtime_error("Set starting state before getCurrentState()! "
                "Use function: setStartingState(stateName)");
        }

        return currentState_;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<IState<DataPack>>> statesMap_;
    std::unordered_map<ActionType, std::list<Transition>> transitionMap_;
    std::shared_ptr<IState<DataPack>> currentState_;
};

} // scorpion
