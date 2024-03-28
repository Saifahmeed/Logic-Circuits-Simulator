# Logic Simulator

This is a command-line application for simulating digital circuits. The application is designed to run from the terminal using the following format:

logicsim <library_file> <circuit_file> <stimuli_file>

## Prerequisites

Before running the application, ensure you have the following:

- C++ compiler installed on your system.
- A compatible operating system (e.g., Windows, macOS, Linux).

## Usage

Follow these steps to run the application:

1. Compile the source code using a C++ compiler.
2. Open the terminal.
3. Navigate to the directory containing the compiled executable file.
4. Execute the application using the specified format.

Replace <library_file>, <circuit_file>, and <stimuli_file> with the appropriate file paths:

- <library_file>: Path to the file containing component library information.
- <circuit_file>: Path to the file describing the circuit.
- <stimuli_file>: Path to the file containing stimuli for simulation.

## Example

logicsim library.txt circuit.txt stimuli.txt

## Components Library Format

The component library file should contain information about various logic gates/components. Each line in the file represents a single component with the following format:

<component_name> <num_inputs> <output_expression> <delay>

- <component_name>: Name of the component.
- <num_inputs>: Number of inputs for the component.
- <output_expression>: Expression representing the output logic of the component.
- <delay>: Delay in picoseconds.

## Circuit File Format

The circuit file should describe the circuit layout. It consists of two sections: INPUTS: and COMPONENTS:. Each section lists the inputs and components used in the circuit, respectively.

Example format:

INPUTS:
A B C

COMPONENTS:
comp1 AND A B output1
comp2 OR output1 C output2

## Stimuli File Format

The stimuli file contains input stimuli for the simulation. Each line represents a stimulus with the format:

<time_stamp> <input> <logic_value>

- <time_stamp>: Time stamp in picoseconds.
- <input>: Name of the input signal.
- <logic_value>: Logic value (0 or 1) for the input at the specified time stamp.

## Output

The simulation results are written to a file named simulation.sim in the current directory. Each line in the output file represents a change in the circuit's state, including input changes and output values at specific time stamps.

## Error Handling

The application performs error checks during execution and provides appropriate error messages if:

- Required files cannot be opened.
- Input/output naming conflicts occur.
- Circuit components are missing or incorrectly defined.
- Incorrect input/output configurations are detected.

Please ensure that your input files follow the specified formats to avoid errors.

Authors: - 
Saif Abd Elfattah
Adham Hassan
Amr Mohamed Tolba
