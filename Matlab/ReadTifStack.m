function FinalImage = ReadTifStack(FileTif)

    InfoImage=imfinfo(FileTif);
    mImage=InfoImage(1).Width;
    nImage=InfoImage(1).Height;
    NumberImages=length(InfoImage);

    if isfield(InfoImage(1),'SampleFormat')
        switch InfoImage(1).SampleFormat
            case 'Unsigned integer'
                format = 'uint16';
            case 'IEEE floating point'
                format = 'single';
        end
    elseif isfield(InfoImage(1),'BitDepth')
        switch InfoImage(1).BitDepth
            case 8
                format = 'uint8';
            case 16
                format = 'uint16';
            case 32
                format = 'uint32';
        end 
    end
        
    FinalImage=zeros(nImage,mImage,NumberImages,format);
    for i=1:NumberImages
       FinalImage(:,:,i)=imread(FileTif,'Index',i);
    end