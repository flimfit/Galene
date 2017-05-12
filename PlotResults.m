output_folder = ['X:\Sean Warren\Motion correction paper\Paper\Figures\']

% Set up file names
file = 'X:\Sean Warren\Motion correction paper\2017-03-31 ANRN361 Rac1-on window pancreas gut\01 pancreas _1_1.pt3';
file = 'X:\Sean Warren\Motion correction paper\SimulatedData\Simulated Data Amplitude=4_44444 Frequency=0_7 Frames=25 _001_001.ffd';
file = 'X:\Sean Warren\Motion correction paper\Sim23\Simulated Data Amplitude=20 Frequency=0_7 Frames=25 _001_001.ffd';
%file = 'X:\Sean Warren\Motion correction paper\Rac1 on crypts\2017-05-02 ANRN354 crypts\2017-05-02 ANRN354 Rac1-on crypts Intravital\01 control crypts 6x_2_1.pt3';
%file = 'X:\Sean Warren\Motion correction paper\Rac1 on crypts\2017-04-13 ANRN351 Rac1on crypts\02 Rac1-on gut drug=PMA_11_1.pt3';
%file = 'X:\Sean Warren\Motion correction paper\2017-04-05 ANRN361 Rac1-on optical windows\02b pancreas 1000hz zoom=6 _10_1.pt3';
%file = 'X:\Sean Warren\Motion correction paper\PRex1 virgin mammary 16wk\160623 - PRex1-RacFRET no1\Mammary gland 1137_8_1_001.ffd'
%file = 'X:\Sean Warren\Motion correction paper\2017-03-03 ANRN360 Rac1-on gut\02 Rac1-on gut bidirectional_14_1_001.ffd'
[~,filename,ext] = fileparts(file); 


points_file = strrep(file,ext,'_realignment.csv');
aligned_stack_file = strrep(file,ext,'-aligned-stack.tif');
unaligned_stack_file = strrep(file,ext,'-unaligned-stack.tif');

% Read in images
aligned = ReadTifStack(aligned_stack_file);
unaligned = ReadTifStack(unaligned_stack_file);

addpath('Build2017/FLIMreader/Source/FLIMreaderMex/Release/')

%%

r = FLIMreaderMex(file);
m = FLIMreaderMex(r,'GetMetadata')
FLIMreaderMex(r,'Delete');
clear FLIMreaderMex

%%
set(0,'DefaultAxesFontSize',8)

zoom = 6;
scan_speed = 1000;

data = ReadPointFile(points_file,zoom,scan_speed);


fig = figure(1);
subplot(3,1,[1 2])
plot(data.t(:),real(data.points(:)),'.-');
hold on;
plot(data.t(:),imag(data.points(:)),'.-');
hold off;
set(gca,'box','off','TickDir','out');
ylabel('Displacement (\mum)');
xlim([0 max(data.t)]);
%ylim([-20 40])
%legend({'x','y'},'Box','off','Location','SouthWest')

subplot(3,1,3);
plot(data.t_frame,data.correlation,'k');
set(gca,'box','off','TickDir','out');
ylabel('Correlation');
xlabel('Time (s)')
ylim([0 1.2])
xlim([0 max(t2)]);
hold on;
plot([0 max(t2)],0.7*[1 1],'r')
hold off;
fig.PaperUnits = 'centimeters';
fig.PaperPosition = [0 0 10 5];
print(fig,[output_folder filename '-traces.pdf'],'-dpdf')

%%
figure(2)
a = aligned(:,:,1:1:end);

im_merge = 0;
col = jet(size(a,3));
for i=1:size(a,3)
   
    im = double(a(:,:,i));
    im = repmat(im,[1 1 3]);
    coli = col(i,:);
    coli = reshape(coli,[1 1 3]);
    im = im .* coli;
    im_merge = im_merge + im;
    
    imagesc(a(:,:,i));
    pause(0.5);
    
end

im_merge = im_merge / size(a,3);

imagesc(im_merge)

%%

downsample = 4;

kern = ones(downsample,downsample);

idx = 1:40:size(aligned,3);
clf(3)
ha = tight_subplot(2, length(idx), 0.005, 0, 0, figure(3));

im0 = unaligned(:,:,1);
im0 = conv2(im0,kern);
im0 = im0(1:downsample:end,1:downsample:end);

for i=1:length(idx)
    
    im1 = unaligned(:,:,idx(i));
    im2 = aligned(:,:,idx(i));

    im1 = conv2(im1,kern);
    im1 = im1(1:downsample:end,1:downsample:end);

    im2 = conv2(im2,kern);
    im2 = im2(1:downsample:end,1:downsample:end);
    
    axes(ha(i))
    imagesc(imfuse(im1,im0));
    daspect([1 1 1]);
    set(gca,'XTick',[],'YTick',[])
    caxis([0 prctile(im1(:),99)]);

    axes(ha(i+length(idx)))

    imagesc(imfuse(im2,im0));
    daspect([1 1 1]);
    set(gca,'XTick',[],'YTick',[])
    caxis([0 prctile(im2(:),99)]);
    %caxis([0 10])
    
end
colormap('gray')
