function [recovery] = ExtractRecovery(images, roi, p)

    if nargin < 3
        p = [];
    end
    
    assert(length(roi) > 2)

    roi_x = real(roi);
    roi_y = imag(roi);
    
    px = real(p);
    py = imag(p);
    
    sz = size(images{1});
    [X,Y] = meshgrid(1:sz(2),1:sz(1));
    mask1 = inpoly([X(:),Y(:)],[roi_x,roi_y]);
        
    recovery = zeros(size(images));
    for j=1:length(images)
        
        if ~isempty(p)
            rx = roi_x+px(j);
            ry = roi_y+py(j);

            sel = (X >= min(floor(rx))) & ...
                  (X <= max(ceil(rx)))  & ...
                  (Y >= min(floor(ry))) & ...
                  (Y <= max(ceil(ry)));

            if sum(sel(:)) > 0 % check if ROI contains any valid pixels
                mask = inpoly([X(sel),Y(sel)],[rx,ry]);
                im = images{j}(sel);
                v = sum(im(mask));
            else
                v = 0;
            end
        else
            v = sum(images{j}(mask1));
        end
        
        recovery(j) = v;
    end
        
    %initial = recovery(1:n_initial);
    %recovery = recovery / mean(initial);
    
end