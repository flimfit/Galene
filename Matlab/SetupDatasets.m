
root = 'X:\Sean Warren\Motion correction paper\';

% Gut window
dataset(1).file = [root 'FinalData\Gut Window\02 Rac1-on gut bidirectional_14_1_001'];
dataset(1).file = [root 'Final Datasets\Rac1-on crypt window'];
dataset(1).I_max = 6;
dataset(1).zoom = 6;
dataset(1).scan_rate = 1400 * 2; % bidirectional
dataset(1).threshold = 0.8;
dataset(1).n_px = [256 256];
dataset(1).resize = 1;
dataset(1).I_lim = [200 3500];
dataset(1).tau_lim = [1500 3500];
dataset(1).reference_idx = 74;
dataset(1).frame_rate = 30;
dataset(1).title_ext = '01';
dataset(1).irf_ma = 980;
dataset(1).scalebar = 50;

% Pancreas window
dataset(2).file = [root 'FinalData\Pancreas Window\02b pancreas 1000hz zoom=6 _10_1'];
dataset(2).file = [root 'Final Datasets\Rac1-on pancreas window'];
dataset(2).I_max = 3;
dataset(2).zoom = 6;
dataset(2).scan_rate = 1000;
dataset(2).threshold = nan;
dataset(2).n_px = [256 256];
dataset(2).resize = 1;
dataset(2).reference_idx = 1;
dataset(2).I_lim = [100 450];
dataset(2).tau_lim = [1500 3500];
dataset(2).frame_rate = 30;
dataset(2).title_ext = '03';
dataset(2).irf_ma = 1280;
dataset(2).scalebar = 50;

% Ex vivo gut PMA
dataset(3).file = [root 'FinalData\Gut Ex vivo PMA\02 Rac1-on gut drug=PMA_11_1'];
dataset(3).I_max = 1.5;
dataset(3).zoom = 6;
dataset(3).scan_rate = 1000;
dataset(3).threshold = nan;
dataset(3).n_px = [512 512];
dataset(3).tau_lim = [1500 3500];
dataset(3).I_lim = [50 400];
dataset(3).resize = 2;
dataset(3).reference_idx = 1;
dataset(3).frame_rate = 30;
dataset(3).title_ext = '02';
dataset(3).irf_ma = 980;
dataset(3).scalebar = 50;

% Mammary gland
dataset(4).file = [root 'FinalData\Mammary Gland\Mammary gland 1137_8_1_001'];
dataset(4).I_max = 4;
dataset(4).zoom = 2.5;
dataset(4).scan_rate = 600;
dataset(4).threshold = nan;
dataset(4).n_px = [512 512];
dataset(4).tau_lim = [1500 3500];
dataset(4).I_lim = [0 450];
dataset(4).resize = 2;
dataset(4).reference_idx = 1;
dataset(4).frame_rate = 15;
dataset(4).title_ext = '04';
dataset(4).irf_ma = 1350;
dataset(4).scalebar = 100;

% NADH
dataset(5).file = [root 'FinalData\NADH\01 SKPCAkt73 liver zoom=2-25 bidirectional px=256 _29_1'];
dataset(5).I_max = 4;
dataset(5).zoom = 2.25;
dataset(5).scan_rate = 1000 * 2; % bidirectional
dataset(5).threshold = nan; 
dataset(5).n_px = [512 512];
dataset(5).tau_lim = [1000 3500];
dataset(5).I_lim = [200 750];
dataset(5).resize = 2;
dataset(5).reference_idx = 1;
dataset(5).frame_rate = 30;
dataset(5).title_ext = '05';
dataset(5).irf_ma = 500;
dataset(5).scalebar = 100;

% Human skin UQ
dataset(6).file = [root 'FinalData\UQ\FIFO data skin 256 0-85_31_zoom600-P12'];
dataset(6).I_max = 30;
dataset(6).zoom = 3.4775; % => gives um/px=1.3976
dataset(6).scan_rate = 125;
dataset(6).threshold = 0.8;
dataset(6).n_px = [256 256];
dataset(6).resize = 1;
dataset(6).I_lim = [150 1500];
dataset(6).tau_lim = [0 5000];
dataset(6).reference_idx = 26;
dataset(6).frame_rate = 10;
dataset(6).title_ext = '06';
dataset(6).irf_ma = 1760;
dataset(6).scalebar = 100;

% Human skin
dataset(7).file = [root 'FinalData\Imperial\AutoSequence 2015-07-16 12-09-43 FLIM'];
dataset(7).I_max = 50;
dataset(7).zoom = 6.5147; % => gives um/px=50/67
dataset(7).scan_rate = 256;
dataset(7).threshold = nan;
dataset(7).n_px = [256 255];
dataset(7).resize = 1;
dataset(7).I_lim = [0 400];
dataset(7).tau_lim = [0 5000];
dataset(7).reference_idx = 1;
dataset(7).frame_rate = 8;
dataset(7).title_ext = '07';
dataset(7).irf_ma = 1220;
dataset(7).scalebar = 100;

% what fraction of the image should a 50um scale bar take? 
f_50_um = 50 * [dataset.zoom] / 1.2442e+03;

%um_per_px = 1.2442e+03 / (n_px * zoom);