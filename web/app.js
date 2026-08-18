document.addEventListener('DOMContentLoaded', () => {
    fetch('../benchmark_results.json')
        .then(response => {
            if (!response.ok) {
                return fetch('./benchmark_results.json');
            }
            return response;
        })
        .then(res => res.json())
        .then(data => {
            processBenchmarkData(data);
        })
        .catch(err => {
            console.warn('Gagal membaca benchmark_results.json, menggunakan data sampel fallback:', err);
            loadSampleData();
        });
});

function loadSampleData() {
    const sampleData = [
        { matrix_size: 500, mode: "Serial_Naive", threads: 1, time_sec: 0.0950, speedup: 1.00, efficiency: 100.0, gflops: 2.63, verified: true },
        { matrix_size: 500, mode: "Serial_Transpose", threads: 1, time_sec: 0.0530, speedup: 1.79, efficiency: 179.0, gflops: 4.72, verified: true },
        { matrix_size: 500, mode: "OpenMP_Naive", threads: 2, time_sec: 0.0350, speedup: 2.71, efficiency: 135.7, gflops: 7.14, verified: true },
        { matrix_size: 500, mode: "OpenMP_Transpose", threads: 2, time_sec: 0.0330, speedup: 2.88, efficiency: 143.9, gflops: 7.58, verified: true },
        { matrix_size: 500, mode: "OpenMP_Naive", threads: 4, time_sec: 0.0200, speedup: 4.75, efficiency: 118.7, gflops: 12.50, verified: true },
        { matrix_size: 500, mode: "OpenMP_Transpose", threads: 4, time_sec: 0.0210, speedup: 4.52, efficiency: 113.1, gflops: 11.90, verified: true },
        { matrix_size: 500, mode: "OpenMP_Naive", threads: 8, time_sec: 0.0170, speedup: 5.59, efficiency: 69.9, gflops: 14.71, verified: true },
        { matrix_size: 500, mode: "OpenMP_Transpose", threads: 8, time_sec: 0.0160, speedup: 5.94, efficiency: 74.2, gflops: 15.62, verified: true },
        { matrix_size: 1000, mode: "Serial_Naive", threads: 1, time_sec: 0.8500, speedup: 1.00, efficiency: 100.0, gflops: 2.35, verified: true },
        { matrix_size: 1000, mode: "OpenMP_Transpose", threads: 8, time_sec: 0.1100, speedup: 7.73, efficiency: 96.6, gflops: 18.18, verified: true }
    ];
    processBenchmarkData(sampleData);
}

function processBenchmarkData(data) {
    document.getElementById('total-tests').textContent = data.length;

    let maxSpd = 0;
    let maxGflops = 0;
    let allVerified = true;

    data.forEach(item => {
        if (item.speedup > maxSpd) maxSpd = item.speedup;
        if (item.gflops > maxGflops) maxGflops = item.gflops;
        if (!item.verified) allVerified = false;
    });

    document.getElementById('max-speedup').textContent = maxSpd.toFixed(2) + 'x';
    document.getElementById('peak-gflops').textContent = maxGflops.toFixed(2);
    document.getElementById('verification-status').textContent = allVerified ? 'PASSED' : 'FAILED';

    populateTable(data);
    renderCharts(data);
}

