function [x,y] = GetPointsFromLeicaRoiFile(filename, im_width_px, im_width_um)
% Load ROI from a Leica '.roi' file
% Tested with files saved from Leica SP8 FRAP module 
% Somewhat reverse-engineered - use with care! 

    dim = im_width_px;
    imsize = im_width_um * 1e-6;

    a = parseXML(filename);
    roi =  a.Children(2).Children(6).Children(2).Children(2).Children(2);
    points = roi.Children(2).Children;
    points = points(2:(end-1));

    transform = roi.Children(4);
    translation = transform.Children(4).Attributes;

    y0 = str2double(translation(2).Value);
    x0 = str2double(translation(1).Value);

    x = [];
    y = [];

    for i=1:2:length(points)
       x(end+1) = str2double(points(i).Attributes(1).Value);
       y(end+1) = str2double(points(i).Attributes(2).Value);
    end

    x = x - x0;
    y = y - y0;

    x = - x / imsize * dim + dim/2;
    y = - y / imsize * dim + dim/2;

end