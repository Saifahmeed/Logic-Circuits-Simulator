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
using namespace std;

// Structures for representing stimulus, component, and circuit components
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

struct circ_comp {
    string name;
    string type;
    map<string, pair<bool, int>> output; // Change output to a map with simulation time
    vector<pair<string, bool>> inputs;
};
vector < Stimulus > get_stimulus(string filename) {
  vector < Stimulus > stims; // Vector to store parsed stimuli
  ifstream stimfile(filename); // Open file stream for reading stimulus information
  if (stimfile.fail()) { // Check if file opening fails
    cout << "error\n"; // Display error message
  } else {
    string line; // Variable to store each line read from the file
    while (getline(stimfile, line)) { // Read each line from the file
      stringstream currline(line); // Create stringstream to parse the line
      Stimulus stim; // Temporary stimulus object
      int time_stamp_ps; // Timestamp of the stimulus
      string input; // Input signal name
      int logic_value; // Logic value of the input signal
      currline >> time_stamp_ps >> input >> logic_value; // Parse stimulus information
      stim.input = input; // Assign parsed input signal name to the stimulus
      stim.logic_value = logic_value; // Assign parsed logic value to the stimulus
      stim.time_stamp_ps = time_stamp_ps; // Assign parsed timestamp to the stimulus
      stims.push_back(stim); // Store the stimulus in the vector
    }
  }
  return stims; // Return vector containing parsed stimuli
}
// Function to evaluate bitwise expressions
bool evaluateExpression(const string& output_expression, const vector<pair<string, bool>>& inputs) {
    unordered_map<string, bool> inputMap;
    for (size_t i = 0; i < inputs.size(); ++i) {
        inputMap["i" + to_string(i + 1)] = inputs[i].second;
    }

    stack<bool> operands;
    stack<char> operators;

    // Regular expression to match continuous words
    regex word_regex("[A-Za-z0-9]+");
    auto words_begin = sregex_iterator(output_expression.begin(), output_expression.end(), word_regex);
    auto words_end = sregex_iterator();

    string modified_expression = output_expression;
    for (sregex_iterator i = words_begin; i != words_end; ++i) {
        smatch match = *i;
        string word = match.str();

        if (inputMap.find(word) != inputMap.end()) {
            // Replace word with its value
            modified_expression = regex_replace(modified_expression, regex("\\b" + word + "\\b"), inputMap[word] ? "1" : "0");
        } else {
            cerr << "Variable " << word << " not found in inputs." << endl;
            return false;
        }
    }

    for (char c : modified_expression) {
        if (c == '0' || c == '1') {
            operands.push(c - '0'); // Convert char to int
        } else if (c == '&' || c == '|' || c == '~' || c == '(') {
            operators.push(c);
        } else if (c == ')') {
            // Evaluate expression within parentheses
            while (!operators.empty() && operators.top() != '(') {
                char op = operators.top();
                operators.pop();
                if (op == '~') {
                    bool operand = operands.top();
                    operands.pop();
                    operands.push(!operand);
                } else {
                    bool operand2 = operands.top();
                    operands.pop();
                    bool operand1 = operands.top();
                    operands.pop();
                    if (op == '&') {
                        operands.push(operand1 & operand2);
                    } else if (op == '|') {
                        operands.push(operand1 | operand2);
                    }
                }
            }
            if (!operators.empty() && operators.top() == '(') {
                operators.pop(); // Discard the '('
            } else {
                cerr << "Mismatched parentheses in the expression." << endl;
                return false;
            }
        } else {
            cerr << "Invalid character in the expression: " << c << endl;
            return false;
        }
    }

    // Final evaluation
    while (!operators.empty()) {
        char op = operators.top();
        operators.pop();
        if (op == '~') {
            bool operand = operands.top();
            operands.pop();
            operands.push(!operand);
        } else {
            bool operand2 = operands.top();
            operands.pop();
            bool operand1 = operands.top();
            operands.pop();
            if (op == '&') {
                operands.push(operand1 & operand2);
            } else if (op == '|') {
                operands.push(operand1 | operand2);
            }
        }
    }

    if (operands.size() != 1) {
        cerr << "Invalid expression format." << endl;
        return false;
    }

    return operands.top();
}

