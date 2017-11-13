output_folder = ['X:\Sean Warren\Motion correction paper\Paper\Figures\']

SetupDatasets();
data = dataset(7);

[~,filename,ext] = fileparts(data.file); 

points_file = [data.file '_realignment.csv'];

set(0,'DefaultAxesFontSize',8)

points = ReadPointFile(points_file,data.n_px(1),data.zoom,data.scan_rate);

tmin = 0;
tmax = max(points.t(:));

points.t_frame = points.t_frame + 0.5*points.t_frame(2);

%points.t = points.t - 5;
%points.t_frame = points.t_frame - 5;
%tmax = 10;


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
%ylabel('Displacement (\mum)');
xlim([tmin tmax]);
%ylim([-200 200]);
subplot(3,1,3);
plot(points.t_frame,points.correlation);
hold on;
plot(points.t_frame,points.unaligned_correlation);
set(gca,'box','off','TickDir','out');
%ylabel('Correlation');
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
fig.PaperPosition = [0 0 5 5];
print(fig,[output_folder filename '-traces.pdf'],'-dpdf','-painters')
