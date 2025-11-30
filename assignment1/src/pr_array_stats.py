import pandas as pd
import matplotlib.pyplot as plt
import os

# Set paths for input CSV and output image
results_dir = 'results'
csv_file = os.path.join(results_dir, 'raw_results_stats.csv')
output_plot = os.path.join(results_dir, 'plot_array_stats.png')

# Check if input file exists
if not os.path.exists(csv_file):
    print(f"Error: File {csv_file} not found.")
    exit()

# Load data from CSV
df = pd.read_csv(csv_file)

# Setup the figure size
plt.figure(figsize=(10, 6))

# Plot Serial Time
plt.plot(df['Size'], df['Serial_Time'], marker='o', label='Serial Execution', linewidth=2)

# Plot Parallel Time
plt.plot(df['Size'], df['Parallel_Time'], marker='s', linestyle='--', color='red', label='Parallel Execution (4 Threads)', linewidth=2)

# Configure labels, title, and grid
plt.title('Exercise 1.3: False Sharing Effect (Serial vs Parallel)', fontsize=14)
plt.xlabel('Array Size (Number of Elements)', fontsize=12)
plt.ylabel('Execution Time (seconds)', fontsize=12)
plt.legend(fontsize=11)
plt.grid(True, linestyle=':', alpha=0.6)

# Save the plot
plt.savefig(output_plot)
print(f"Plot saved successfully to: {output_plot}")