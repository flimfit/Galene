function tc = ShowFLIM(tau, I, lim, Ilim, show_bar)

    if nargin < 4
        show_bar = true;
    end

    cmap = jet(256);
    
    tc = tau;
    tc = (tc - lim(1)) / (lim(2)-lim(1));
    tc(tc<0) = 0;
    tc(tc>1) = 1;
    tc = round(tc * 255) + 1;
    tc(isnan(tc)) = 0;
    tc = ind2rgb(tc,cmap);
    
    Is = double(I);
    Is = (Is - Ilim(1)) / (Ilim(2)-Ilim(1));
    Is(Is>1) = 1;
    tc = tc .* repmat(Is,[1 1 3]);
  
    if (show_bar)
        tc(:,1:5,1) = 1;
        tc(:,1:5,2:3) = 0;
        tc(:,251:256,1) = 1;
        tc(:,251:256,2:3) = 0;
    end