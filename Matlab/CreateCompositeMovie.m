
SetupDatasets();
data = dataset(7);

add_title = false;

aligned_name = [data.file '-aligned-stack.tif'];
unaligned_name = [data.file '-unaligned-stack.tif'];
aligned_presv_name = [data.file '-aligned-int-presv-stack.tif'];
coverage_name = [data.file '-coverage-stack.tif'];
ma_name = [data.file '-ma-image.tif'];
unaligned_ma_name = [data.file '-unaligned-ma-image.tif'];
output_folder = 'C:\Users\CIMLab\Dropbox\00 Papers in preparation\Motion Compensation Paper\Movies\';

aligned = ReadTifStack(aligned_name);
unaligned = ReadTifStack(unaligned_name);
aligned_presv = ReadTifStack(aligned_presv_name);
ma = ReadTifStack(ma_name);
unaligned_ma = ReadTifStack(unaligned_ma_name);

points_file = [data.file '_realignment.csv'];
points = ReadPointFile(points_file,data.n_px(1),data.zoom,data.scan_rate);

%%


if exist(coverage_name,'file')
    coverage = ReadTifStack(coverage_name);
else
    coverage = ones(size(aligned),'uint16');
end

unaligned(~isfinite(unaligned)) = 0;

sel = 1:size(aligned,3);

aligned = aligned(:,:,sel);
unaligned = unaligned(:,:,sel);
aligned_presv = aligned_presv(:,:,sel);
coverage = coverage(:,:,sel);


% For NADH dataset, normalise frame intensity as power is adjusted through
% movie. Otherwise merging green and magenta images doesn't really work
if strcmp(data.title_ext,'05')
    norm = sum(sum(unaligned,1),2);
    norm = norm / norm(1);
    aligned = double(aligned) ./ norm;
    unaligned = double(unaligned) ./ norm;
    aligned_presv = double(aligned_presv) ./ norm;
end

% For Imperial dataset we need to skip first 5 frames
if strcmp(data.title_ext,'07')
    aligned = aligned(:,:,6:end);
    unaligned = unaligned(:,:,6:end);
    aligned_presv = aligned_presv(:,:,6:end);
    coverage = coverage(:,:,6:end);
    ma = ma(:,:,2:end);
    unaligned_ma = unaligned_ma(:,:,2:end);
end


n = size(ma,3);
%%
I_scale = 255 / data.I_max;
um_per_px = 1.2442e+03 / (size(aligned,1) * data.zoom);
dt_frame = size(aligned,1) / data.scan_rate;

[~,filename] = fileparts(data.file); 

if add_title
    extra_string = '';
else
    extra_string = '-notitle';
end

writer = VideoWriter([output_folder filename '-composite' extra_string '.avi']);
writer.FrameRate = data.frame_rate;
open(writer);

aligned_accum = zeros(data.n_px/data.resize);
unaligned_accum = zeros(data.n_px/data.resize);
coverage_accum = zeros(data.n_px/data.resize);

title_file = ['X:\Sean Warren\Motion correction paper\Paper\Movies\Movie Titles-' data.title_ext '.png'];
title = imread(title_file);

pad = 30;

if strcmp(data.title_ext,'06')
    f0 = imread('frame520.png');
else
    f0 = imread('frame542.png');
end

top_row_0 = double([unaligned(:,:,data.reference_idx), unaligned(:,:,data.reference_idx)]) * I_scale;
top_row_0 = repmat(top_row_0,[1 1 3]);
top_row_0(:,:,2) = 0; % magenta

figure(3)
clf

n_fade = 0.5 * data.frame_rate;
n_title = 3 * data.frame_rate;

if add_title
    for i=1:n_title
        writeVideo(writer,title);
        imshow(title);
        drawnow
    end
end

for i=1:n_fade
    f = uint8(double(title) * (n_fade-i) / n_fade);
    writeVideo(writer,f);
    imshow(f);
    drawnow
end

for i=1:n
    
    accept_frame = ~isfinite(data.threshold) || points.correlation(i) > data.threshold;
    
    if accept_frame
        aligned_accum = aligned_accum + double(aligned_presv(:,:,i));
        coverage_accum = coverage_accum + double(coverage(:,:,i));
        aligned_i = aligned(:,:,i);        
    else
        aligned_i = zeros(size(aligned(:,:,i)),'uint8');
    end 
        
    
    unaligned_accum = unaligned_accum + double(unaligned(:,:,i));
    
    top_row_1 = double([unaligned(:,:,i) aligned_i]) * I_scale;
    top_row_1 = repmat(top_row_1,[1 1 3]);
    top_row_1(:,:,[1 3]) = 0; % green
        
    top_row = uint8((top_row_0 + top_row_1) / 2);
    
    b1 = ShowFLIM(ma(:,:,i)-data.irf_ma,aligned_accum./coverage_accum*n, data.tau_lim, data.I_lim, false);
    b1 = uint8(b1 * 255);
    
    b2 = ShowFLIM(unaligned_ma(:,:,i)-data.irf_ma,unaligned_accum/i*n, data.tau_lim, data.I_lim, false);
    b2 = uint8(b2 * 255);
    
    bottom_row = [b2 b1];
    
    f = uint8([top_row; bottom_row]);

    if (data.resize > 1)
        f = RebinStack(f,data.resize);
        f = f / data.resize^2;
        f = uint8(f);
    end
    
    f = AddScaleBar(f,um_per_px * data.resize,data.scalebar,5);
    
    sz = size(f);    
    fx = f0;
    fx(pad+1:end-pad,pad+1:pad+size(f,2),:) = f;
    f = fx;

    mean_counts = ceil(mean(aligned_accum(:)));
    str = ['Photons/px: ' num2str(mean_counts)];
    f = insertText(f, [pad+sz(1)*0.75 size(f,1)], str, 'Font','HelveticaLTStd-Bold','FontSize',15,... 
        'BoxColor', 'black', 'TextColor','white', 'AnchorPoint', 'CenterBottom');
    
    t = round((i-1) * dt_frame);
    str = ['Time: ' num2str(t) ' s'];
    f = insertText(f, [pad+sz(1)*0.25 size(f,1)], str, 'Font','HelveticaLTStd-Bold','FontSize',15,... 
        'BoxColor', 'black', 'TextColor','white', 'AnchorPoint', 'CenterBottom');
    
    if ~accept_frame
        f = insertText(f, [pad+sz(1)*0.75-3 pad*2], ' Rejected Frame ', 'Font','HelveticaLTStd-Bold','FontSize',15,... 
        'BoxColor', 'Red', 'TextColor','white', 'AnchorPoint', 'CenterBottom', 'BoxOpacity', 1);
    end
    
    imshow(f)
    daspect([1 1 1]);
    caxis([0 10])
    drawnow
    for rep=1:4
        writeVideo(writer,f);
    end
end
close(writer);