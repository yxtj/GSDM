function data=smyLoader(fn)
    % load the summary file and remove the entry with 0 number of motifs
    
    data=tdfread(fn);
    
    idx=find(data.num==0);
    if isempty(idx)
        return
    end
    
    fields=fieldnames(data);
    for i=1:length(fields)
        data.(fields{i})(idx)=[];
    end
    
end
