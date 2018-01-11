
%%
%{
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
%}
%%
%{
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
%}
