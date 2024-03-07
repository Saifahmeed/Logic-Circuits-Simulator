#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>
using namespace std;
struct Component {
    string name;
    int num_inputs;
    string output_expression;
    int delay_ps;
};
struct CircuitComponent {
    string name;
    string type;
    string output;
    vector<string> inputs;
};
struct Stimulus {
    int time_stamp_ps;
    string input;
    int logic_value;
};
int main() {
    // Hash table to store components from the library file
    unordered_map<string, Component> component_hash;

    // Reading the library file
    ifstream lib_file("library_file.lib");
    if (!lib_file) {
        cerr << "Error: Unable to open library file." << endl;
        return 1;
    }

    string line;
    while (getline(lib_file, line)) {
        stringstream ss(line);
        string name, output_expr;
        int num_inputs, delay;

        if (ss >> name >> num_inputs >> output_expr >> delay) {
            Component component;
            component.name = name;
            component.num_inputs = num_inputs;
            component.output_expression = output_expr;
            component.delay_ps = delay;

            component_hash[name] = component;
        }
    }
    lib_file.close();

    // Hash table to store components from the circuit file
    unordered_map<string, CircuitComponent> circuit_component_hash;

    // Reading the circuit file
    ifstream cir_file("circuit_file.cir");
    if (!cir_file) {
        cerr << "Error: Unable to open circuit file." << endl;
        return 1;
    }

    bool inputs_flag = false;
    while (getline(cir_file, line)) {
        if (line == "INPUTS:") {
            inputs_flag = true;
            continue;
        } else if (line == "COMPONENTS:") {
            inputs_flag = false;
            continue;
        }

        stringstream ss(line);
        string name, type, output;
        vector<string> inputs;

        if (inputs_flag) {
            ss >> name;
        } else {
            ss >> name >> type >> output;
            string input;
            while (ss >> input) {
                inputs.push_back(input);
            }
        }

        CircuitComponent circuit_component;
        circuit_component.name = name;
        if (!inputs_flag) {
            circuit_component.type = type;
            circuit_component.output = output;
            circuit_component.inputs = inputs;
            circuit_component_hash[name] = circuit_component;
        }
    }
    cir_file.close();

    // Reading stimuli file
    vector<Stimulus> stimuli_data;
    ifstream stim_file("stimuli_file.stim");
    if (!stim_file) {
        cerr << "Error: Unable to open stimuli file." << endl;
        return 1;
    }

    while (getline(stim_file, line)) {
        stringstream ss(line);
        Stimulus stimulus;
        char comma;
        if (ss >> stimulus.time_stamp_ps >> comma >> stimulus.input >> comma >> stimulus.logic_value) {
            stimuli_data.push_back(stimulus);
        }
    }
    stim_file.close();
    return 0;
}
