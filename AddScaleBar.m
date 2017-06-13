function im =  AddScaleBar(im, microns_per_pixel, bar_width_microns, bar_height_pixels)
% AddScaleBar Add a small while scale bar to the bottom right of an image
%    im : Image to add scale bar
%    microns_per_pixels: Number of microns per pixel
%    bar_width_microns: How wide the bar should be in microns
%    bar_height_pixels: How tall the bar should be in pixels 

    imw = size(im,2);
    imh = size(im,1);

    w = bar_width_microns / microns_per_pixel;
    h = bar_height_pixels;

    pos = [imw - w - h, imh - 2*h, w, h];
    im = insertShape(im, 'FilledRectangle', pos, 'Color', 'white', 'Opacity', 1);

%   To add a text label use something like...
%   pos_text = [imw - 0.5*w - h, imh - 2*h - 5];
%   im = insertText(im, pos_text, '50um','AnchorPoint','CenterBottom','BoxOpacity',0,'FontSize',int32(w*0.2));