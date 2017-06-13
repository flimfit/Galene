output_folder = ['X:\Sean Warren\Motion correction paper\Paper\Figures\']

%{
% Set up file names
file = 'X:\Sean Warren\Motion correction paper\2017-03-31 ANRN361 Rac1-on window pancreas gut\01 pancreas _1_1.pt3';
file = 'X:\Sean Warren\Motion correction paper\SimulatedData\Simulated Data Amplitude=4_44444 Frequency=0_7 Frames=25 _001_001.ffd';
file = 'X:\Sean Warren\Motion correction paper\Sim23\Simulated Data Amplitude=20 Frequency=0_7 Frames=25 _001_001.ffd';
%file = 'X:\Sean Warren\Motion correction paper\Rac1 on crypts\2017-05-02 ANRN354 crypts\2017-05-02 ANRN354 Rac1-on crypts Intravital\01 control crypts 6x_2_1.pt3';
%file = 'X:\Sean Warren\Motion correction paper\Rac1 on crypts\2017-04-13 ANRN351 Rac1on crypts\02 Rac1-on gut drug=PMA_11_1.pt3';
%file = 'X:\Sean Warren\Motion correction paper\2017-04-05 ANRN361 Rac1-on optical windows\02b pancreas 1000hz zoom=6 _10_1.pt3';
%file = 'X:\Sean Warren\Motion correction paper\PRex1 virgin mammary 16wk\160623 - PRex1-RacFRET no1\Mammary gland 1137_8_1_001.ffd'
%file = 'X:\Sean Warren\Motion correction paper\2017-03-03 ANRN360 Rac1-on gut\02 Rac1-on gut bidirectional_14_1_001.ffd'

file = 'X:\Sean Warren\Motion correction paper\Simulated Data 4\Sim Amplitude=17_7778 Frequency=1_33333 Angle=0 Frames=50 _001_001';
%}

SetupDatasets();
data = dataset(1);

[~,filename,ext] = fileparts(data.file); 

points_file = [data.file '_realignment.csv'];

set(0,'DefaultAxesFontSize',8)

points = ReadPointFile(points_file,data.n_px,data.zoom,data.scan_rate);

%{
Amplitude=17.7778;
Frequency=1.33333;

simulated = Amplitude * - sin(2*pi*Frequency*data.t_unscaled);

tmin = 2;
tmax = 4;
%}

tmin = 0;
tmax = max(points.t(:));


fig = figure(1);
subplot(3,1,[1 2])

col = lines(5);

if isfinite(data.threshold)
    
    sel = points.correlation' > data.threshold;
    sel = repmat(sel,[size(points.points,1) 1]);
    
    x = points.t;
    xa = x;
    xa(~sel) = nan;
    xb = x;
    xb(sel) = nan;
    y1 = real(points.points);
    y2 = imag(points.points);
    
    plot(xa,y1,'-','Color',col(1,:));
    hold on;
    plot(xa,y2,'-','Color',col(2,:));
    plot(xb,y1,'-','Color',1-0.4*(1-col(1,:)));
    plot(xb,y2,'-','Color',1-0.4*(1-col(2,:)));
    
else
    plot(points.t(:),real(points.points(:)),'-');
    hold on;
    plot(points.t(:),imag(points.points(:)),'-');
end
%plot(data.t(:),simulated(:),'-')

hold off;
set(gca,'box','off','TickDir','out');
ylabel('Displacement (\mum)');
xlim([tmin tmax]);
%ylim([-200 200]);
subplot(3,1,3);
plot(points.t_frame,points.correlation);
hold on;
plot(points.t_frame,points.unaligned_correlation);
set(gca,'box','off','TickDir','out');
ylabel('Correlation');
xlabel('Time (s)')
ylim([0 1.2])
xlim([tmin tmax]);
if isfinite(data.threshold)
    hold on;
    plot([tmin tmax],data.threshold*[1 1],'k--');
end
xlim([tmin tmax])
hold off;
fig.PaperUnits = 'centimeters';
fig.PaperPosition = [0 0 10 5];
print(fig,[output_folder filename '-traces.pdf'],'-dpdf','-painters')
