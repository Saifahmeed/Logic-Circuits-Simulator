#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm> 
using namespace std;
struct Stimulus {
    int time_stamp_ps;
    string input;
    int logic_value;
};
struct Component {
    string name;
    int num_inputs;
    string output_expression;
    int delay_ps;
};
unordered_map<string, Component> get_component(string filename){
    unordered_map<string, Component> components;
    ifstream compfile(filename);
    if(compfile.fail()){
        cout << "error\n";
    }
    else{
        string line;
        while(getline(compfile,line)){
            stringstream currline(line);
            string name;
            int num_inputs;
            string output_expression;
            int delay_ps;
            Component comp;
            currline >> name >> num_inputs >> output_expression >> delay_ps;
            comp.name = name;
            comp.num_inputs = num_inputs;
            comp.output_expression = output_expression;
            comp.delay_ps = delay_ps;
            components[name] = comp;
        }
    }
    return components;
}
bool AND(const vector<bool>& ins) {
    bool output = 1;
    for (bool i : ins) {
        output = output && i;
    }
    return output;
}
bool OR(const vector<bool>& ins) {
    bool output = 0;
    for (bool i : ins) {
        output = output || i;
    }
    return output;
}
bool NAND(const vector<bool>& ins) {
    return !AND(ins);
}
bool NOT(const vector<bool>& ins) {
    return !ins[0];
}
bool evaluate_output_expression(const string& gate_type, const vector<bool>& inputs) {
    // Select the appropriate logic function based on the gate_type
    if (gate_type == "AND") {
        return AND(inputs);
    } else if (gate_type == "OR") {
        return OR(inputs);
    } else if (gate_type == "NAND") {
        return NAND(inputs);
    } else if (gate_type == "NOT") {
        return NOT(inputs);
    }
    return false;
}
struct circ_comp {
    string name;
    string type;
    string output;
    vector<string> inputs;
};
map<string, circ_comp> get_circuit(string filename){
    map<string, circ_comp> circuit;
    ifstream circuit_file(filename);
    if(circuit_file.fail()){
        cout << "Error: Failed to open circuit file." << endl;
    }
    else{
        string line;
        string section;
        while(getline(circuit_file, line)){
            stringstream currline(line);
            string token;
            currline >> token;
            if(token == "INPUTS:") {
                section = "INPUTS";
            } else if (token == "COMPONENTS:") {
                section = "COMPONENTS";
            } else if (section == "INPUTS") {
                // Read circuit inputs
                while (currline >> token) {
                    circuit[token] = {token, "INPUT", token, {}};
                }
            } else if (section == "COMPONENTS") {
                string name, type, output;
                vector<string> inputs;
                name = token;
                currline >> token;
                type = token;
                currline >> token;
                output = token;
                while (currline >> token) {
                    token.erase(remove_if(token.begin(), token.end(), [](char c) { return c == ','; }), token.end());
                    inputs.push_back(token);
                }
                circuit[name] = {name, type, output, inputs};
            }
        }
    }
    return circuit;
}
vector<Stimulus> get_stimulus(string filename){
    vector<Stimulus> stims;
    ifstream stimfile(filename);
    if(stimfile.fail()){
        cout << "error\n";
    }
    else{
        string line;
        while(getline(stimfile,line)){
            stringstream currline(line);
            Stimulus stim;
            int time_stamp_ps;
            string input;
            int logic_value;
            currline >> time_stamp_ps >> input >> logic_value ;
            stim.input = input;
            stim.logic_value = logic_value;
            stim.time_stamp_ps =time_stamp_ps;
            stims.push_back(stim);
        }
    }
    return stims;
}
void simulation(unordered_map<string, Component>& components, map<string, circ_comp>& circuit, vector<Stimulus>& stimuli) {
    unordered_map<string, vector<bool>> inputs;
    // Initialize inputs to 0 for all components in the circuit
    for (const auto& comp : circuit) {
        inputs[comp.first] = vector<bool>(1, false); 
    }
    
    // Initialize outputs map
    unordered_map<string, bool> outputs;
    for (auto& circ : circuit) {
        circ_comp comp = circ.second;
        outputs[comp.name] = evaluate_output_expression(comp.type, inputs[comp.name]);
    }
    // Process stimuli
for (const auto& stim : stimuli) {
    // Update input value
    inputs[stim.input].push_back(stim.logic_value);
    cout << stim.time_stamp_ps << ", " << stim.input << ", " << stim.logic_value << endl;
    for (auto& circ : circuit) {
        circ_comp comp = circ.second;
        if (inputs.find(comp.name) == inputs.end()) {
            cerr << "Input '" << comp.name << "' not found in the circuit." << endl;
            continue; // Skip to the next component
        }
        inputs[comp.name].push_back(stim.logic_value); // Update input value
        bool new_output = evaluate_output_expression(comp.type, inputs[comp.name]);
        cout << "Old output for " << comp.name << ": " << outputs[comp.name] << ", New output: " << new_output << endl;
        if (new_output != outputs[comp.name]) {
            outputs[comp.name] = new_output;
            // Print changed output
            cout << stim.time_stamp_ps << ", " << comp.name << ", " << outputs[comp.name] << endl;
            }
        }
    }
}
int main(){
    string filename = "circuit-1.circ";
    map<string, circ_comp> circuit = get_circuit(filename); 
    // Display circuit components
    cout << "Circuit Components:" << endl;
    for (auto& comp : circuit) {
        cout << "Name: " << comp.second.name << ", Type: " << comp.second.type << ", Output: " << comp.second.output << ", Inputs: ";
        for (const auto& input : comp.second.inputs) {
            cout << input << " ";
        }
        cout << endl;
    }
    // Read stimuli from file
    string stimulus_filename = "circuit-1.stim";
    vector<Stimulus> stimuli = get_stimulus(stimulus_filename);
    cout << "\nStimuli:" << endl;
    for (const auto& stim : stimuli) {
        cout << "Time: " << stim.time_stamp_ps << ", Input: " << stim.input << ", Logic Value: " << stim.logic_value << endl;
    }
    string components_filename = "circuit-1.lib";
    unordered_map<string, Component> components = get_component(components_filename);
    cout << "Components:" << endl;
    for (const auto& comp : components) {
        const Component& component = comp.second;
        cout << "Name: " << component.name << ", Num Inputs: " << component.num_inputs 
             << ", Output Expression: " << component.output_expression << ", Delay (ps): " << component.delay_ps << endl;
    }
    simulation(components, circuit, stimuli);
    return 0;
}
//          for(int i = 1 ; i <= 5 ; i++){
//         char filenum = i + 48;
//         string filename = "components";
//         filename += filenum;
//         filename += ".txt";
//         comp.push_back(get_component(filename));
//     }
