// Copyright (C) 2021 Piotr Skorupa

#include "StateMachine.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <functional>
#include <iostream>
#include <vector>

struct DeathHelper
{
    // EXPECT_DEATH is not catching properly some exceptions (???)
    // Error message: {Caught std::exception-derived exception escaping the death test statement.}
    // When exception occurs, then death is forced by std::abort()
    DeathHelper(const std::function<void()>& throwable) try
    {
        throwable();
    }
    catch(const std::exception& e)
    {
        std::abort();
    }
};

struct TestData {};

struct TestState1 : public scorpion::State<TestState1, TestData>
{
    std::string description() const override { return "TestState1"; }
    void beforeTransitionImpl() {}
    void afterTransitionImpl() {}
    bool validateImpl(const TestData& data) { return true; }
};

struct TestState2 : public scorpion::State<TestState1, TestData>
{
    std::string description() const override { return "TestState2"; }
    void beforeTransitionImpl() {}
    void afterTransitionImpl() {}
    bool validateImpl(const TestData& data) { return true; }
};

struct TestState3 : public scorpion::State<TestState1, TestData>
{
    std::string description() const override { return "TestState3"; }
    void beforeTransitionImpl() {}
    void afterTransitionImpl() {}
    bool validateImpl(const TestData& data) { return true; }
};

struct TestState4 : public scorpion::State<TestState1, TestData>
{
    std::string description() const override { return "TestState4"; }
    void beforeTransitionImpl() {}
    void afterTransitionImpl() {}
    bool validateImpl(const TestData& data) { return true; }
};

enum class TestAction
{
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE
};

class StateMachineFixture : public ::testing::Test
{
public:
    StateMachineFixture()
    {
        sut_ = std::make_unique<scorpion::StateMachine<TestAction, TestData>>();
    }

    void registerStates()
    {
        sut_->registerState<TestState1>("teststate1");
        sut_->registerState<TestState2>("teststate2");
        sut_->registerState<TestState3>("teststate3");
        sut_->registerState<TestState4>("teststate4");
    }

    void setStartingState(const char* stateName)
    {
        registerStates();
        sut_->setStartingState(stateName);
    }

    void setTransitions()
    {
        sut_->addTransition(TestAction::ONE, "teststate1", "teststate2");
        sut_->addTransition(TestAction::ONE, "teststate2", "teststate3");
        sut_->addTransition(TestAction::ONE, "teststate3", "teststate4");
        sut_->addTransition(TestAction::ONE, "teststate4", "teststate1");

        sut_->addTransition(TestAction::TWO, "teststate1", scorpion::Transition::NO);
        sut_->addTransition(TestAction::TWO, "teststate2", scorpion::Transition::NO);
        sut_->addTransition(TestAction::TWO, "teststate3", scorpion::Transition::NO);
        sut_->addTransition(TestAction::TWO, "teststate4", scorpion::Transition::NO);

        sut_->addTransition(TestAction::THREE, "teststate1", "teststate2");
        sut_->addTransition(TestAction::THREE, "teststate2", "teststate1");
        sut_->addTransition(TestAction::THREE, "teststate3", "teststate2");
        sut_->addTransition(TestAction::THREE, "teststate4", "teststate1");

        sut_->addTransition(TestAction::FOUR, "teststate1", "teststate3");
        sut_->addTransition(TestAction::FOUR, "teststate2", "teststate4");
        sut_->addTransition(TestAction::FOUR, "teststate3", "teststate4");
        sut_->addTransition(TestAction::FOUR, "teststate4", "teststate2");

        sut_->addTransition(TestAction::FIVE, "teststate1", "teststate1"); // No transition
        // INVALID TRANSITIONS
        sut_->addTransition(TestAction::FIVE, "teststate2", "teststate434324");
        // Not added
        // sut_->addTransition(TestAction::FIVE, "teststate3", "teststate4");
        // sut_->addTransition(TestAction::FIVE, "teststate4", "teststate2");

    }

