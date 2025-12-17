% analyze_results.m
% Post-processing and visualization of Prisoner's Dilemma 

% This script: 
% 1. reads CSV files produced by the C simulation 
% 2. extracts cooperation time series 
% 3. generates all figures used in the final report 

clear; close all; clc;

% -------- PARAMETERS --------

% Noise probabilities explored in the simulatons   
noise_levels = [0.00, 0.01, 0.02, 0.05, 0.10];
P = length(noise_levels);

% Automatically determine project root directory 
% This ensure the script works regardless of where MATLAB is launched from
this_file = mfilename('fullpath');
project_root = fileparts(fileparts(fileparts(this_file)));

% Input and output directories 
data_dir = fullfile(project_root, "data");
fig_dir  = fullfile(project_root, "results", "figures");

if ~exist(fig_dir, 'dir')
    mkdir(fig_dir);
end

% -------- READ DATA --------

% Cell array storing cooperation time series for each noise level 
coop_all = cell(P,1);

% Vector storing final cooperation frction for each noise level 
final_coop = zeros(P,1);

for i = 1:P
    p = noise_levels(i);

    % Construct filename corresponding to noise level 
    fname = fullfile(data_dir, sprintf("pnoise_%.2f.csv", p));

    % Read CSV file (header is ignored automatically)
    data = readmatrix(fname);

    % Column 1 = time steps
    % Column 2 = cooperation fractions
    time = data(:,1);
    coop = data(:,2);

    % Store final cooperation trajectory 
    coop_all{i} = coop(:);

    %Store final cooperation level
    final_coop(i) = coop(end);
end

% ---- FIGURE 1 ----
% Cooperation dynamics vs time (all noise levels on the same plot)

figure; hold on;

% Use distinct colors for each noise level
colors = lines(P);

for i = 1:P
    plot(coop_all{i}, 'LineWidth', 2, 'Color', colors(i,:));
end

xlabel("Time step");
ylabel("Fraction cooperating");
title("Cooperation dynamics for different noise levels");
legend(arrayfun(@(p) sprintf("p_{noise} = %.2f", p), noise_levels, ...
       'UniformOutput', false), 'Location', 'best');
grid on;

saveas(gcf, fullfile(fig_dir, "coop_time_multiple_noise.png"));   

% -------- FIGURE 2 --------
% Final cooperation vs noise
figure;
plot(noise_levels, final_coop,'-o','LineWidth',2);
xlabel("Noise probability");
ylabel("Final cooperation fraction");
title("Final cooperation as a function of noise");
grid on;
saveas(gcf, fullfile(fig_dir,"final_coop_vs_noise.png"));

% -------- FIGURE 3 (HEATMAP) --------
% Heatmap: cooperation across time and noise

% Number of time steps (assumed equal for all noise levels)
T = length(coop_all{1});

% Matrix where rows = time, columns = noise levels 
coop_matrix = zeros(T,P);

for i = 1:P
    coop_matrix(:,i) = coop_all{i};
end

figure;
imagesc(noise_levels,1:T,coop_matrix);
% Ensure time increases upward 
set(gca,'YDir','normal');
colorbar;
xlabel("Noise probability");
ylabel("Time step");
title("Cooperation dynamics across noise levels");
saveas(gcf, fullfile(fig_dir,"coop_heatmap_time_noise.png"));

disp("MATLAB analysis complete. Figures saved to results/figures/");

