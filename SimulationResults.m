root = 'X:\Sean Warren\Motion correction paper\Simulated Data\45deg\';

files = dir([root '*.csv']);
files = {files.name};

zoom = 1;
scan_speed = 1000;

meta = regexp(files,'Amplitude=(?<Amplitude>[0-9_]+) Frequency=(?<Frequency>[0-9_]+)','names','once');

%%
clear data;
for i=1:length(files)
    d = ReadPointFile([root files{i}],zoom,scan_speed);
    
    fields = fieldnames(meta{i});
    for j=1:length(fields)
        d.(fields{j}) = str2double(strrep(meta{i}.(fields{j}),'_','.'));
    end
    data(i) = d;
end

%%
amplitudes = unique([data.Amplitude]);
frequencies = unique([data.Frequency]);

corr = nan(length(amplitudes),length(frequencies));

for i=1:length(amplitudes)
    for j=1:length(frequencies)

        sel = [data.Frequency] == frequencies(j) & [data.Amplitude] == amplitudes(i);
        d = data(sel);
        
        if ~isempty(d)
            corr(i,j) = mean(d.correlation);
            f70(i,j) = mean(d.correlation > 0.7);
        end
    end
end

%corr(amplitudes==0,:) = corr(1,1);
subplot(1,2,1)
imagesc(frequencies,amplitudes,corr);
caxis([0.5 1])
colorbar

subplot(1,2,2)
imagesc(frequencies,amplitudes,f70);
caxis([0.5 1])
colorbar