    void expectTransition(const TestAction& action, const std::string& stateDescription)
    {
        sut_->handleAction(action);
        EXPECT_EQ(sut_->getCurrentState()->description(), stateDescription);
    }

    void expectTransitions(const std::vector<TestAction>& actions,
        const std::vector<std::string>& statesDescriptions)
    {
        auto expectedStateDescription = statesDescriptions.begin();
        for (const auto & action : actions)
        {
            sut_->handleAction(action);
            EXPECT_EQ(sut_->getCurrentState()->description(), *(expectedStateDescription++));
        }
    }

protected:
    std::unique_ptr<scorpion::StateMachine<TestAction, TestData>> sut_;
};

TEST_F(StateMachineFixture, SetStartingStateWithNoRegisteredStates_DeathOnUnhandledException)
{
    EXPECT_DEATH(sut_->setStartingState("teststate1"), "")
        << "Should be terminated on unhandled exception";
}

TEST_F(StateMachineFixture, HandleActionWithNoStartingState_DeathOnUnhandledException)
{
    EXPECT_DEATH(sut_->handleAction(TestAction::ONE), "")
        << "Should be terminated on unhandled exception";
}

TEST_F(StateMachineFixture, GetCurrentStateWithWithNoStartingState_DeathOnUnhandledException)
{
    EXPECT_DEATH(DeathHelper([this](){ sut_->getCurrentState(); }), "")
        << "Should be terminated on unhandled exception";
}

TEST_F(StateMachineFixture, SetStartingState_SUCCESS)
{
    setStartingState("teststate1");

    EXPECT_EQ(sut_->getCurrentState()->description(), TestState1{}.description());
}

TEST_F(StateMachineFixture, TestTransitions_StartingTestState1)
{
    setStartingState("teststate1");
    setTransitions();

    expectTransitions(
        {TestAction::ONE, TestAction::ONE, TestAction::ONE, TestAction::ONE},
        {
            TestState2{}.description(),
            TestState3{}.description(),
            TestState4{}.description(),
            TestState1{}.description()
        });

    expectTransitions(
        {TestAction::TWO, TestAction::TWO, TestAction::TWO, TestAction::TWO},
        {
            // No transitions on action TWO
            TestState1{}.description(),
            TestState1{}.description(),
            TestState1{}.description(),
            TestState1{}.description()
        });

    expectTransitions(
        {TestAction::THREE, TestAction::THREE, TestAction::THREE, TestAction::THREE},
        {
            TestState2{}.description(),
            TestState1{}.description(),
            TestState2{}.description(),
            TestState1{}.description()
        });

    expectTransitions(
        {TestAction::FOUR, TestAction::FOUR, TestAction::FOUR, TestAction::FOUR},
        {
            TestState3{}.description(),
            TestState4{}.description(),
            TestState2{}.description(),
            TestState4{}.description()
        });

    expectTransitions(
        {TestAction::ONE, TestAction::TWO, TestAction::THREE, TestAction::FOUR},
        {
            TestState1{}.description(),
            TestState1{}.description(), // Action TWO - no transition
            TestState2{}.description(),
            TestState4{}.description()
        });
}

TEST_F(StateMachineFixture, TestNoTransitions_StartingTestState1)
{
    setStartingState("teststate1");
    setTransitions();

    expectTransitions(
        {TestAction::TWO, TestAction::FIVE},
        {
            TestState1{}.description(),
            TestState1{}.description(),
        });
}

TEST_F(StateMachineFixture, TestNotAddedTransitions_DeathOnUnhandledException)
{
    setStartingState("teststate3");
    setTransitions();

    EXPECT_DEATH(sut_->handleAction(TestAction::FIVE), "")
        << "Should be terminated on unhandled exception";
}

TEST_F(StateMachineFixture, TestNotExistingStateAddedToTransitions_DeathOnUnhandledException)
{
    setStartingState("teststate2");
    setTransitions();

    EXPECT_DEATH(sut_->handleAction(TestAction::FIVE), "")
        << "Should be terminated on unhandled exception";
}
