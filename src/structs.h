#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <map>
#include <vector>

// Structures for representing stimulus, component, and circuit components
struct Stimulus {
    int time_stamp_ps;
    std::string input;
    int logic_value;
};

struct Component {
    std::string name;
    int num_inputs;
    std::string output_expression;
    int delay_ps;
};

struct circ_comp {
    std::string name;
    std::string type;
    std::map<std::string, std::pair<bool, int>> output; // Change output to a map with simulation time
    std::vector<std::pair<std::string, bool>> inputs;
};

#endif // STRUCTS_H
