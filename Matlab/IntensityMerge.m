SetupDatasets();
data = dataset(7);

aligned_ffh = [data.file '-aligned.ffh'];
intensity_file = [data.file '-aligned I raw.tif'];
tau_file = [data.file '-aligned tau_1 raw.tif'];
merged_file = [data.file '-aligned-intensity-normalised-merged.tif'];


r = FlimReaderMex(aligned_ffh);
intensity_normalisation = FlimReaderMex(r,'GetIntensityNormalisation');
FlimReaderMex(r,'Delete');
clear FlimReaderMex

intensity_normalisation = double(intensity_normalisation);
%intensity_normalisation(end-1:end,:) = 50;
intensity_normalisation = reshape(intensity_normalisation,[255,256]);
imagesc(intensity_normalisation)

intensity_normalisation(intensity_normalisation > 1e3) = 0;
intensity_normalisation = intensity_normalisation / max(intensity_normalisation(:));

intensity = imread(intensity_file);
tau = imread(tau_file);

intensity = intensity ./ intensity_normalisation;
imagesc(intensity);

merged = ShowFLIM(tau',intensity',data.tau_lim,data.I_lim,false);

figure(3)
imagesc(merged)
daspect([1 1 1])

imwrite(merged,merged_file)