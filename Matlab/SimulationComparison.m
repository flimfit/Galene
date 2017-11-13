
program = 'Unaligned';
suffix = 'stack';

root = ['C:\Users\CIMLab\Documents\User Data\Sean\Motion Correction\Simulated Data Comparison\' program '\']

files = dir([root '*' suffix '.tif']);
files = {files.name};

zoom = 1;
scan_speed = 1000;

meta = regexp(files,'Amplitude=(?<Amplitude>[0-9_]+) Frequency=(?<Frequency>[0-9_]+) Angle=(?<Angle>[0-9_]+)','names','once');

clear data d;
for i=1:length(files)
    fields = fieldnames(meta{i});
    for j=1:length(fields)
        d.(fields{j}) = str2double(strrep(meta{i}.(fields{j}),'_','.'));
    end
    data(i) = d;
end

%%
im = ReadTifStack([root files{1}]);
im = single(im);
ref = mean(im,3);                
                
figure(2)
imagesc(mean(im,3));


%%
figure(1)



amplitudes = unique([data.Amplitude]);
frequencies = unique([data.Frequency]);
angles = unique([data.Angle]);

corr = nan*ones(length(amplitudes),length(frequencies),length(angles));

for k=1:length(angles)
    for i=1:length(amplitudes)
        disp([num2str(i) '/' num2str(length(amplitudes))])
        for j=1:length(frequencies)

            sel = [data.Frequency] == frequencies(j) ...
                & [data.Amplitude] == amplitudes(i) ...
                & [data.Angle] == angles(k);
            
            if sum(sel) == 1
                im = ReadTifStack([root files{sel}]);
                im = single(im);
                
                %c = zeros(1,size(im,3));
                %for t=1:size(im,3)
                %   c(t) = corr2(ref,im(:,:,t));  
                %end

                % SIMA gives cropped images
                sz = size(im);
                if any(sz(1:2)~=size(ref))
                    [optimizer, metric] = imregconfig('monomodal');
                    im = imregister(im,ref, 'translation', optimizer, metric);
                end
                
                im_mask = sum(im > 0, 3);
                im_mean = mean(im,3) ./ im_mask;
                im_mean(~isfinite(im_mean)) = nan;                
                c = nancorr2(ref, im_mean); 
                
                im8 = uint8(im_mean * 1000);
                out_file = strrep([root files{sel}],'.tif','-sum.tif');
                imwrite(im8, out_file);
                
                corr(i,j,k) = mean(c);
            end
        end
    end
end
corr(1,:,:) = corr(1,1,1);
corr(:,1,:) = corr(1,1,1);
%%

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
print(gcf,['Correlation-2D-' program '.pdf'],'-dpdf');