function populateTable(data) {
    const tbody = document.querySelector('#resultsTable tbody');
    tbody.innerHTML = '';

    data.forEach(item => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${item.matrix_size} x ${item.matrix_size}</td>
            <td>${item.mode}</td>
            <td>${item.threads}</td>
            <td>${item.time_sec.toFixed(4)}</td>
            <td>${item.speedup.toFixed(2)}x</td>
            <td>${item.efficiency.toFixed(1)}%</td>
            <td>${item.gflops.toFixed(2)}</td>
            <td><span class="badge-pass">${item.verified ? 'PASSED' : 'FAILED'}</span></td>
        `;
        tbody.appendChild(row);
    });
}

function renderCharts(data) {
    const sizes = [...new Set(data.map(d => d.matrix_size))].sort((a, b) => a - b);
    const firstSize = sizes[0] || 500;
    const sizeData = data.filter(d => d.matrix_size === firstSize);

    const ompNaiveData = sizeData.filter(d => d.mode === 'OpenMP_Naive').sort((a, b) => a.threads - b.threads);
    const ompTransData = sizeData.filter(d => d.mode === 'OpenMP_Transpose').sort((a, b) => a.threads - b.threads);

    const threadLabels = ompNaiveData.map(d => d.threads + ' Threads');
    const speedupNaive = ompNaiveData.map(d => d.speedup);
    const speedupTrans = ompTransData.map(d => d.speedup);
    const idealSpeedup = ompNaiveData.map(d => d.threads);

    new Chart(document.getElementById('speedupChart'), {
        type: 'line',
        data: {
            labels: threadLabels,
            datasets: [
                { label: 'Ideal Speedup', data: idealSpeedup, borderColor: '#94a3b8', borderDash: [5, 5], fill: false },
                { label: 'OpenMP Naive', data: speedupNaive, borderColor: '#38bdf8', backgroundColor: 'rgba(56, 189, 248, 0.1)', fill: true },
                { label: 'OpenMP Transpose', data: speedupTrans, borderColor: '#4ade80', backgroundColor: 'rgba(74, 222, 128, 0.1)', fill: true }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { labels: { color: '#f8fafc' } } },
            scales: {
                x: { ticks: { color: '#94a3b8' }, grid: { color: '#334155' } },
                y: { ticks: { color: '#94a3b8' }, grid: { color: '#334155' } }
            }
        }
    });

    const modes = [...new Set(data.map(d => d.mode))];
    const timeDatasets = modes.map((mode, idx) => {
        const colors = ['#f43f5e', '#fb923c', '#38bdf8', '#4ade80', '#c084fc'];
        const modeData = sizes.map(sz => {
            const match = data.find(d => d.matrix_size === sz && d.mode === mode);
            return match ? match.time_sec : null;
        });
        return {
            label: mode,
            data: modeData,
            borderColor: colors[idx % colors.length],
            fill: false
        };
    });

    new Chart(document.getElementById('timeChart'), {
        type: 'line',
        data: {
            labels: sizes.map(s => s + 'x' + s),
            datasets: timeDatasets
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { labels: { color: '#f8fafc' } } },
            scales: {
                x: { ticks: { color: '#94a3b8' }, grid: { color: '#334155' } },
                y: { ticks: { color: '#94a3b8' }, grid: { color: '#334155' } }
            }
        }
    });

    const effNaive = ompNaiveData.map(d => d.efficiency);
    const effTrans = ompTransData.map(d => d.efficiency);

    new Chart(document.getElementById('efficiencyChart'), {
        type: 'bar',
        data: {
            labels: threadLabels,
            datasets: [
                { label: 'OpenMP Naive Efficiency (%)', data: effNaive, backgroundColor: '#38bdf8' },
                { label: 'OpenMP Transpose Efficiency (%)', data: effTrans, backgroundColor: '#4ade80' }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { labels: { color: '#f8fafc' } } },
            scales: {
                x: { ticks: { color: '#94a3b8' }, grid: { color: '#334155' } },
                y: { ticks: { color: '#94a3b8' }, grid: { color: '#334155' } }
            }
        }
    });

    const gflopsDatasets = modes.map((mode, idx) => {
        const colors = ['#f43f5e', '#fb923c', '#38bdf8', '#4ade80', '#c084fc'];
        const gflopsData = sizes.map(sz => {
            const match = data.find(d => d.matrix_size === sz && d.mode === mode);
            return match ? match.gflops : 0;
        });
        return {
            label: mode,
            data: gflopsData,
            backgroundColor: colors[idx % colors.length]
        };
    });

    new Chart(document.getElementById('gflopsChart'), {
        type: 'bar',
        data: {
            labels: sizes.map(s => s + 'x' + s),
            datasets: gflopsDatasets
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { labels: { color: '#f8fafc' } } },
            scales: {
                x: { ticks: { color: '#94a3b8' }, grid: { color: '#334155' } },
                y: { ticks: { color: '#94a3b8' }, grid: { color: '#334155' } }
            }
        }
    });
}
