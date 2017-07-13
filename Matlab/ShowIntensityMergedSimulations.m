  
sim_ffd = [file '.ffd'];
aligned_ffh = [file '-aligned.ffh'];

r = FlimReaderMex(sim_ffd);
flim = FlimReaderMex(r,'GetData',0);
t = FlimReaderMex(r,'GetTimePoints');
FlimReaderMex(r,'Delete');
clear FlimReaderMex

r = FlimReaderMex(aligned_ffh);
intensity_normalisation = FlimReaderMex(r,'GetIntensityNormalisation');
aligned_flim = FlimReaderMex(r,'GetData',0);
aligned_t = FlimReaderMex(r,'GetTimePoints');
FlimReaderMex(r,'Delete');
clear FlimReaderMex

%%

irf_ma = 1000;

ma = squeeze(mean(double(flim) .* t',1)./mean(double(flim),1)) - irf_ma;
aligned_ma = squeeze(mean(double(aligned_flim) .* aligned_t',1)./mean(double(aligned_flim),1)) - irf_ma;

intensity = squeeze(sum(flim,1));
aligned_intensity = squeeze(sum(aligned_flim,1));
aligned_intensity = aligned_intensity ./ double(intensity_normalisation) * 50;


figure(2)
subplot(1,2,1)
tau = MeanArrivalToLifetime(ma,12.5e3);
im = ShowFLIM(tau,intensity,[2000 4000],[0 700],false);
im = permute(im,[2 1 3]);
imagesc(im)
daspect([1 1 1])
imwrite(im,[file '-merged.tif']);

subplot(1,2,2)
aligned_tau = MeanArrivalToLifetime(aligned_ma,12.5e3);
im = ShowFLIM(aligned_tau,aligned_intensity,[2000 4000],[0 700],false);
im = permute(im,[2 1 3]);
imagesc(im);
imwrite(im,[file '-aligned_merged.tif']);
daspect([1 1 1])

