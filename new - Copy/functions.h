#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <stack>
#include <regex>
#include <string>
#include <cassert>
#include <algorithm>
#include "structs.h"
using namespace std;

vector<Stimulus> get_stimulus(string filename);
bool evaluateExpression(const string &output_expression, const vector<pair<string, bool>> &inputs);
unordered_map<string, Component> get_component(string filename);
void evaluate_and_propagate(map<string, circ_comp>& circuit, unordered_map<string, Component>& components, vector<Stimulus>& stimuli);
pair<map<string, circ_comp>, map<string, bool>> get_circuit(string filename);
void simulate(const vector<Stimulus>& stimuli, map<string, circ_comp>& circuit, unordered_map<string, Component>& components);
void test_for_stimlui_error(vector<Stimulus>& stimuli, map<string, bool>& inputs);
void circuit_test(map<string, circ_comp>& circuit, map<string, bool>& inputs, unordered_map<string, Component>& components);
#endif // FUNCTIONS_H
