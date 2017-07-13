function FinalImage = ReadTifStack(FileTif)

    InfoImage=imfinfo(FileTif);
    mImage=InfoImage(1).Width;
    nImage=InfoImage(1).Height;
    NumberImages=length(InfoImage);

    switch InfoImage(1).SampleFormat
        case 'Unsigned integer'
            format = 'uint16';
        case 'IEEE floating point'
            format = 'single';
    end
    
    FinalImage=zeros(nImage,mImage,NumberImages,format);
    for i=1:NumberImages
       FinalImage(:,:,i)=imread(FileTif,'Index',i);
    end