function tau = MeanArrivalToLifetime(ma,T)

    gamma0 = ma / T;
    gamma = ma / T;

    for i=1:4
        f = 1./(exp(1./gamma)-1);
        gamma = gamma - (gamma0-gamma + f) ./ (exp(1./gamma)./(gamma.^2).*f.^2-1); 
    end
    
    tau = gamma * T;