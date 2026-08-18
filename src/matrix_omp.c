#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <string.h>

double* allocate_matrix(int n) {
    double* mat = (double*)malloc((size_t)n * n * sizeof(double));
    if (!mat) {
        fprintf(stderr, "Error alokasi memori matriks %dx%d\n", n, n);
        exit(EXIT_FAILURE);
    }
    return mat;
}

void init_matrix(double* mat, int n) {
    for (int i = 0; i < n * n; i++) {
        mat[i] = (double)(rand() % 100) / 10.0;
    }
}

void zero_matrix(double* mat, int n) {
    memset(mat, 0, (size_t)n * n * sizeof(double));
}

int verify_matrix(double* A, double* B, int n) {
    double eps = 1e-4;
    for (int i = 0; i < n * n; i++) {
        if (fabs(A[i] - B[i]) > eps) {
            return 0;
        }
    }
    return 1;
}

void matmul_serial(double* A, double* B, double* C, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i * n + k] * B[k * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}

void matmul_omp(double* A, double* B, double* C, int n, int threads) {
    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i * n + k] * B[k * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}

void transpose_matrix(double* src, double* dst, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dst[j * n + i] = src[i * n + j];
        }
    }
}

void matmul_transpose_serial(double* A, double* B, double* C, int n) {
    double* B_T = allocate_matrix(n);
    transpose_matrix(B, B_T, n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i * n + k] * B_T[j * n + k];
            }
            C[i * n + j] = sum;
        }
    }

    free(B_T);
}

void matmul_transpose_omp(double* A, double* B, double* C, int n, int threads) {
    double* B_T = allocate_matrix(n);

    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            B_T[j * n + i] = B[i * n + j];
        }
    }

    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i * n + k] * B_T[j * n + k];
            }
            C[i * n + j] = sum;
        }
    }

    free(B_T);
}

void matadd_serial(double* A, double* B, double* C, int n) {
    for (int i = 0; i < n * n; i++) {
        C[i] = A[i] + B[i];
    }
}

void matadd_omp(double* A, double* B, double* C, int n, int threads) {
    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < n * n; i++) {
        C[i] = A[i] + B[i];
    }
}

