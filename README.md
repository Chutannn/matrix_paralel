# Project Komputasi Paralel: Pemrosesan Matriks dengan OpenMP (Bahasa C)

Proyek ini dibuat untuk membandingkan performa algoritma serial dan paralel menggunakan OpenMP pada pemrosesan matriks (khususnya perkalian matriks $N \times N$ skala besar).

## Fitur Utama

1. **Algoritma Perkalian Matriks**:
   - Serial Naive ($O(N^3)$)
   - Serial Transpose ($O(N^3)$ teroptimasi cache)
   - OpenMP Naive ($1, 2, 4, 8, 16$ threads)
   - OpenMP Transpose ($1, 2, 4, 8, 16$ threads)
2. **Pengukuran Metrik Performa**:
   - Waktu Eksekusi (detik) menggunakan `omp_get_wtime()`
   - Speedup ($S = T_{serial} / T_{paralel}$)
   - Efisiensi Paralel ($E = S / p \times 100\%$)
   - Computation Throughput (GFLOPS)
3. **Verifikasi Kebenaran**:
   - Pengecekan otomatis kesesuaian nilai matriks hasil serial vs paralel ($\epsilon = 10^{-4}$).
4. **Ekspor Data & Visualisasi Web**:
   - Hasil eksekusi disimpan otomatis ke file `benchmark_results.csv` dan `benchmark_results.json`.
   - Dashboard web berbasis HTML/CSS/JS (Chart.js) untuk visualisasi grafik.

---

## Struktur Direktori

```text
matrix_paralel/
├── src/
│   └── matrix_omp.c
├── web/
│   ├── index.html
│   ├── styles.css
│   └── app.js
├── Makefile
├── benchmark_results.csv
├── benchmark_results.json
└── README.md
```

---

## Cara Kompilasi dan Eksekusi

### 1. Prasyarat System
- GCC Compiler yang mendukung OpenMP (`-fopenmp`).

### 2. Kompilasi
Kompilasi langsung dengan GCC:
```bash
gcc -O3 -fopenmp -Wall src/matrix_omp.c -o matrix_omp.exe
```

### 3. Menjalankan Benchmark
Eksekusi pengujian default (matriks $500 \times 500$, $1500 \times 1500$, dan $3000 \times 3000$):
```bash
./matrix_omp.exe
```

Atau menjalankan pengujian dengan ukuran matriks kustom (misal $1000 \times 1000$):
```bash
./matrix_omp.exe 1000
```

---

## Menjalankan Web Visualizer

Untuk melihat dashboard visualisasi grafik hasil benchmark:
1. Buka file `web/index.html` langsung di browser, atau
2. Jalankan HTTP server lokal sederhana (misal menggunakan Python):
   ```bash
   python -m http.server 8000
   ```
   Lalu akses `http://localhost:8000/web/` di browser.

---

## Ringkasan Hasil Pengujian ($N = 500$, $1500$, dan $3000$)

| Ukuran Matriks | Mode | Thread | Waktu (detik) | Speedup | Efisiensi (%) | GFLOPS | Status Verifikasi |
|---|---|---|---|---|---|---|---|
| 500 x 500 | Serial Naive | 1 | ~0.06 s | 1.00x | 100.0% | 3.9 GFLOPS | PASSED |
| 500 x 500 | OpenMP Transpose | 16 | ~0.01 s | ~5.2x | 32.8% | 20.8 GFLOPS | PASSED |
| 1500 x 1500 | Serial Naive | 1 | ~2.31 s | 1.00x | 100.0% | 2.9 GFLOPS | PASSED |
| 1500 x 1500 | OpenMP Transpose | 16 | ~0.31 s | ~7.3x | 45.4% | 21.1 GFLOPS | PASSED |
| 3000 x 3000 | Serial Naive | 1 | ~18.5 s | 1.00x | 100.0% | 2.9 GFLOPS | PASSED |
| 3000 x 3000 | OpenMP Transpose | 16 | ~2.40 s | ~7.7x | 48.1% | 22.5 GFLOPS | PASSED |

### Analisis Singkat
- Kombinasi **OpenMP + Cache Optimization (Transpose)** memberikan peningkatan performa tertinggi (hingga **7.7x - 8.8x speedup** pada 16 threads).
- Pada ukuran matriks $3000 \times 3000$, perbedaan waktu menjadi sangat terlihat jelas (Serial memakan waktu ~18.5 detik, sedangkan OpenMP Transpose hanya butuh ~2.4 detik).
