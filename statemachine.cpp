/*
 * =====================================================================================
 *
 *       Filename:  statemachine.cpp
 *
 *    Description:  Trial state machine implementation
 *
 *        Version:  1.0
 *        Created:  04/05/2018 08:17:02
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Andrew Epstein
 *
 * =====================================================================================
 */

#include <cstdlib>
#include <unordered_map>
#include <set>
#include <vector>
#include <string>

using std::string;
using std::unordered_map;
using std::vector;
using std::pair;
using std::make_pair;

class State {
	private:
		string _name;
	public:
		State( string name) : _name( name ) { };
		string getName() {
			return _name;
		}
};

class Action {
	private:
		uint8_t _character;
	public:
		Action( uint8_t character ) : _character( character ) { };
		uint8_t getCharacter() {
			return _character;
		}

};

class StateMachine {
	private:
		unordered_map<pair<State, Action>, State> stateMap;
		vector<State> stateList;
	public:
		StateMachine() { };
		void addTransition( State previousState, Action a, State newState ) {
			auto x = make_pair( previousState, a );
			stateMap.insert( make_pair<x, newState> );
		}

};

int main() {
	return 0;
}