int main(int argc, char* argv[]) {
    int sizes[] = {500, 1500, 3000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    if (argc > 1) {
        int custom_size = atoi(argv[1]);
        if (custom_size > 0) {
            sizes[0] = custom_size;
            num_sizes = 1;
        }
    }

    int max_threads = omp_get_max_threads();
    int thread_counts[] = {1, 2, 4, 8, 16};
    int valid_thread_counts[5];
    int num_thread_configs = 0;

    for (int i = 0; i < 5; i++) {
        if (thread_counts[i] <= max_threads || thread_counts[i] == 1 || thread_counts[i] == 2 || thread_counts[i] == 4) {
            if (num_thread_configs == 0 || thread_counts[i] > valid_thread_counts[num_thread_configs - 1]) {
                valid_thread_counts[num_thread_configs++] = thread_counts[i];
            }
        }
    }

    FILE* csv_file = fopen("benchmark_results.csv", "w");
    FILE* json_file = fopen("benchmark_results.json", "w");

    if (csv_file) {
        fprintf(csv_file, "MatrixSize,Mode,Threads,TimeSec,Speedup,Efficiency,GFLOPS,Verified\n");
    }

    if (json_file) {
        fprintf(json_file, "[\n");
    }

    printf("=================================================================================\n");
    printf("              BENCHMARK PEMROSESAN MATRIKS SERIAL VS OPENMP PARALEL              \n");
    printf("=================================================================================\n");
    printf("Max Available Threads: %d\n\n", max_threads);

    int first_json_entry = 1;

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        printf(">>> Menguji Matriks Ukuran: %d x %d <<<\n", n, n);

        double* A = allocate_matrix(n);
        double* B = allocate_matrix(n);
        double* C_ref = allocate_matrix(n);
        double* C_test = allocate_matrix(n);

        srand(42);
        init_matrix(A, n);
        init_matrix(B, n);

        zero_matrix(C_ref, n);
        double start_time = omp_get_wtime();
        matmul_serial(A, B, C_ref, n);
        double serial_time = omp_get_wtime() - start_time;
        double serial_gflops = (2.0 * n * n * n) / (serial_time * 1e9);

        printf("  [Serial Naive]      Time: %8.4f s | GFLOPS: %6.2f\n", serial_time, serial_gflops);

        if (csv_file) {
            fprintf(csv_file, "%d,Serial_Naive,1,%.6f,1.00,100.00,%.2f,1\n", n, serial_time, serial_gflops);
        }
        if (json_file) {
            if (!first_json_entry) fprintf(json_file, ",\n");
            fprintf(json_file, "  {\"matrix_size\": %d, \"mode\": \"Serial_Naive\", \"threads\": 1, \"time_sec\": %.6f, \"speedup\": 1.00, \"efficiency\": 100.00, \"gflops\": %.2f, \"verified\": true}",
                    n, serial_time, serial_gflops);
            first_json_entry = 0;
        }

        zero_matrix(C_test, n);
        start_time = omp_get_wtime();
        matmul_transpose_serial(A, B, C_test, n);
        double serial_trans_time = omp_get_wtime() - start_time;
        double serial_trans_gflops = (2.0 * n * n * n) / (serial_trans_time * 1e9);
        int verified = verify_matrix(C_ref, C_test, n);

        printf("  [Serial Transpose]  Time: %8.4f s | GFLOPS: %6.2f | Status: %s\n", 
               serial_trans_time, serial_trans_gflops, verified ? "PASSED" : "FAILED");

        if (csv_file) {
            fprintf(csv_file, "%d,Serial_Transpose,1,%.6f,%.2f,%.2f,%.2f,%d\n", 
                    n, serial_trans_time, serial_time / serial_trans_time, (serial_time / serial_trans_time) * 100.0, serial_trans_gflops, verified);
        }
        if (json_file) {
            fprintf(json_file, ",\n  {\"matrix_size\": %d, \"mode\": \"Serial_Transpose\", \"threads\": 1, \"time_sec\": %.6f, \"speedup\": %.2f, \"efficiency\": %.2f, \"gflops\": %.2f, \"verified\": %s}",
                    n, serial_trans_time, serial_time / serial_trans_time, (serial_time / serial_trans_time) * 100.0, serial_trans_gflops, verified ? "true" : "false");
        }

        for (int t = 0; t < num_thread_configs; t++) {
            int threads = valid_thread_counts[t];

            zero_matrix(C_test, n);
            start_time = omp_get_wtime();
            matmul_omp(A, B, C_test, n, threads);
            double omp_time = omp_get_wtime() - start_time;
            double speedup = serial_time / omp_time;
            double efficiency = (speedup / threads) * 100.0;
            double gflops = (2.0 * n * n * n) / (omp_time * 1e9);
            verified = verify_matrix(C_ref, C_test, n);

            printf("  [OpenMP Naive  %2dT] Time: %8.4f s | Speedup: %5.2fx | Eff: %5.1f%% | GFLOPS: %6.2f | Status: %s\n",
                   threads, omp_time, speedup, efficiency, gflops, verified ? "PASSED" : "FAILED");

            if (csv_file) {
                fprintf(csv_file, "%d,OpenMP_Naive,%d,%.6f,%.2f,%.2f,%.2f,%d\n",
                        n, threads, omp_time, speedup, efficiency, gflops, verified);
            }
            if (json_file) {
                fprintf(json_file, ",\n  {\"matrix_size\": %d, \"mode\": \"OpenMP_Naive\", \"threads\": %d, \"time_sec\": %.6f, \"speedup\": %.2f, \"efficiency\": %.2f, \"gflops\": %.2f, \"verified\": %s}",
                        n, threads, omp_time, speedup, efficiency, gflops, verified ? "true" : "false");
            }

            zero_matrix(C_test, n);
            start_time = omp_get_wtime();
            matmul_transpose_omp(A, B, C_test, n, threads);
            double omp_trans_time = omp_get_wtime() - start_time;
            double trans_speedup = serial_time / omp_trans_time;
            double trans_efficiency = (trans_speedup / threads) * 100.0;
            double trans_gflops = (2.0 * n * n * n) / (omp_trans_time * 1e9);
            verified = verify_matrix(C_ref, C_test, n);

            printf("  [OpenMP Transp %2dT] Time: %8.4f s | Speedup: %5.2fx | Eff: %5.1f%% | GFLOPS: %6.2f | Status: %s\n",
                   threads, omp_trans_time, trans_speedup, trans_efficiency, trans_gflops, verified ? "PASSED" : "FAILED");

            if (csv_file) {
                fprintf(csv_file, "%d,OpenMP_Transpose,%d,%.6f,%.2f,%.2f,%.2f,%d\n",
                        n, threads, omp_trans_time, trans_speedup, trans_efficiency, trans_gflops, verified);
            }
            if (json_file) {
                fprintf(json_file, ",\n  {\"matrix_size\": %d, \"mode\": \"OpenMP_Transpose\", \"threads\": %d, \"time_sec\": %.6f, \"speedup\": %.2f, \"efficiency\": %.2f, \"gflops\": %.2f, \"verified\": %s}",
                        n, threads, omp_trans_time, trans_speedup, trans_efficiency, trans_gflops, verified ? "true" : "false");
            }
        }

        free(A);
        free(B);
        free(C_ref);
        free(C_test);
        printf("---------------------------------------------------------------------------------\n");
    }

    if (csv_file) fclose(csv_file);
    if (json_file) {
        fprintf(json_file, "\n]\n");
        fclose(json_file);
    }

    printf("\nBenchmark selesai. Hasil disimpan di 'benchmark_results.csv' dan 'benchmark_results.json'\n");
    return 0;
}
