clear pi
output_folder = ['X:\Sean Warren\Motion correction paper\Paper\Figures\'];

Amplitude=35.5556;
Frequency=1.55556;
Angle=90;

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

Amplitude_scaled = 2 * Amplitude / data.n_px * 100 % Convert to percentage of FOV
Frequency_scaled = Frequency * data.scan_rate / data.n_px % Convert from relative -> Hz

    
[~,filename,ext] = fileparts(data.file); 
points_file = [data.file '_realignment.csv'];


points = ReadPointFile(points_file,data.n_px,data.zoom,data.scan_rate);

t_sim = linspace(min(points.t_unscaled(:)),max(points.t_unscaled(:)),10e3);
t_sim_plot = linspace(min(points.t(:)),max(points.t(:)),10e3);

simulated_x = Amplitude * cos(Angle * pi / 180) * - sin(2*pi*Frequency*t_sim) + 0.5 * cos(Angle * pi / 180);
simulated_y = Amplitude * sin(Angle * pi / 180) * + cos(2*pi*Frequency*t_sim) - 0.5 * sin(Angle * pi / 180);


tmin = 0;
tmax = max(points.t(:));

%tmin = 1;
%tmax = 3;

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
    plot(points.t(:),real(points.points(:)),'x','MarkerSize',3,'Color',col(1,:));
    hold on;
    plot(points.t(:),imag(points.points(:)),'x','MarkerSize',3,'Color',col(2,:));
end
plot(t_sim_plot,simulated_x(:),'-','Color',0.5 + col(1,:) / 2)
plot(t_sim_plot,simulated_y(:),'-','Color',0.5 + col(2,:) / 2)

hold off;
set(gca,'box','off','TickDir','out');
ylabel('Displacement (\mum)');
xlim([tmin tmax]);
ylim([-40 40])


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

if tmax == max(points.t(:))
    fig.PaperPosition = [0 0 10 5];
    print(fig,[output_folder filename '-traces.pdf'],'-dpdf','-painters')
else
    fig.PaperPosition = [0 0 5 3.5];
    print(fig,[output_folder filename '-traces ' num2str(tmin) '-' num2str(tmax) '.pdf'],'-dpdf','-painters')
end    

ShowIntensityMergedSimulations
