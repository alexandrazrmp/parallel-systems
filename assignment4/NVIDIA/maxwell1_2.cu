%%writefile Sources/maxwell.cu
#include "dli.h"
#include <thrust/iterator/zip_iterator.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/tuple.h>

// FIXME(Step 1):
// accept device containers instead of `std::vector<float>`
void update_hx(int n, float dx, float dy, float dt, thrust::device_vector<float> &hx_vec,
               thrust::device_vector<float> &ez_vec) {
  
  // Use raw pointers for maximum performance
  float* hx = thrust::raw_pointer_cast(hx_vec.data());
  float* ez = thrust::raw_pointer_cast(ez_vec.data());

  // FIXME(Step 2):
  // Use zip and transform iterators to avoid materializing `ez[i + n] - ez[i]`
  auto begin = thrust::make_zip_iterator(thrust::make_tuple(hx, ez + n, ez));
  auto end   = thrust::make_zip_iterator(thrust::make_tuple(hx + (hx_vec.size() - n), ez + ez_vec.size(), ez + (ez_vec.size() - n)));

  thrust::for_each(thrust::device, begin, end, 
    [=] __device__ (auto t) {
        float& h_val = thrust::get<0>(t);  // Write to hx
        float ez_next = thrust::get<1>(t); // Read ez[i+n]
        float ez_curr = thrust::get<2>(t); // Read ez[i]
        // Compute difference on the fly
        h_val = h_val - dli::C0 * dt / 1.3f * (ez_next - ez_curr) / dy;
    });
}

// FIXME(Step 1):
// accept device containers instead of `std::vector<float>`
void update_hy(int n, float dx, float dy, float dt, thrust::device_vector<float> &hy_vec,
               thrust::device_vector<float> &ez_vec) {
  
  float* hy = thrust::raw_pointer_cast(hy_vec.data());
  float* ez = thrust::raw_pointer_cast(ez_vec.data());

  // FIXME(Step 2):
  // Use zip and transform iterators to avoid materializing `ez[i] - ez[i + 1]`
  auto begin = thrust::make_zip_iterator(thrust::make_tuple(hy, ez, ez + 1));
  auto end   = thrust::make_zip_iterator(thrust::make_tuple(hy + (hy_vec.size() - 1), ez + (ez_vec.size() - 1), ez + ez_vec.size()));

  thrust::for_each(thrust::device, begin, end, 
    [=] __device__ (auto t) {
        float& h_val = thrust::get<0>(t);  // Write to hy
        float ez_curr = thrust::get<1>(t); // Read ez[i]
        float ez_next = thrust::get<2>(t); // Read ez[i+1]
        // Compute difference on the fly
        h_val = h_val - dli::C0 * dt / 1.3f * (ez_curr - ez_next) / dx;
    });
}

// FIXME(Step 1):
// accept device containers instead of `std::vector<float>`
void update_dz(int n, float dx, float dy, float dt, thrust::device_vector<float> &hx_vec,
               thrust::device_vector<float> &hy_vec, thrust::device_vector<float> &dz_vec,
               int total_cells) {
  
  float* hx = thrust::raw_pointer_cast(hx_vec.data());
  float* hy = thrust::raw_pointer_cast(hy_vec.data());
  float* dz = thrust::raw_pointer_cast(dz_vec.data());

  // FIXME(Step 1):
  // compute for each on GPU
  auto counting_iter = thrust::make_counting_iterator(0);
  
  // Optimization: Start from n+1 to avoid branch divergence inside the kernel
  thrust::for_each(thrust::device, counting_iter + n + 1, counting_iter + total_cells,
                [n, dx, dy, dt, hx, hy, dz] __device__ (int cell_id) {
                    float hx_diff = hx[cell_id - n] - hx[cell_id];
                    float hy_diff = hy[cell_id] - hy[cell_id - 1];
                    dz[cell_id] += dli::C0 * dt * (hx_diff / dx + hy_diff / dy);
                });
}

// FIXME(Step 1):
// accept device containers instead of `std::vector<float>`
void update_ez(thrust::device_vector<float> &ez_vec, thrust::device_vector<float> &dz_vec) {
  
  float* ez = thrust::raw_pointer_cast(ez_vec.data());
  float* dz = thrust::raw_pointer_cast(dz_vec.data());

  // FIXME(Step 1):
  // compute transformation on GPU
  thrust::transform(thrust::device, dz, dz + dz_vec.size(), ez,
                 [] __host__ __device__ (float d) { return d / 1.3f; });
}

// Do not change the signature of this function
void simulate(int cells_along_dimension, float dx, float dy, float dt,
              thrust::device_vector<float> &d_hx,
              thrust::device_vector<float> &d_hy,
              thrust::device_vector<float> &d_dz,
              thrust::device_vector<float> &d_ez) {
  
  int cells = cells_along_dimension * cells_along_dimension;

  // FIXME(Step 2):
  // Remove `cell_ids` vector and use counting iterator instead

  // FIXME(Step 2):
  // Remove `buffer` vector and use fancy iterators instead

  for (int step = 0; step < dli::steps; step++) {
    update_hx(cells_along_dimension, dx, dy, dt, d_hx, d_ez);
    update_hy(cells_along_dimension, dx, dy, dt, d_hy, d_ez);
    update_dz(cells_along_dimension, dx, dy, dt, d_hx, d_hy, d_dz, cells);
    update_ez(d_ez, d_dz);
  }
}