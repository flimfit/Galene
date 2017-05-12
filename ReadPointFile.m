function data = ReadPointFile(points_file,zoom,scan_speed)

    filedata = fileread(points_file);
    filedata = strrep(filedata,'-nan(ind)','0');
 
    f = fopen(points_file,'w');
    fprintf(f,filedata);
    fclose(f);

    um_per_px = 2.29 * 0.397 / zoom;

    d = csvread(points_file,0,1,[0 1 4 1]);

    data.line_duration = d(1);
    data.interline_duration = d(2);
    data.frame_duration = d(3);
    data.interframe_duration = d(4); 
    data.lines = d(5);
    
    points = csvread(points_file,6,0);
    data.correlation = points(:,2);
    data.coverage = points(:,3);
    points = points(:,4:end);
    data.points = points' * um_per_px;

    tc = (0:size(data.points,2)-1) * data.interframe_duration;
    tr = (0:size(data.points,1)-1)' * data.frame_duration / (size(data.points,1)-1);
    data.t = tc + tr;

    
    frame_time = data.lines / scan_speed;
    
    t = data.t / data.frame_duration * frame_time;
    data.t_frame = t(1:size(data.points,1):end);

