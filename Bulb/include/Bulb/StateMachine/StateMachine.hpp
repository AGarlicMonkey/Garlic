#pragma once

namespace blb {
	template<typename T>
	std::vector<T*> toRawVector(const std::vector<std::unique_ptr<T>>& vec){
		std::vector<T*> out;
		for(auto& elem : vec) {
			out.push_back(elem.get());
		}
		return out;
	}

	class Transition;
	class State;

	class Action { //TODO: Have the action class as a template?
		//FUNCTIONS
	public:
	};

	class Condition { //TODO: Have the condition as a template? this way lambdas can be used?
		//FUNCTIONS
	public:
		virtual ~Condition() = 0;

		virtual bool test() const = 0;
	};

	class State {
		//VARIABLES
	private:
		//TODO: custom deleter for unique ptrs so allocators can be used
		std::vector<std::unique_ptr<Action>> entryActions;
		std::vector<std::unique_ptr<Action>> actions;
		std::vector<std::unique_ptr<Action>> exitActions;

		std::vector<std::unique_ptr<Transition>> transitions;

		//FUNCTIONS
	public:
		//TODO: Ctors

		std::vector<Action*> getEntryActions() {
			return toRawVector(entryActions);
		}
		std::vector<Action*> getActions() {
			return toRawVector(actions);
		}
		std::vector<Action*> getExitActions() {
			return toRawVector(exitActions);
		}

		std::vector<Transition*> getTransitions() {
			return toRawVector(transitions);
		}
	};

	class Transition {
		//VARIABLES
	private:
		//TODO: custom deleter for unique ptrs so allocators can be used
		std::unique_ptr<Condition> condition;
		std::unique_ptr<State> state;
		std::vector<std::unique_ptr<Action>> actions;

		//FUNCTIONS
	public:
		//TODO: Ctors

		bool isTriggered() const {
			condition->test();
		}

		State* getState() const {
			return state.get();
		}

		std::vector<Action*> getActions() const {
			return toRawVector(actions);
		}
	};

	class StateMachine {
		//VARIABLES
	private:
		State* currentState = nullptr; //TODO: have a way to set the initial state

		//FUNCTIONS
	public:
		//TODO: Ctors

		std::vector<Action*> update() {
			Transition* triggeredTransition = nullptr;

			for(auto* transition : currentState->getTransitions()) {
				if(transition->isTriggered()) {
					triggeredTransition = transition;
					break;
				}
			}

			std::vector<Action*> actions;
			if(triggeredTransition != nullptr) {
				State* targetState = triggeredTransition->getState();
				
				actions.insert(actions.end(), currentState->getExitActions().begin(), currentState->getExitActions().end());
				actions.insert(actions.end(), triggeredTransition->getActions().begin(), triggeredTransition->getActions().end());
				actions.insert(actions.end(), targetState->getEntryActions().begin(), targetState->getEntryActions().end());

				currentState = targetState;
			} else {
				actions = currentState->getActions();
			}

			return actions;
		}
	};
}