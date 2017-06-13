root = 'X:\Sean Warren\Motion correction paper\Simulated Data 4\';

files = dir([root '*.csv']);
files = {files.name};

zoom = 1;
scan_speed = 1000;

meta = regexp(files,'Amplitude=(?<Amplitude>[0-9_]+) Frequency=(?<Frequency>[0-9_]+) Angle=(?<Angle>[0-9_]+)','names','once');

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
angles = unique([data.Angle]);

corr = nan*ones(length(amplitudes),length(frequencies),length(angles));
f70 = nan*ones(length(amplitudes),length(frequencies),length(angles));

for k=1:length(angles)
    for i=1:length(amplitudes)
        for j=1:length(frequencies)

            sel = [data.Frequency] == frequencies(j) ...
                & [data.Amplitude] == amplitudes(i) ...
                & [data.Angle] == angles(k);
            d = data(sel);

            if ~isempty(d)
                corr(i,j,k) = mean(d.correlation);
                f70(i,j,k) = mean(d.correlation > 0.7);
            end
        end
    end
end
corr(1,:,:) = corr(1,1,1);
corr(:,1,:) = corr(1,1,1);
f70(1,:,:) = f70(1,1,1);
f70(:,1,:) = f70(1,1,1);

amplitudes_scaled = amplitudes / 256 * 100; % Convert to percentage of FOV
frequencies_scaled = frequencies * scan_speed / 256; % Convert from relative -> Hz

set(0,'defaultAxesFontSize',8)

for i=1:length(angles)
    subplot(1,length(angles),i)
    imagesc(frequencies_scaled,amplitudes_scaled,corr(:,:,i));
    caxis([0 1])
    set(gca,'TickDir','out','Box','off','YDir','normal')
%    colorbar
end
%{
for i=1:length(angles)
    subplot(2,length(angles),i+length(angles))
    imagesc(frequencies,amplitudes_scaled,f70(:,:,i));
    caxis([0 1])
    colorbar
end
%}

set(gcf,'Units','centimeters','PaperPosition',[0,0,14,4]);
print(gcf,'Correlation-2D.pdf','-dpdf');