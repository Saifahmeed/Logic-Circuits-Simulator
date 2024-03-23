#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

#include "structs.h"

// Function prototypes
std::vector<Stimulus> get_stimulus(std::string filename);
bool evaluateExpression(const std::string& output_expression, const std::vector<std::pair<std::string, bool>>& inputs);
std::unordered_map<std::string, Component> get_component(std::string filename);
void evaluate_and_propagate(std::map<std::string, circ_comp>& circuit, std::unordered_map<std::string, Component>& components);
std::pair<std::map<std::string, circ_comp>, std::map<std::string, bool>> get_circuit(std::string filename);
void simulate(const std::vector<Stimulus>& stimuli, std::map<std::string, circ_comp>& circuit, std::unordered_map<std::string, Component>& components, const std::string& output_dir);

#endif // FUNCTIONS_H
