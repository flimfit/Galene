lines = 20;

pixels = lines * lines;

p = 0:(pixels-1);

x = mod(p,lines) + 0.5;
y = floor(p / lines) + 0.5;
x = x / lines;
y = y / lines;

im = imread('simulated_intensity.png');
n = size(im,1);

im = repmat(double(im),[1 1 3]) / 256;
imh = imagesc(1:256,1:256,zeros(256,256));
im(im > 1) = 1;
hold on;


x = (x + 0.05) * 256 * 0.9;
y = (y + 0.05) * 256 * 0.9;

t = linspace(0,3,pixels*3);
freq = 4;
offset_x = round(15 * sin(freq * t * 2*pi));
offset_y = round(15 * cos(freq / 5 * t * 2*pi));


c = [0.7 0 0];
h = plot(0,0,'LineWidth',2,'Color',c); hold on;
h1 = plot(0,0,'o','Color',[0.7 0 0],'MarkerSize',5,'MarkerFaceColor',c,'MarkerEdgeColor',c);
set(gca,'Box','off','XTick',[],'YTick',[],'YDir','reverse');

pad = lines / 10;
xlim([1,256]);
ylim([1,256]);
hold off;

v = VideoWriter('scan-animation-blank.avi');
open(v)

for i=1:(2*pixels)
    
    imx = (1:256)+128+offset_x(i);
    imy = (1:256)+128+offset_y(i);
 
    imi = im(imy,imx,:);
    
    imi = ones(256,256);
    
    imi = imresize(imi,2);
    
    idx = mod(i-1,pixels) + 1;
    
    if idx == 1
        idx = 2; % for insertShape
    end
    
    sel = 1:idx;
    poly = [x(sel);y(sel)];
    poly = poly(:)'*2;
    
    
    imi=insertShape(imi,'Line',poly,'Color','r','LineWidth',1);
    
%    set(h,'XData',x(sel),'YData',y(sel))
%    set(h1,'XData',x(idx),'YData',y(idx))
 
    set(imh,'CData',imi);

    imi(imi>1) = 1;
    writeVideo(v,imi);
    
    drawnow;
end

close(v);
