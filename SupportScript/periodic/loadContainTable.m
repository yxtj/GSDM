function data = loadContainTable(fn)
    fid=fopen(fn,'r');
    data=cell(0,1);
    line = fgetl(fid);
    i=1;
    while ~feof(fid) && ischar(line)
        %disp(line)
        data{i}=str2num(line);
        i=i+1;
        line = fgetl(fid);
    end
    fclose(fid);
    data=data';
end
