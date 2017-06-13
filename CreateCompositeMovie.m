
SetupDatasets();
data = dataset(1);

aligned_name = [data.file '-aligned-stack.tif'];
unaligned_name = [data.file '-unaligned-stack.tif'];
aligned_presv_name = [data.file '-aligned-int-presv-stack.tif'];

aligned = ReadTifStack(aligned_name);
unaligned = ReadTifStack(unaligned_name);
aligned_presv = ReadTifStack(aligned_presv_name);

sel = 1:size(aligned,3);

aligned = aligned(:,:,sel);
unaligned = unaligned(:,:,sel);
aligned_presv = aligned_presv(:,:,sel);
n = size(aligned,3);

%%
%{
for i=1:n
   
    subplot(1,n,i)
    
    imagesc(aligned(:,:,i));
    daspect([1 1 1])
    colormap('gray')
end
%}
%%

I_scale = 255 / data.I_max;
um_per_px = 1.2442e+03 / (size(aligned,1) * data.zoom);
dt_frame = size(aligned,1) / data.scan_rate;
scale_bar_length = 50; %um
writer = VideoWriter([data.file '-composite.avi']);
writer.FrameRate = 30;
open(writer);

aligned_accum = 0;
unaligned_accum = 0;



top_row_0 = double([unaligned(:,:,data.reference_idx), unaligned(:,:,data.reference_idx)]) * I_scale;
top_row_0 = repmat(top_row_0,[1 1 3]);
top_row_0(:,:,2) = 0; % magenta

figure(3)
clf
for i=1:n
    unaligned_accum = unaligned_accum + double(unaligned(:,:,i));
    aligned_accum = aligned_accum + double(aligned_presv(:,:,i));

    top_row_1 = double([unaligned(:,:,i) aligned(:,:,i)]) * I_scale;
    top_row_1 = repmat(top_row_1,[1 1 3]);
    top_row_1(:,:,[1 3]) = 0; % green
        
    top_row = uint8((top_row_0 + top_row_1) / 2);
    
    bottom_row = [unaligned_accum aligned_accum] / i * I_scale;
    bottom_row = repmat(bottom_row,[1 1 3]);
    %bottom_row(:,:,[1 3]) = 0;
    f = uint8([top_row; bottom_row]);

    if (data.resize > 1)
        f = RebinStack(f,data.resize);
        f = uint8(f);
    end
    
    f = AddScaleBar(f,um_per_px * data.resize,scale_bar_length,5);
    
    mean_counts = ceil(mean(aligned_accum(:)));
    str = ['Photons/px: ' num2str(mean_counts)];
    f = insertText(f, [size(f,2)/2+10 size(f,1)-10], str, 'BoxColor', 'w', 'TextColor', 'black', 'AnchorPoint', 'LeftBottom');
    
    t = round((i-1) * dt_frame);
    str = [num2str(t) ' s'];
    f = insertText(f, [10 size(f,1)-10], str, 'BoxColor', 'w', 'TextColor', 'black', 'AnchorPoint', 'LeftBottom');
    
    imagesc(f);
    daspect([1 1 1]);
    caxis([0 10])
    drawnow
    writeVideo(writer,f);
end
close(writer);