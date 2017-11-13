folder = 'C:\Users\CIMLab\Documents\User Data\Sean\Motion Correction\FRAP for motion correction\';
file_root = '010417_33004_172_FRAP001';

im_width = 448;
um_per_pixel = 0.118;
dt_frame = 0.6;

[x,y] = GetPointsFromLeicaRoiFile([folder file_root '.roi'], im_width, 0.118 * im_width);

[X,Y] = meshgrid(1:im_width,1:im_width);

sel = X > min(x) & X < max(x) & Y > min(y) & Y < max(y);
imagesc(sel)

file_suffixes = {'.ome.tif', '-aligned-stackreg.ome.tif', '-aligned.ome.tif'};

r = [];
im = {};
for f=1:length(file_suffixes)

    im{f} = double(ReadTifStack([folder file_root file_suffixes{f}]));

    for i=1:size(im{f},3)
       imi = im{f}(:,:,i);
       c = imi(sel);
       r(i,f) = mean(c); 
    end
end
%%
r = r(1:end-1,:);
r0 = mean(r(1:5,:),1);
rn = r ./ r0;



t = (0:(size(rn,1)-1)) * dt_frame;

for i=1:size(rn,2)
    subplot(1,size(rn,2),i)
    plot(t,rn(:,i))
    ylim([0 1.4]);
    box off;
    set(gca,'TickDir','out')
    ylabel('Intensity');
    xlabel('Time [s] ')
end
%set(gcf,'Units','centimeters','PaperPosition',[0,0,14,4]);
%print(gcf,['FLIP-' program '.pdf'],'-dpdf');

%%

downsample = 2;
intensity_norm = 200;

for j=1:size(rn,2)    
    im_re{j} = RebinStack(im{j},downsample) / intensity_norm; 
    
    t_norm = mean(mean(im_re{j},1),2);
    im_re{j} = im_re{j} ./ t_norm * t_norm(1);
    
end

writer = VideoWriter(['FRAP comparison.avi']);
writer.FrameRate = 20;
open(writer);


for i=1:size(rn,1)
    frame = [];
    for j=1:size(rn,2)
        fj = imfuse(im_re{j}(:,:,i), im_re{j}(:,:,1), 'scaling', 'none');
        titles = {'Uncorrected', 'StackReg', 'Galene'};
        fj = insertText(fj, [0 0], titles{j}, 'Font','HelveticaLTStd-Bold','FontSize',15,... 
            'BoxColor', 'black', 'TextColor','white', 'AnchorPoint', 'LeftTop');
        frame = [frame fj];
    end
    
    t = round((i-1) * dt_frame);
    str = ['Time: ' num2str(t) ' s'];
    frame = insertText(frame, [0 size(frame,1)], str, 'Font','HelveticaLTStd-Bold','FontSize',15,... 
        'BoxColor', 'black', 'TextColor','white', 'AnchorPoint', 'LeftBottom');

    frame = AddScaleBar(frame, um_per_pixel * downsample, 20, 5);
    
    writeVideo(writer,frame);
    imagesc(frame);
    caxis([0 80])
    daspect([1 1 1])
    drawnow
end

close(writer)
