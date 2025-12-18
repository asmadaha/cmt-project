#!/bin/bash

# Compile C code
gcc -O3 -std=c11 src/c/pd_simulation.c -o bin/pd_simulation -lm

# Run simulations
./bin/pd_simulation 0.00 data/pnoise_0.00.csv
./bin/pd_simulation 0.01 data/pnoise_0.01.csv
./bin/pd_simulation 0.02 data/pnoise_0.02.csv
./bin/pd_simulation 0.05 data/pnoise_0.05.csv
./bin/pd_simulation 0.10 data/pnoise_0.10.csv

# Run MATLAB analysis (Linux VDI compatible)
/usr/local/bin/matlab-2021b -nodisplay -nosplash -batch "addpath('src/matlab'); analyze_results"
