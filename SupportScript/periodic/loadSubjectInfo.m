function [id,type,len]=loadSubjectInfo(fn)
    fid=fopen(fn,'r');
    id=cell(0);
    type=zeros(0);
    len=zeros(0);
    line = fgetl(fid);
    i=1;
    while ~feof(fid) && ischar(line)
        %disp(line)
        C=textscan(line,'%s %d %d');
        id{i}=C{1};
        type(i)=C{2};
        len(i)=C{3};
        i=i+1;
        line = fgetl(fid);
    end
    id=id';
    type=type';
    len=len';
end