// Function to parse component information from a file and store it in a map
unordered_map < string, Component > get_component(string filename) {
  unordered_map < string, Component > components;
  ifstream compfile(filename);
  if (compfile.fail()) { // Check if file opening fails
    cout << "error\n";
  } else {
    string line;
    while (getline(compfile, line)) { // Read each line from the file
      stringstream currline(line); // Create stringstream to parse the line
      string name;
      int num_inputs;
      string output_expression;
      int delay_ps;
      Component comp;
      currline >> name >> num_inputs >> output_expression >> delay_ps; // Parse component information
      comp.name = name; // Assign parsed name to the component
      comp.num_inputs = num_inputs; // Assign parsed number of inputs to the component
      comp.output_expression = output_expression; // Assign parsed output expression to the component
      comp.delay_ps = delay_ps; // Assign parsed delay to the component
      components[name] = comp; // Store the component in the map with its name as key
    }
  }
  return components;
}
void evaluate_and_propagate(map<string, circ_comp>& circuit,unordered_map<string, Component>& components) {
    for (auto& comp : circuit) { // Iterate over circuit components
        auto& comp_info = comp.second;
                bool output_value = evaluateExpression(components[comp_info.type].output_expression, comp_info.inputs); // Change to use output_expression
                comp_info.output[comp_info.output.begin()->first].first = output_value; // Update output value of the component

        for (auto& other_comp : circuit) { // Iterate over other circuit components
            auto& other_comp_info = other_comp.second;
            for (auto& input : other_comp_info.inputs) { // Iterate over inputs of other components
                if (input.first == comp_info.output.begin()->first) { // If input matches the output of current component
                    input.second = comp_info.output[comp_info.output.begin()->first].first; // Update input value
                }
            }
        }
    }
}


