import pandas as pd
import matplotlib.pyplot as plt
import io

# ---------------------------------------------------------
# 1. YOUR DATA (Embedded directly for ease of use)
# ---------------------------------------------------------

mpi_data = """N,Nodes,Iteration,Send_Time,Comp_Time,Recv_Time,Total_Time,Status
200000,2,1,0.001674,86.572095,0.002020,86.575789,OK
200000,2,2,0.001680,91.140062,0.021748,91.163491,OK
200000,2,3,0.001673,86.359397,0.001795,86.362865,OK
200000,2,4,0.013713,84.837717,0.001796,84.853227,OK
200000,4,1,0.105491,84.773090,0.144004,85.022585,OK
200000,4,2,0.087292,114.347634,0.053885,114.488811,OK
200000,4,3,0.015319,97.619026,0.008876,97.643222,OK
200000,4,4,0.137046,92.312904,0.030004,92.479954,OK
200000,8,1,0.229126,50.280399,0.016210,50.525735,OK
200000,8,2,1.088066,49.261087,0.024067,50.373220,OK
200000,8,3,0.657259,55.693347,0.080064,56.430670,OK
200000,8,4,0.504126,48.202378,0.034031,48.740535,OK
200000,16,1,0.491607,31.383667,0.025984,31.901258,OK
200000,16,2,0.313750,18.735577,0.020784,19.070111,OK
200000,16,3,0.377270,18.805140,0.027647,19.210057,OK
200000,16,4,0.469091,20.435318,0.020806,20.925215,OK
200000,32,1,0.176301,8.446909,0.032032,8.655241,OK
200000,32,2,0.182438,7.339679,0.017474,7.539590,OK
200000,32,3,0.202911,7.303883,0.002361,7.509155,OK
200000,32,4,0.167669,7.336581,0.008035,7.512285,OK
200000,64,1,0.277048,5.569359,0.015366,5.861774,OK
200000,64,2,0.130443,6.138256,0.010398,6.279097,OK
200000,64,3,0.193751,6.721257,0.004701,6.919709,OK
200000,64,4,0.121799,7.184552,0.003342,7.309693,OK"""

openmp_data = """degree,threads,run,serial_time,parallel_time,verification
200000,1,1,23.467973,23.494727,PASS
200000,1,2,22.970085,22.428118,PASS
200000,1,3,22.275824,22.268275,PASS
200000,1,4,22.852490,23.004846,PASS
200000,2,1,22.367774,11.528369,PASS
200000,2,2,22.431847,11.567517,PASS
200000,2,3,22.331492,11.569431,PASS
200000,2,4,22.374146,11.512883,PASS
200000,4,1,22.318477,6.089070,PASS
200000,4,2,22.438508,6.107346,PASS
200000,4,3,22.304185,6.792379,PASS
200000,4,4,22.326970,6.078164,PASS
200000,8,1,22.433322,7.220958,PASS
200000,8,2,22.989762,6.082362,PASS
200000,8,3,22.233573,6.072490,PASS
200000,8,4,22.420245,6.076739,PASS"""

pthreads_data = """degree,threads,run,init_time,serial_time,parallel_time,verification
200000,1,1,0.004986,22.437961,22.247714,PASS
200000,1,2,0.004992,22.181280,22.191396,PASS
200000,1,3,0.004935,22.194969,22.402402,PASS
200000,1,4,0.004865,22.179597,22.191564,PASS
200000,2,1,0.005284,22.445687,11.532069,PASS
200000,2,2,0.005284,22.247226,11.599454,PASS
200000,2,3,0.004895,22.367643,11.526479,PASS
200000,2,4,0.007956,22.261059,11.719466,PASS
200000,4,1,0.005069,23.244690,10.053832,PASS
200000,4,2,0.005255,23.165596,10.222412,PASS
200000,4,3,0.004967,23.102388,10.126320,PASS
200000,4,4,0.004988,23.395999,12.357970,PASS
200000,8,1,0.005237,23.667981,12.276440,PASS
200000,8,2,0.005088,23.707903,12.419000,PASS
200000,8,3,0.005121,23.582900,6.664103,PASS
200000,8,4,0.004997,22.326434,6.768581,PASS"""

# ---------------------------------------------------------
# 2. PROCESSING
# ---------------------------------------------------------

# Read the strings as CSV files
df_mpi = pd.read_csv(io.StringIO(mpi_data))
df_omp = pd.read_csv(io.StringIO(openmp_data))
df_pth = pd.read_csv(io.StringIO(pthreads_data))

# Group by Nodes/Threads and calculate the mean (average of the 4 runs)
mpi_avg = df_mpi.groupby('Nodes').mean(numeric_only=True).reset_index()
omp_avg = df_omp.groupby('threads').mean(numeric_only=True).reset_index()
pth_avg = df_pth.groupby('threads').mean(numeric_only=True).reset_index()

# ---------------------------------------------------------
# 3. PLOTTING
# ---------------------------------------------------------

# --- Plot 1: MPI Total Execution Time ---
plt.figure(figsize=(8, 5))
plt.plot(mpi_avg['Nodes'], mpi_avg['Total_Time'], marker='o', linestyle='-', color='blue')
plt.title('Total Execution Time vs MPI Processes (N=200,000)')
plt.xlabel('Number of Processes')
plt.ylabel('Time (seconds)')
plt.grid(True)
plt.tight_layout()
plt.savefig('mpi_total_time.png')
print("Saved mpi_total_time.png")
# plt.show() # Uncomment to see the plot on screen

# --- Plot 2: MPI Breakdown (Send/Comp/Recv) ---
plt.figure(figsize=(8, 5))
nodes = mpi_avg['Nodes'].astype(str) # Convert to string for bar labels
send = mpi_avg['Send_Time']
comp = mpi_avg['Comp_Time']
recv = mpi_avg['Recv_Time']

# Create stacked bars
plt.bar(nodes, send, label='Send', color='red')
plt.bar(nodes, comp, bottom=send, label='Compute', color='green')
plt.bar(nodes, recv, bottom=send+comp, label='Receive', color='orange')

plt.title('MPI Time Breakdown (N=200,000)')
plt.xlabel('MPI Processes')
plt.ylabel('Time (seconds)')
plt.legend()
plt.tight_layout()
plt.savefig('mpi_breakdown.png')
print("Saved mpi_breakdown.png")
# plt.show()

# --- Plot 3: Comparison (MPI vs OpenMP vs Pthreads) ---
plt.figure(figsize=(8, 5))

# Plot MPI line
plt.plot(mpi_avg['Nodes'], mpi_avg['Total_Time'], 
         marker='o', label='MPI (Nodes)', color='blue')

# Plot OpenMP line
plt.plot(omp_avg['threads'], omp_avg['parallel_time'], 
         marker='s', label='OpenMP (Threads)', color='green', linestyle='--')

# Plot Pthreads line
plt.plot(pth_avg['threads'], pth_avg['parallel_time'], 
         marker='^', label='Pthreads (Threads)', color='red', linestyle=':')

plt.title('Performance Comparison (N=200,000)')
plt.xlabel('Number of Processes / Threads')
plt.ylabel('Time (seconds)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('comparison_all.png')
print("Saved comparison_all.png")
# plt.show()