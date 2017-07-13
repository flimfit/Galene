SetupDatasets()
data = dataset(1);

folder = 'X:\Sean Warren\Motion correction paper\Paper\Fitting Animation\';
dbuffer = [folder 'dbuffer.csv'];


im = ReadTifStack([folder 'processbuffer.tif']);
ref = imread([folder 'reference.tif']);
ref = ref(:,:,1);
points = ReadDbufferFile(dbuffer,data.n_px,data.zoom,data.scan_rate);

%%

%v = VideoWriter('fitting.avi');
%open(v);

for i=size(im,3)
    
    imi = zeros(256,256,3);
    imi(:,:,2) = im(:,:,i);
    imi(:,:,1) = ref;
    imi(:,:,3) = ref;
    
    imi = imi / 10;
    imi(imi>1) = 1;
    %imi = (im(:,:,i) - ref).^2;
    
    
    imagesc(imi);
    %caxis([0 10])
    drawnow;
    
%    writeVideo(v,imi);
    imwrite(imi,['fitting ' num2str(i) '.png'])

end
  
%%
subplot(1,2,1)
diff = (double(im(:,:,3)) - double(ref));
imagesc(diff.^2)
caxis([0 20])

subplot(1,2,2)
diff = (double(im(:,:,end)) - double(ref));
imagesc(diff.^2)
caxis([0 20])

%close(v)

%%

imi = zeros(256,256,3);
imi(:,:,2) = im(:,:,1);    
imi = imi / 10;
imi(imi>1) = 1;
imwrite(imi,'f0.png');
image(imi)

%%
imi = zeros(256,256,3);
imi(:,:,1) = ref;
imi(:,:,3) = ref;   
imi = imi / 10;
imi(imi>1) = 1;
imwrite(imi,'reference.png');



%%
points = ReadDbufferFile(dbuffer,data.n_px,data.zoom,data.scan_rate);

tf = linspace(1,points.lines*points.lines,length(p))'-1;
x = mod(tf,points.lines)+1;
y = floor(tf / points.lines)+1;


%v = VideoWriter('movement.avi');
%open(v);

padd = 10;
d = 256;
for fr=size(points.points,2)

    p = points.points(:,fr);
    pi = interp1(1:length(p),p,linspace(1,length(p),40)');
    tfi = linspace(1,points.lines*points.lines,length(pi))'-1;
    xi = mod(tfi,points.lines)+1;
    yi = floor(tfi / points.lines)+1;

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


    figure(5)
    clf;
    hold on;
    padd = 50;
    plot([1 1 256 256 1],[1 256 256 1 1],'k');
    %plot(x,y,'x');
    %plot(x-real(p),y+imag(p),'o')
    quiver(xi-real(pi),yi+imag(pi),real(pi),-imag(pi),0,'LineWidth',1.5,'MaxHeadSize',4,'Color',[0.8 0.8 0.8]);
    quiver(x-real(p),y+imag(p),real(p),-imag(p),0,'LineWidth',1.5,'MaxHeadSize',4,'Color',col);
    daspect([1 1 1]);
    xlim([-padd d+padd]);
    ylim([-padd d+padd]);
    set(gca,'YDir','reverse')
        
    F = getframe;
    
   % writeVideo(v,F);

    drawnow
end

%close(v)
