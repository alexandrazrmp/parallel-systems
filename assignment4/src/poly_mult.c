/* poly_mult.c
 *
 * Exercise 4.1 - serial and SIMD serial polynomial multiplication
 *
 * Usage:
 *   ./poly_mult <polynomial_degree_n>
 *
 * Output:
 *   initialization time
 *   serial multiplication time
 *   simd multiplication time (or fallback scalar time)
 *   verification PASS/FAIL
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <string.h>
#include <inttypes.h>


#ifdef __AVX2__
#include <immintrin.h>
#endif

/* Return current time in seconds */
static double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

/* Serial multiplication (standard algorithm)
 * A, B: int32 arrays length n+1
 * C: int64 array length 2*n+1 (output)
 */
void multiply_polynomials_serial(const int32_t *A, const int32_t *B, int64_t *C, int n) {
    for (int i = 0; i <= 2 * n; i++) {
        int64_t sum = 0;
        int jmin = (i - n) > 0 ? (i - n) : 0;
        int jmax = i < n ? i : n;
        for (int j = jmin; j <= jmax; j++) {
            sum += (int64_t)A[j] * (int64_t)B[i - j];
        }
        C[i] = sum;
    }
}

/* SIMD-accelerated serial multiplication (outer-product vectorized)
 * Uses AVX2 intrinsics to process 8 int32 B values at once, accumulating into int64 C.
 * If AVX2 is not available at compile-time, falls back to scalar outer-product.
 */
void multiply_polynomials_simd(const int32_t *A, const int32_t *B, int64_t *C, int n) {
#ifdef __AVX2__
    const int N = n + 1;
    // Zero the result
    // int lenC = 2 * n + 1;
    // We'll perform outer product: for j in 0..n: C[j+k] += A[j] * B[k]
    for (int j = 0; j < N; j++) {
        // broadcast A[j] as 32-bit replicated across 8 lanes
        __m256i va = _mm256_set1_epi32(A[j]);

        int k = 0;
        // Vector loop handling blocks of 8 B elements
        for (; k <= n - 7; k += 8) {
            // load 8 int32 B[k..k+7]
            __m256i vb = _mm256_loadu_si256((const __m256i*)(B + k));
            // 8 parallel int32 products
            __m256i prod32 = _mm256_mullo_epi32(va, vb); // 8 x int32

            // split into two 128-bit lanes (low, high), then widen each to int64x4
            __m128i prod32_lo_128 = _mm256_extracti128_si256(prod32, 0); // contains 4 x int32
            __m128i prod32_hi_128 = _mm256_extracti128_si256(prod32, 1); // contains 4 x int32

            __m256i prod64_lo = _mm256_cvtepi32_epi64(prod32_lo_128); // 4 x int64
            __m256i prod64_hi = _mm256_cvtepi32_epi64(prod32_hi_128); // 4 x int64

            // load the corresponding C parts (4 x int64 each)
            // C[j + k + t] for t = 0..7, so we load two 4-lane vectors at offsets k and k+4
            int idx_lo = j + k;
            int idx_hi = j + k + 4;
            // be careful with bounds: idx_hi + 3 <= 2*n guaranteed because j <= n and k <= n-7
            __m256i cvec_lo = _mm256_loadu_si256((__m256i*)(C + idx_lo));
            __m256i cvec_hi = _mm256_loadu_si256((__m256i*)(C + idx_hi));

            // add and store back
            cvec_lo = _mm256_add_epi64(cvec_lo, prod64_lo);
            cvec_hi = _mm256_add_epi64(cvec_hi, prod64_hi);
            _mm256_storeu_si256((__m256i*)(C + idx_lo), cvec_lo);
            _mm256_storeu_si256((__m256i*)(C + idx_hi), cvec_hi);
        }

        // Handle remaining k values (tail)
        for (; k <= n; k++) {
            C[j + k] += (int64_t)A[j] * (int64_t)B[k];
        }
    }
#else
    // Fallback scalar outer-product (still serial but no intrinsics)
    int N = n + 1;
    for (int j = 0; j < N; j++) {
        for (int k = 0; k < N; k++) {
            C[j + k] += (int64_t)A[j] * (int64_t)B[k];
        }
    }
#endif
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <polynomial_degree_n>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "degree must be positive\n");
        return EXIT_FAILURE;
    }

    int32_t *A = NULL;
    int32_t *B = NULL;
    int64_t *C_serial = NULL;
    int64_t *C_simd = NULL;

    double t0 = get_time();

    A = (int32_t*)malloc((n + 1) * sizeof(int32_t));
    B = (int32_t*)malloc((n + 1) * sizeof(int32_t));
    C_serial = (int64_t*)malloc((2 * n + 1) * sizeof(int64_t));
    C_simd = (int64_t*)malloc((2 * n + 1) * sizeof(int64_t));

    if (!A || !B || !C_serial || !C_simd) {
        fprintf(stderr, "Allocation failed\n");
        return EXIT_FAILURE;
    }

    // Initialize polynomials: random integer coefficients non-zero
    srand(42);
    for (int i = 0; i <= n; i++) {
        int r1 = (rand() % 20) - 10; // -10..9
        int r2 = (rand() % 20) - 10;
        if (r1 == 0) r1 = 1;
        if (r2 == 0) r2 = 1;
        A[i] = (int32_t)r1;
        B[i] = (int32_t)r2;
    }

    // zero result arrays
    int lenC = 2 * n + 1;
    for (int i = 0; i < lenC; i++) {
        C_serial[i] = 0;
        C_simd[i] = 0;
    }

    double t1 = get_time();
    printf("Initialization time: %.6f seconds\n", t1 - t0);

    // Serial multiplication (original algorithm)
    double tstart = get_time();
    multiply_polynomials_serial(A, B, C_serial, n);
    double tend = get_time();
    printf("Serial multiplication time: %.6f seconds\n", tend - tstart);

    // SIMD multiplication (vectorized outer-product)
    tstart = get_time();
    multiply_polynomials_simd(A, B, C_simd, n);
    tend = get_time();
    #ifdef __AVX2__
    printf("SIMD (AVX2) multiplication time: %.6f seconds\n", tend - tstart);
    #else
    printf("SIMD (not available) fallback scalar multiplication time: %.6f seconds\n", tend - tstart);
    #endif

    // Verify results match
    int valid = 1;
    for (int i = 0; i <= 2 * n; i++) {
        if (C_serial[i] != C_simd[i]) {
            valid = 0;
            fprintf(stderr, "Mismatch at index %d: serial=%" PRId64 ", simd=%" PRId64 "\n", i, C_serial[i], C_simd[i]);
            break;
        }
    }
    printf("Verification: %s\n", valid ? "PASS" : "FAIL");

    free(A);
    free(B);
    free(C_serial);
    free(C_simd);

    return valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
