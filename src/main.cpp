#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

#include "functions.h"

int main() {
    // Loop through different values of n
    for (int n = 1; n <= 5; n++) {
        // Construct file paths for this iteration
        std::string tests_folder = "../tests/" + std::to_string(n) + "/";
        std::string components_filename = tests_folder + "circuit-" + std::to_string(n) + ".lib";
        std::string circuit_filename = tests_folder + "circuit-" + std::to_string(n) + ".circ";
        std::string stimulus_filename = tests_folder + "circuit-" + std::to_string(n) + ".stim";

        // Parse component information
        std::unordered_map<std::string, Component> components = get_component(components_filename);

        // Parse circuit information
        auto [circuit, inputs] = get_circuit(circuit_filename);
        std::vector<Stimulus> stimuli = get_stimulus(stimulus_filename); 
        evaluate_and_propagate(circuit, components);
        simulate(stimuli, circuit, components, tests_folder);
    }
    return 0;
}