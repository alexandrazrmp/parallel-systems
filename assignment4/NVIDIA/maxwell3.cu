%%writefile Sources/coarse.cu
#include "dli.h"
#include <cub/cub.cuh>

__global__ void kernel(dli::temperature_grid_f fine,
                       dli::temperature_grid_f coarse) {
  int coarse_row = blockIdx.x / coarse.extent(1);
  int coarse_col = blockIdx.x % coarse.extent(1);
  int row = threadIdx.x / dli::tile_size;
  int col = threadIdx.x % dli::tile_size;
  int fine_row = coarse_row * dli::tile_size + row;
  int fine_col = coarse_col * dli::tile_size + col;

  float thread_value = fine(fine_row, fine_col);

  // FIXME(Step 3):
  // Compute the sum of `thread_value` across threads of a thread block using `cub::BlockReduce`
  
  // 1. Define the BlockReduce type for float and the specific block size
  using BlockReduce = cub::BlockReduce<float, dli::block_threads>;
  
  // 2. Allocate shared memory for CUB
  __shared__ typename BlockReduce::TempStorage temp_storage;

  // 3. Perform the reduction
  float block_sum = BlockReduce(temp_storage).Sum(thread_value);

  // FIXME(Step 3):
  if (threadIdx.x == 0) {
      // 4. Compute average and write to memory
      float block_average = block_sum / dli::block_threads;
      coarse(coarse_row, coarse_col) = block_average;
  }
}

// Don't change the signature of this function
void coarse(dli::temperature_grid_f fine, dli::temperature_grid_f coarse) {
  kernel<<<coarse.size(), dli::block_threads>>>(fine, coarse);
}