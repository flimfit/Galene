clear pi
output_folder = ['X:\Sean Warren\Motion correction paper\Paper\Figures\'];

Amplitude=4.44444;
Frequency=1.33333;
Angle=0;

tmin = 2;
tmax = 4;

tonum = @(x) strrep(num2str(x,6),'.','_');

file = ['X:\Sean Warren\Motion correction paper\Simulated Data 4\Sim Amplitude=' tonum(Amplitude) ...
        ' Frequency=' tonum(Frequency) ' Angle=' num2str(Angle) ' Frames=50 _001_001'];

data.file = file;
data.I_max = 3;
data.zoom = nan;
data.scan_rate = 1000;
data.threshold = nan;
data.n_px = 256;
data.resize = 1;
    
    
[~,filename,ext] = fileparts(data.file); 
points_file = [data.file '_realignment.csv'];


points = ReadPointFile(points_file,data.n_px,data.zoom,data.scan_rate);

t_sim = linspace(min(points.t_unscaled(:)),max(points.t_unscaled(:)),10e3);
t_sim_plot = linspace(min(points.t(:)),max(points.t(:)),10e3);

simulated_x = Amplitude * cos(Angle * pi / 180) * - sin(2*pi*Frequency*t_sim);
simulated_y = Amplitude * sin(Angle * pi / 180) * + cos(2*pi*Frequency*t_sim);


tmin = 0;
tmax = max(points.t(:));


set(0,'DefaultAxesFontSize',8)
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
    plot(points.t(:),real(points.points(:)),'x');
    hold on;
    plot(points.t(:),imag(points.points(:)),'x');
end
plot(t_sim_plot,simulated_x(:),'-')
plot(t_sim_plot,simulated_y(:),'-')

hold off;
set(gca,'box','off','TickDir','out');
ylabel('Displacement (\mum)');
xlim([tmin tmax]);
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
