function data = ReadPointFile(points_file,n_px,zoom,scan_speed)

    filedata = fileread(points_file);
    filedata = strrep(filedata,'-nan(ind)','0');
 
    f = fopen(points_file,'w');
    fprintf(f,filedata);
    fclose(f);

    if isfinite(zoom)
        um_per_px = 1.2442e+03 / (n_px * zoom);
    else
        um_per_px = 1;
    end
    
    d = csvread(points_file,0,1,[0 1 4 1]);

    data.line_duration = d(1);
    data.interline_duration = d(2);
    data.frame_duration = d(3);
    data.interframe_duration = d(4); 
    data.lines = d(5);
    
    points = csvread(points_file,6,0);
    data.unaligned_correlation = points(:,2);
    data.correlation = points(:,3);
    data.coverage = points(:,4);
    points = points(:,5:end);
    
    data.points_unscaled = points';
    data.points = points' * um_per_px;

    tc = (0:size(data.points,2)-1) * data.interframe_duration;
    tr = (0:size(data.points,1)-1)' * data.frame_duration / (size(data.points,1)-1);
    data.t_unscaled = (tc + tr) / data.frame_duration;

    
    frame_time = data.lines / scan_speed;
    
    data.t = data.t_unscaled * frame_time;
    data.t_frame = data.t(1,:);

