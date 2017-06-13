function a1 = RebinStack(a,factor)

    a1 = double(a);
    sz = size(a1);
    
    if length(sz) < 3
        sz(3) = 1;
    end

    a1 = reshape(a1,[factor,sz(1)/factor,sz(2:3)]);
    a1 = sum(a1,1);
    a1 = permute(a1,[1 3 2 4]);
    a1 = reshape(a1,[2 sz(2)/factor sz(1)/factor sz(3)]);
    a1 = sum(a1,1);
    a1 = permute(a1,[3 2 4 1]);