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
#include "functions.h"
#include "structs.h"

vector<Stimulus> get_stimulus(string filename) {
    vector<Stimulus> stims; // Vector to store parsed stimuli
    ifstream stimfile(filename); // Open file stream for reading stimulus information
    if (stimfile.fail()) { // Check if file opening fails
        cout << "error\n"; // Display error message
    } else {
        string line; // Variable to store each line read from the file
        while (getline(stimfile, line)) { // Read each line from the file
            // Remove commas from the line
            for (char& c : line) {
                if (c == ',') c = ' '; // Replace comma with space
            }

            stringstream currline(line); // Create stringstream to parse the line
            Stimulus stim; // Temporary stimulus object
            string time_stamp_str, input, logic_value_str; // Strings to store parsed values
            if (currline >> time_stamp_str >> input >> logic_value_str) {
                stim.time_stamp_ps = stoi(time_stamp_str);
                stim.logic_value = stoi(logic_value_str);
                stim.input = input;
                stims.push_back(stim); // Store the stimulus in the vector
            } else {
                cerr << "Invalid line format: " << line << endl;
            }
        }
    }
    return stims; // Return vector containing parsed stimuli
}

// Function to evaluate bitwise expressions
bool evaluateExpression(const string &output_expression, const vector<pair<string, bool>> &inputs) {
    // Remove spaces from the expression
    string modified_expression;
    for (char c : output_expression) {
        if (c != ' ') {
            modified_expression += c;
        }
    }

    unordered_map<string, bool> inputMap;
    for (size_t i = 0; i < inputs.size(); ++i) {
        inputMap["i" + to_string(i + 1)] = inputs[i].second;
    }

    stack<bool> operands;
    stack<char> operators;

    // Regular expression to match continuous words
    regex word_regex("[A-Za-z0-9]+");
    auto words_begin = sregex_iterator(modified_expression.begin(), modified_expression.end(), word_regex);
    auto words_end = sregex_iterator();

    for (size_t i = 0; i < inputs.size(); ++i) {
        inputMap["i" + to_string(i + 1)] = inputs[i].second;
    }

    // Replace variables with their values
    for (auto& pair : inputMap) {
        size_t pos = modified_expression.find(pair.first);
        while (pos != string::npos) {
            modified_expression.replace(pos, pair.first.length(), pair.second ? "1" : "0");
            pos = modified_expression.find(pair.first, pos + 1);
        }
    }

    for (size_t i = 0; i < modified_expression.size(); ++i) {
        char c = modified_expression[i];
        if (c == '0' || c == '1') {
            operands.push(c - '0');
        } else if (c == '&' || c == '|' || c == '(') {
            operators.push(c);
        } else if (c == '~') {
            // Unary logical NOT operator
            if (i + 1 < modified_expression.size() && modified_expression[i + 1] == '(') {
                // Push '~' operator onto the stack
                operators.push(c);
            } else if (i + 1 < modified_expression.size()) {
                // Not followed by '(', treat it as a normal unary NOT
                char next_char = modified_expression[i + 1];
                operands.push(next_char == '0' ? 1 : 0);
                ++i; // Skip next character
            } else {
                // No character follows '~', error handling
                operators.push(c);
            }
        } else if (c == ')') {
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
                cerr << "Mismatched parentheses in the expression: " << modified_expression<<endl;
                return false;
                exit(1);
            }
        } else {
            cerr << "Invalid character "<<c<< " in the expression:: " << modified_expression<< endl;
            return false;
            exit(1);
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
        cerr << "Invalid expression format" << endl;
        return false;
        exit(1);
    }

    return operands.top();
}

