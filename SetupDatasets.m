
root = 'X:\Sean Warren\Motion correction paper\';

% Gut window
dataset(1).file = [root 'FinalData\Gut Window\02 Rac1-on gut bidirectional_14_1_001'];
dataset(1).I_max = 6;
dataset(1).zoom = 6;
dataset(1).scan_rate = 1400 * 2; % bidirectional
dataset(1).threshold = 0.8;
dataset(1).n_px = 256;
dataset(1).resize = 1;
dataset(1).I_lim = [50 800];
dataset(1).tau_lim = [1500 3500];
dataset(1).reference_idx = 74;

% Pancreas window
dataset(2).file = [root 'FinalData\02b pancreas 1000hz zoom=6 _10_1'];
dataset(2).I_max = 3;
dataset(2).zoom = 6;
dataset(2).scan_rate = 1000;
dataset(2).threshold = nan;
dataset(2).n_px = 256;
dataset(2).resize = 1;
dataset(2).reference_idx = 1;

% Ex vivo gut PMA
dataset(3).file = [root 'FinalData\02 Rac1-on gut drug=PMA_11_1'];
dataset(3).I_max = 8;
dataset(3).zoom = 6;
dataset(3).scan_rate = 1000;
dataset(3).threshold = nan;
dataset(3).n_px = 512;
dataset(3).resize = 2;
dataset(3).reference_idx = 1;

% Mammary gland
dataset(4).file = [root 'FinalData\Mammary gland 1137_8_1_001'];
dataset(4).I_max = 16;
dataset(4).zoom = 2.5;
dataset(4).scan_rate = 600;
dataset(4).threshold = nan;
dataset(4).n_px = 512;
dataset(4).resize = 2;
dataset(4).reference_idx = 1;

% NADH
dataset(5).file = [root 'FinalData\NADH\01 SKPCAkt73 liver zoom=2-25 bidirectional px=256 _29_1'];
dataset(5).I_max = 16;
dataset(5).zoom = 2.25;
dataset(5).scan_rate = 1000 * 2; % bidirectional
dataset(5).threshold = nan; 
dataset(5).n_px = 512;
dataset(5).resize = 2;
dataset(5).reference_idx = 1;


% Human skin
dataset(6).file = [root 'Imperial\AutoSequence 2015-07-16 12-09-43 FLIM'];
dataset(6).I_max = 50;
dataset(6).zoom = 6.5147; % => gives um/px=50/67
dataset(6).scan_rate = 256;
dataset(6).threshold = nan;
dataset(6).n_px = 256;
dataset(6).resize = 1;
dataset(6).I_lim = [140 481];
dataset(6).tau_lim = [1000 3500];
dataset(6).reference_idx = 1;


% what fraction of the image should a 50um scale bar take? 
f_50_um = 50 * [dataset.zoom] / 1.2442e+03;

%um_per_px = 1.2442e+03 / (n_px * zoom);