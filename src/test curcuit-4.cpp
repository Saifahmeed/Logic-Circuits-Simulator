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

#include "Testing.cpp" // Include the implementation file for testing
using namespace std;
// Unit test for parsing stimuli
void test_parse_stimuli() {
    vector<Stimulus> stimuli = get_stimulus("../tests/4/circuit-4.stim");

    assert(stimuli.size() == 3);
    assert(stimuli[0].time_stamp_ps == 100 && stimuli[0].input == "A" && stimuli[0].logic_value == 1);
    assert(stimuli[1].time_stamp_ps == 200 && stimuli[1].input == "B" && stimuli[1].logic_value == 1);
    assert(stimuli[2].time_stamp_ps == 300 && stimuli[2].input == "A" && stimuli[2].logic_value == 0);

    std::cout << "Stimuli parsing test passed!" << std::endl;
}

// Unit test for evaluating expressions
void test_evaluate_expression() {
    vector<pair<string, bool>> inputs = {{"i0", true}, {"i1", false}};
    assert(evaluateExpression("~(i0&i1)", inputs) == true);

    inputs = {{"i0", false}, {"i1", false}};
    assert(evaluateExpression("~(i0&i1)", inputs) == true);

    inputs = {{"i0", true}, {"i1", true}};
    assert(evaluateExpression("~(i0&i1)", inputs) == false);

    std::cout << "Expression evaluation test passed!" << std::endl;
}


// Unit test for parsing components
void test_parse_components() {
    unordered_map<string, Component> components = get_component("../tests/4/circuit-4.lib");

assert(components.size() == 6);
assert(components["OR2"].name == "OR2" && components["OR2"].num_inputs == 2 && components["OR2"].output_expression == "i0|i1" && components["OR2"].delay_ps == 50);
assert(components["NAND2"].name == "NAND2" && components["NAND2"].num_inputs == 2 && components["NAND2"].output_expression == "~(i0&i1)" && components["NAND2"].delay_ps == 900);
assert(components["AND2"].name == "AND2" && components["AND2"].num_inputs == 2 && components["AND2"].output_expression == "i0&i1" && components["AND2"].delay_ps == 30);
assert(components["XOR2"].name == "XOR2" && components["XOR2"].num_inputs == 2 && components["XOR2"].output_expression == "(~i0&i1)|(i0&~i1)" && components["XOR2"].delay_ps == 25);
assert(components["NOR2"].name == "NOR2" && components["NOR2"].num_inputs == 2 && components["NOR2"].output_expression == "~(i0|i1)" && components["NOR2"].delay_ps == 35);
assert(components["NOT"].name == "NOT" && components["NOT"].num_inputs == 1 && components["NOT"].output_expression == "~i1" && components["NOT"].delay_ps == 75);

    std::cout << "Components parsing test passed!" << std::endl;
}
bool compareFiles(const string& filename1, const string& filename2) {
    ifstream file1(filename1);
    ifstream file2(filename2);

    if (!file1.is_open() || !file2.is_open()) {
        cerr << "Error: Unable to open files." << endl;
        return false;
    }

    string line1, line2;
    while (getline(file1, line1) && getline(file2, line2)) {
        if (line1 != line2) {
            cerr << "Files are different." << endl;
            return false;
        }
    }

    if (file1.eof() != file2.eof()) {
        cerr << "Files have different sizes." << endl;
        return false;
    }

    cout << "Files are identical." << endl;
    return true;
}

// Test function to compare simulation output file with expected file
void test_simulation_output() {
    string simulatedFile = "simulation.sim";
    string expectedFile = "../tests/4/circuit-4.sim";

    // Assert that both files are identical
    assert(compareFiles(simulatedFile, expectedFile));
}

int main() {
    // Run unit tests
    test_parse_stimuli();
    test_evaluate_expression();
    test_parse_components();

    // Parse component information
    unordered_map<string, Component> components = get_component("../tests/4/circuit-4.lib");

    // Parse circuit information
    auto [circuit_map, inputs] = get_circuit("../tests/4/circuit-4.circ");

    // Adjust types of circuit and components accordingly
    map<string, circ_comp> circuit = circuit_map;

    // Evaluate and propagate
    evaluate_and_propagate(circuit, components);

    // Get stimuli
    vector<Stimulus> stimuli = get_stimulus("../tests/4/circuit-4.stim");

    // Simulate
    simulate(stimuli, circuit, components, "./");
    test_simulation_output();

    return 0;
}