// Function to parse circuit information from a file and store it in a map
pair<map<string, circ_comp>, map<string, bool>> get_circuit(string filename) {
    map<string, circ_comp> circuit; // Map to store parsed circuit components
    map<string, bool> inputs; // Map to store parsed input signals
    ifstream circuit_file(filename); // Open file stream for reading circuit information
    if (circuit_file.fail()) { // Check if file opening fails
        cout << "Error: Failed to open circuit file." << endl; // Display error message
    } else {
        string line; // Variable to store each line read from the file
        string section; // Variable to determine the section of the file being parsed
        while (getline(circuit_file, line)) { // Read each line from the file
            stringstream currline(line); // Create stringstream to parse the line
            string token; // Token to store parsed values
            currline >> token; // Read first token from the line
            if (token == "INPUTS:") { // Check if current line specifies inputs section
                section = "INPUTS";
            } else if (token == "COMPONENTS:") { // Check if current line specifies components section
                section = "COMPONENTS";
            } else if (section == "INPUTS") { // Parse input signals
                for (char c : line) {
                    if (c != ' ') {
                        inputs[string(1, c)] = false; // Store input signal name with initial logic value as false
                    }
                }
            } else if (section == "COMPONENTS") { // Parse circuit components
                string name, type, output_name;
                map<string, pair<bool, int>> output_map;
                vector<pair<string, bool>> component_inputs;
                name = token; // Component name
                currline >> type; // Component type
                currline >> output_name; // Output signal name
                output_map[output_name] = {false, 0}; // Initialize output signal with initial logic value as false and simulation time as 0
                while (currline >> token) { // Parse input signals of the component
                    token.erase(remove_if(token.begin(), token.end(), [](char c) {
                        return c == ',';
                    }), token.end());
                    component_inputs.push_back({token, false}); // Store input signal with initial logic value as false
                }
                circuit[name] = {name, type, output_map, component_inputs}; // Store parsed component in the map
            }
        }
    }
    return {circuit, inputs}; // Return parsed circuit components and input signals
}
  void simulate(const vector<Stimulus>& stimuli, map<string, circ_comp>& circuit, unordered_map<string, Component>& components) {
      vector<pair<int, string>> output_data; // Vector to store output data

      for (const auto& stim : stimuli) { // Iterate over stimuli
          map<string, pair<bool, int>> temp_outputs; // Temporary map to store current output values
          // Vector to store simulation times for each component
          vector<int> simulation_vector;

          for (auto& comp : circuit) { // Iterate over circuit components
              simulation_vector={0};
              auto& comp_info = comp.second;

              // Find the input signal associated with the stimulus
              for (auto& input : comp_info.inputs) {
                  if (input.first == stim.input) {
                      simulation_vector.push_back(stim.time_stamp_ps);
                      input.second = stim.logic_value;
                  }
                  for (auto& comp : circuit) { // Iterate over circuit components
                  auto& comp_info = comp.second;
                  if (input.first == comp_info.output.begin()->first) {
                      simulation_vector.push_back(comp_info.output.begin()->second.second);
                  }
              }
              }
              int max_simulation_time = *max_element(simulation_vector.begin(), simulation_vector.end());
              // Get the maximum simulation time from the vector
              for (const auto& comp : components) { // Iterate over components
                const Component& component = comp.second;
                if (component.name == comp_info.type) {
              comp_info.output.begin()->second.second = component.delay_ps + max_simulation_time;
                }
          }
          temp_outputs[comp_info.name] = comp_info.output.at(comp_info.output.begin()->first); // Store current output values
            bool output_value = evaluateExpression(components[comp_info.type].output_expression, comp_info.inputs); // Evaluate output expression
          comp_info.output[comp_info.output.begin()->first] = {output_value, comp_info.output[comp_info.output.begin()->first].second}; // Update output value of the component

              for (auto& other_comp : circuit) { // Iterate over other circuit components
                  auto& other_comp_info = other_comp.second;
                  for (auto& input : other_comp_info.inputs) { // Iterate over inputs of other components
                      if (input.first == comp_info.output.begin()->first) { // If input matches the output of current component
                          input.second = output_value; // Update input value
                      }
                  }
              }
              if (temp_outputs.at(comp_info.name).first != comp_info.output.at(comp_info.output.begin()->first).first) { // If output value changed
                  for (const auto& comp : components) { // Iterate over components
                      const Component& component = comp.second;
                    pair<int, string> output_pair = {comp_info.output.begin()->second.second,
                                  to_string(comp_info.output.begin()->second.second) + ", " +
                                  comp_info.output.begin()->first + ", " +
                                  to_string(comp_info.output.begin()->second.first)}; // Access the boolean value
                      // Push the pair into output_data
                      output_data.push_back(output_pair);
                        break; // Stop searching further as component is found
                      }
                  }
          }            
          // Store stimulus data
          output_data.push_back({stim.time_stamp_ps,
                                to_string(stim.time_stamp_ps) + ", " + stim.input + ", " +
                                to_string(stim.logic_value)});
      }
      // Sort output data
      sort(output_data.begin(), output_data.end());
      // Print output data
      for (const auto& data : output_data) {
          cout << data.second << endl;
      }
  }

int main() {
    string components_filename = "circuit-1.lib"; // Components file name
    unordered_map<string, Component> components = get_component(components_filename); // Parse component information

    string filename = "circuit-1.circ"; // Circuit file name
    auto[circuit, inputs] = get_circuit(filename); // Parse circuit information
    evaluate_and_propagate(circuit, components); // Evaluate and propagate circuit signals

    cout << "Circuit Components:" << endl; // Display circuit components
    for (const auto& comp : circuit    ) { // Iterate over circuit components
        const auto& comp_info = comp.second;
        cout << "Name: " << comp_info.name << ", Type: " << comp_info.type <<
             ", Output: " << comp_info.output.begin()->first << " (" << comp_info.output.begin()->second.first << ")" <<
             ", Inputs: ";
        for (const auto& input : comp_info.inputs) { // Iterate over inputs of the component
            cout << input.first << " (" << input.second << ") ";
        }
        cout << endl;
    }

    string stimulus_filename = "circuit-1.stim"; // Stimulus file name
    vector<Stimulus> stimuli = get_stimulus(stimulus_filename); // Parse stimulus information


    simulate(stimuli, circuit, components); // Simulate the circuit

    return 0;
}
