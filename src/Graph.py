import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from PIL import Image

# Initialize outputs map
outputs = {}

# Function to read simulation data from file
def read_simulation_data(simulation_folder):
    initial_inputs = {}
    with open(os.path.join(simulation_folder, "simulation.sim"), "r") as file:
        for line in file:
            parts = [x.strip() for x in line.split(',')]
            sim_time, var, value = parts[0], parts[1], parts[2]
            if var not in initial_inputs:
                initial_inputs[var] = not bool(int(value))  # Store negation of the first occurrence
            if var in outputs:
                outputs[var].append(int(sim_time))
            else:
                outputs[var] = [int(sim_time)]
    return initial_inputs

# Iterate over directories
for n in range(1, 6):
    simulation_folder = os.path.join("../tests", str(n))
    initial_inputs = read_simulation_data(simulation_folder)

    # Determine the overall simulation time
    overall_simulation_time = max(max(times) for times in outputs.values())

    # Function to generate waveform for a variable
    def generate_waveform(sim_times, initial_value):
        data = []
        current_value = initial_value
        for i in range(overall_simulation_time + 1):
            if i in sim_times:
                current_value = 1 - current_value  # Flip the value
            data.append(current_value)
        return data

    # Number of variables
    num_vars = len(outputs)

    # Create subplots
    fig, axs = plt.subplots(num_vars, 1, figsize=(8, 4 * num_vars))

    # Initialization function
    def init():
        for ax in axs:
            ax.set_ylim(0, 1)
            ax.set_yticks([0, 1])
            ax.set_xticks(range(0, overall_simulation_time + 1, 100))
        return []

    # Update function
    def update(frame):
        for idx, (var, sim_times) in enumerate(outputs.items()):
            initial_value = initial_inputs[var]
            data = generate_waveform(sim_times, initial_value)[:int(frame*100)+1]
            xs = np.repeat(range(len(data)), 2)
            ys = np.repeat(data, 2)
            xs = xs[1:]
            ys = ys[:-1]
            xs = np.append(xs, xs[-1] + 1)
            ys = np.append(ys, ys[-1])
            axs[idx].cla()
            axs[idx].plot(xs, ys, label=var)
            axs[idx].set_title(var)
            for time in sim_times:
                if time <= int(frame*100):
                    axs[idx].text(time, 1, str(time), ha='center', fontsize=7)
        return []

    num_frames = int((overall_simulation_time + 1) / 10)

    ani = FuncAnimation(fig, update, frames=num_frames, init_func=init, blit=True)

    plt.subplots_adjust(hspace=1.5)

    frames = []

    # Render each frame and append to frames list
    for frame_idx in range(50):
        update(frame_idx)
        fig.canvas.draw()
        frame = np.array(fig.canvas.renderer.buffer_rgba())
        frames.append(Image.fromarray(frame))

    # Save GIF in the respective directory
    gif_path = os.path.join(simulation_folder, "simulation_animation.gif")
    frames[0].save(gif_path, format='GIF', append_images=frames[1:], save_all=True, duration=100, loop=0)
    
    # Clear outputs for the next iteration
    outputs.clear()

    plt.close(fig)  # Close the figure to release memory