// Function to parse component information from a file and store it in a map
unordered_map<string, Component> get_component(string filename) {
    unordered_map<string, Component> components;
    ifstream compfile(filename);
    if (compfile.fail()) {
        cout << "Error: Failed to open file." << endl;
    } else {
        string line;
        while (getline(compfile, line)) {
            // Remove commas from the line
            for (char& c : line) {
                if (c == ',') c = ' '; // Replace comma with space
            }
            
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

void evaluate_and_propagate(map<string, circ_comp>& circuit,unordered_map<string, Component>& components, vector<Stimulus>& stimuli) {
    for (const auto& stim : stimuli) {
        if(stim.time_stamp_ps==0){
            for (auto& comp : circuit) { // Iterate over circuit components
            auto& comp_info = comp.second;
            for (auto& input : comp_info.inputs) {
                if (input.first == stim.input) {
                    input.second = stim.logic_value;
                }
            }
    }}}
    auto is_time_stamp_zero = [](const Stimulus& stim) {
    return stim.time_stamp_ps == 0;
};
// // Remove elements from the stimuli vector where time_stamp_ps is 0
// stimuli.erase(std::remove_if(stimuli.begin(), stimuli.end(), is_time_stamp_zero), stimuli.end());

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

pair < map < string, circ_comp > , map < string, bool >> get_circuit(string filename) {
  map < string, circ_comp > circuit; // Map to store parsed circuit components
  map < string, bool > inputs; // Map to store parsed input signals
  unordered_map < string, int > output_count; // Map to count occurrences of output names
  vector < string > output_check;
  ifstream circuit_file(filename); // Open file stream for reading circuit information

  if (circuit_file.fail()) { // Check if file opening fails
    cout<<"Failed to open circuit file."; // Display error message and break
    exit(1);
  } else {
    string line; // Variable to store each line read from the file
     // Variables to track if INPUTS and COMPONENTS sections are found
  bool found_inputs = false;
  bool found_components = false;
  while (getline(circuit_file, line)) {
    if (!found_inputs && line.find("INPUTS:") != string::npos) { // Check if INPUTS section is found
      found_inputs = true;
    } else if (!found_components && line.find("COMPONENTS:") != string::npos) { // Check if COMPONENTS section is found
      found_components = true;
    }
    if (found_inputs && found_components) {
      // Both sections found, exit the loop
      break;
    }
  }
  // Check if INPUTS section is not found
  if (!found_inputs) {
    cout << "Error: 'INPUTS:' section not found in the file. Please edit the file to include 'INPUTS:' section with the appropriate format." << endl;
    exit(1); // Exit the program
  }

  // Check if COMPONENTS section is not found
  if (!found_components) {
    cout << "Error: 'COMPONENTS:' section not found in the file. Please edit the file to include 'COMPONENTS:' section with the appropriate format." << endl;
    exit(1); // Exit the program
  }

  // Reset file stream to beginning
  circuit_file.clear();
  circuit_file.seekg(0, ios::beg);
    string section; // Variable to determine the section of the file being parsed
    while (getline(circuit_file, line)) { // Read each line from the file
      // Remove commas from the line and replace them with space then read
      for (char & c: line) {
        if (c == ',') c = ' '; // Replace comma with space
      }
      stringstream currline(line); // Create stringstream to parse the line
      string token; // Token to store parsed values
      currline >> token; // Read first token from the line
      if (token == "INPUTS:") { // Check if current line specifies inputs section
        section = "INPUTS";
      } else if (token == "COMPONENTS:") { // Check if current line specifies components section
        section = "COMPONENTS";
      } else if (section == "INPUTS") { // Parse input signals
        for (char c: line) {
          if (c != ' ') {
            inputs[string(1, c)] = false; // Store input signal name with initial logic value as false
          }
        }
      } else if (section == "COMPONENTS") { // Parse circuit components
        string name, type, output_name;
        map < string, pair < bool, int >> output_map;
        vector < pair < string, bool >> component_inputs;
        name = token; // Component name
        currline >> type; // Component type
        currline >> output_name; // Output signal name

        if (inputs.find(output_name) != inputs.end()) {
            cout<<"Output name "<< output_name<<" conflicts with input name. This input will be assumed to be zero first then will depend on the output. However, this is not advicable because the program will keep simulating with same pattern forever (dont worry we have limited this time)"<< endl;
            // exit(1);
        }

        // Increment count for output name
        output_count[output_name]++;

        // Check if output name appears more than one time
        if (output_count[output_name] > 1) {
            cout<<"Output "<< output_name<<" appears as output for more than 1 component";
            exit(1);
        }
        output_check.push_back(output_name);
        output_map[output_name] = {
          false,
          0
        }; // Initialize output signal with initial logic value as false and simulation time as 0

        // Parse input signals for the component
        while (currline >> token) {
          if (token != ",") { // Check if token is not a comma
            // Check if token is not found in inputs section or in output_check vector
            if (inputs.find(token) == inputs.end()) {
               if (find(output_check.begin(), output_check.end(), token) != output_check.end()) {
                 if (output_map.find(token) != output_map.end()) {
                        cout<< "Error: Input "<< token<<" is the output of the same circuit.";
                        exit(1);
                    } 
                    else {    component_inputs.push_back({token, false});       
                }}
                else{
                    cout<< "Input "<< token<<" is not found in either inputs or previous outpus.";
                        exit(1);
                }}
                else{
                    component_inputs.push_back({token, false});
                    }
                }
              }

        
        circuit[name] = {
          name,
          type,
          output_map,
          component_inputs
        }; // Store parsed component in the map
      }
    }
  }
  return {
    circuit,
    inputs
  }; // Return parsed circuit components and input signals
}
void simulate(const vector < Stimulus > & stimuli, map < string, circ_comp > & circuit, unordered_map < string, Component > & components, map < string, bool > inputs, int simulation_time) {
    map < int, map < string, bool >> tick_time;
    vector < OutputInfo > outputInfos;
    vector < string > output_names;
    vector < pair <string,bool>> zerostim_names;
    vector < string > stim_names;
    int fcurrent_tick = 0;
    int fnncurrent_tick = 0;
    int fncurrent_tick = 0;
    for (const auto & comp: circuit) {
      output_names.push_back(comp.second.output.begin() -> first);
    }
    for (const auto &  stim : stimuli) {
        if (stim.time_stamp_ps == 0) {
        zerostim_names.push_back ({stim.input,stim.logic_value});
        }
    }
    for (const auto &  stim : stimuli) {
            stim_names.push_back(stim.input);
        }
        
    for (const auto & input: inputs) {
      if (find(stim_names.begin(), stim_names.end(), input.first) == stim_names.end()) {
        while (fncurrent_tick < simulation_time) {
          tick_time[fncurrent_tick][input.first] = 0;
          fncurrent_tick++;
        }}
    } 
    for (const auto & input: inputs) {
       if (find(output_names.begin(), output_names.end(), input.first) != output_names.end()) {
        while (fcurrent_tick < simulation_time) {
          tick_time[fcurrent_tick][input.first] = input.second;
          fcurrent_tick++;
        }}}

    for (const auto & input: inputs) {
      for (const auto &zero_stim : zerostim_names) {
        if (input.first == zero_stim.first) {
            // If it exists, set tick_time[0][input.first] to the corresponding logic value
            tick_time[0][input.first] = zero_stim.second;
        }
    }}
    int timme=0;
    for(const auto & input: inputs){
      if(tick_time[0].count(input.first) == 0){
        while(timme< simulation_time){
          tick_time[timme][input.first] = 0;
          timme++;
        }
      }
    }
    for(const auto & input: inputs){
      if(tick_time[0].count(input.first) == 0){
        while(timme< simulation_time){
          tick_time[timme][input.first] = 0;
          timme++;
        }
      }
    }  
    for(const auto& comp: circuit){
    auto & comp_info = comp.second;
    for (const auto & input: comp_info.inputs) {
      if(tick_time[0].count(input.first) == 0){
       if (find(output_names.begin(), output_names.end(), input.first) == output_names.end()) {
        int fcurrent_tick = 0;
        while (fcurrent_tick < simulation_time) {
          tick_time[fcurrent_tick][input.first] = 0;
          fcurrent_tick++;
        }}}
    }}

    for (const auto & stim: stimuli) {
      int current_tick = stim.time_stamp_ps;
      while (current_tick < simulation_time) {
        // Update input values if a stimulus matches the current tick
        if (stim.time_stamp_ps == current_tick) {
          tick_time[current_tick][stim.input] = stim.logic_value;
        } else {
          tick_time[current_tick][stim.input] = tick_time[current_tick - 1][stim.input];
        }
        current_tick++;
      }
    }
      int kcurrent_tick = 1;
      while (kcurrent_tick < simulation_time) {
        // Update input values if a stimulus matches the current tick
        for (const auto & stim: stimuli) {
        if (tick_time[kcurrent_tick].count(stim.input) == 0) {
          tick_time[kcurrent_tick][stim.input] = tick_time[kcurrent_tick - 1][stim.input];
        } 
      }
        kcurrent_tick++;
    }
    int neww = 0;
    while(neww<10){
    // Propagate input values to outputs for the current tick
    for (const auto & comp: circuit) {
      auto & comp_info = comp.second;
      int current_tick = 0;
      while (current_tick < simulation_time) {
        vector < pair < string, bool >> current_tick_inputs;
        for (const auto & input: comp_info.inputs) {
          if (tick_time[current_tick].count(input.first) > 0) {
            current_tick_inputs.push_back({
              input.first,
              tick_time[current_tick][input.first]
            });
          }
        }
        if (current_tick_inputs.size() == comp_info.inputs.size()) {
          bool output_value = evaluateExpression(components[comp_info.type].output_expression, current_tick_inputs);
          int simulation_time = current_tick + components[comp_info.type].delay_ps;
          tick_time[simulation_time][comp_info.output.begin() -> first] = output_value;
        }
        current_tick++;
      }
    }
    neww++;}
    
    int current_tick = 0;
    vector < pair < string, bool >> prev_tick_inputs;
    unordered_map < string, bool > circuit_outputs; // To keep track of already encountered outputs
    while (current_tick < simulation_time) {
      for (const auto & comp: circuit) {
        auto & comp_info = comp.second;
        if (tick_time.find(current_tick) != tick_time.end() && tick_time[current_tick].count(comp_info.output.begin() -> first) > 0) {
          if (circuit_outputs.find(comp_info.output.begin() -> first) == circuit_outputs.end()) {
            prev_tick_inputs.push_back({
              comp_info.output.begin() -> first,
              tick_time[current_tick][comp_info.output.begin() -> first]
            });
            outputInfos.push_back(OutputInfo(current_tick, comp_info.output.begin() -> first, tick_time[current_tick][comp_info.output.begin() -> first]));
            circuit_outputs[comp_info.output.begin() -> first] = true;
          }
        }
      }
      current_tick++;
    }

    int current_tick_n = 0;
    while (current_tick_n < simulation_time) {
      for (const auto & comp: circuit) {
        auto & comp_info = comp.second;
        if (tick_time.find(current_tick_n) != tick_time.end() && tick_time[current_tick_n].count(comp_info.output.begin() -> first) > 0) {
          // Search for the pair with the matching output name
          for (auto & prev_tick_input: prev_tick_inputs) {
            if (prev_tick_input.first == comp_info.output.begin() -> first) {
              // Update the value
              if (prev_tick_input.second != tick_time[current_tick_n][comp_info.output.begin() -> first]) {
                outputInfos.push_back(OutputInfo(current_tick_n, comp_info.output.begin() -> first, tick_time[current_tick_n][comp_info.output.begin() -> first]));
                // Update the previous value
                prev_tick_input.second = tick_time[current_tick_n][comp_info.output.begin() -> first];
              }
            }
          }
        }
      }
      current_tick_n++;
    }
    // Sort the vector based on time
    sort(outputInfos.begin(), outputInfos.end(), [](const OutputInfo & a,
      const OutputInfo & b) {
      return a.time < b.time;
    });

    // Display sorted output information
    for (const auto & outputInfo: outputInfos) {
      cout << outputInfo.time << ", " << outputInfo.outputName << ", " << outputInfo.value << endl;
    }

}
void test_for_stimlui_error(vector<Stimulus>& stimuli, map<string, bool>& inputs){  
    vector<std::string> uniqueStimulusInputs;
  for (const auto& stim : stimuli) {
    if (std::find(uniqueStimulusInputs.begin(), uniqueStimulusInputs.end(), stim.input) == uniqueStimulusInputs.end()) {
        uniqueStimulusInputs.push_back(stim.input);
    }
}  
for (const auto& uniqueInput : uniqueStimulusInputs) {
    bool matched = false;
    for (const auto& input : inputs) {
        if (uniqueInput == input.first) {
            matched = true;
            break;
        }
    }
    if (!matched) {
        cout << "Small error: Stimulus input " << uniqueInput << " does not match any of the circuit inputs so it doesn't have any effect" << endl;
    }
}
    }
void circuit_test(map<string, circ_comp>& circuit, map<string, bool>& inputs, unordered_map<string, Component>& components){

    for (const auto& input : inputs) {
    bool matched = false;
    for (const auto& comp : circuit) {
        for (const auto& comp_input : comp.second.inputs) {
            if (input.first == comp_input.first) {
                matched = true;
                break;
            }
        }
            }
    if (!matched) {
        cout << "Small error: Circuit input " << input.first << " is not used in any of the components" << endl;
}}
bool found = false; // Flag to track if component type is found

for (const auto& comp : circuit) {
    const auto& comp_info = comp.second;
    found = false; // Reset the flag for each component in the circuit

    for (const auto& comp : components) { // Iterate over components
        const Component& component = comp.second;
        if (component.name == comp_info.type) {
            found = true; // Set the flag if component type is found
            break; // No need to continue checking once found
        }
    }

    if (!found) {
        // Return an error only if component type is not found
        cout << "Error: Component " << comp_info.type << " not found in the components library; check the format from the library to see if it is written wrong or add the gate there yourself." << endl;
    exit(1);
    }
}

    for (const auto& comp : circuit) {
    const auto& comp_info = comp.second;
    if (isdigit(comp_info.type.back())) {
        int num_inputs = comp_info.type.back() - '0';
        if (num_inputs != comp_info.inputs.size()) {
            cout << "Error: There are "<<comp_info.inputs.size()<< " inputs in "<<comp_info.name <<" component that has a " << comp_info.type << " gate that takes " << num_inputs << " inputs."<< endl;
        exit(1);
        }
    } else {
        if (comp_info.inputs.size() != 1) {
            cout << "Error: There are "<<comp_info.inputs.size()<< " inputs in "<<comp_info.name <<" component that has a " << comp_info.type << " gate that takes 1 input."<< endl;                  
               exit(1);
                }
    } 

}}