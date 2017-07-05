SetupDatasets();
data = dataset(1);


points_file = [data.file '_realignment.csv'];
aligned_name = [data.file '-aligned-stack.tif'];
unaligned_name = [data.file '-unaligned-stack.tif'];
aligned_presv_name = [data.file '-aligned-int-presv-stack.tif'];


aligned = ReadTifStack(aligned_name);
unaligned = ReadTifStack(unaligned_name);
aligned_presv = ReadTifStack(aligned_presv_name);
points = ReadPointFile(points_file,data.n_px,data.zoom,data.scan_rate);

%%

figure(10);
clf(10)
frame = 283;

d = 256;
padd = 10;

imr = unaligned(:,:,1);
imu = unaligned(:,:,frame);
ima = aligned(:,:,frame);
p = points.points(:,frame);
c = points.correlation(frame);

%imr = RebinStack(imr,2);
%imu = RebinStack(imu,2);
%ima = RebinStack(ima,2);

colormap('gray')
subplot(1,4,1)
imagesc(imr);
daspect([1 1 1])
xlim([-padd d+padd]);
ylim([-padd d+padd]);

hold on;
for i=linspace(1,256,5)
    plot([1 d],[i i],'r-');
    plot([i i],[1 d],'r-');    
end
    

subplot(1,4,2)
imagesc(imu);
daspect([1 1 1])
xlim([-padd d+padd]);
ylim([-padd d+padd]);

hold on;
for i=linspace(1,256,5)
    plot([1 d],[i i],'r-');
    plot([i i],[1 d],'r-');    
end


subplot(1,4,3)
hold on;
tf = linspace(1,points.lines*points.lines,length(p))'-1;
x = mod(tf,points.lines)+1;
y = floor(tf / points.lines)+1;
x = x;
y = y;

pi = interp1(1:length(p),p,linspace(1,length(p),40)');
tfi = linspace(1,points.lines*points.lines,length(pi))'-1;
xi = mod(tfi,points.lines)+1;
yi = floor(tfi / points.lines)+1;
xi = xi;
yi = yi;


col = lines(5);
col = col(2,:);

%plot(x,y,'x');
%plot(x-real(p),y+imag(p),'o')
quiver(xi-real(pi),yi+imag(pi),real(pi),-imag(pi),0,'LineWidth',1.5,'MaxHeadSize',4,'Color',[0.8 0.8 0.8]);
quiver(x-real(p),y+imag(p),real(p),-imag(p),0,'LineWidth',1.5,'MaxHeadSize',4,'Color',col);
daspect([1 1 1])
xlim([-padd d+padd]);
ylim([-padd d+padd]);
set(gca,'YDir','reverse')


subplot(1,4,4)
imagesc(imfuse(ima,imr));

for i=1:4
    subplot(1,4,i)
    daspect([1 1 1])
    xlim([-padd d+padd]);
    ylim([-padd d+padd]);
    set(gca,'Box','off','TickDir','out','XTick',[],'YTick',[]);
end

%%

figure(5)
clf;
hold on;
padd = 100
plot([1 1 256 256 1],[1 256 256 1 1],'k');
%plot(x,y,'x');
%plot(x-real(p),y+imag(p),'o')
quiver(xi-real(pi),yi+imag(pi),real(pi),-imag(pi),0,'LineWidth',1.5,'MaxHeadSize',4,'Color',[0.8 0.8 0.8]);
quiver(x-real(p),y+imag(p),real(p),-imag(p),0,'LineWidth',1.5,'MaxHeadSize',4,'Color',col);
daspect([1 1 1]);
xlim([-padd d+padd]);
ylim([-padd d+padd]);
set(gca,'YDir','reverse')



%%

unaligned_b = RebinStack(unaligned,2);

figure(11);
colormap('gray')
subplot(1,20,1);
imagesc(sum(unaligned_b,3))
set(gca,'Box','off','TickDir','out','XTick',[],'YTick',[]);
daspect([1 1 1]);
for i=2:20
    subplot(1,20,i);
    imagesc(unaligned_b(:,:,i*2));
    set(gca,'Box','off','TickDir','out','XTick',[],'YTick',[]);
    daspect([1 1 1]);
end


%%

clf
aligned_b = RebinStack(aligned,2);
imagesc(sum(aligned_b,3))
set(gca,'Box','off','TickDir','out','XTick',[],'YTick',[]);
daspect([1 1 1]);

%%

clf

b = sum(aligned_b,3);
kern = ones(3,3);
%b = conv2(b,kern,'same');
[gx,gy] = gradient(b);

imagesc(gx)
colormap('jet')
gradient