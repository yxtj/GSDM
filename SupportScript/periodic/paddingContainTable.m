function mat=paddingContainTable(data, lmax)
    l=length(data);
    if nargin<2
        lmax=0;
        for i=1:l;
            lmax=max(lmax,length(data{i}));
        end
    end
    
    mat=zeros(length(data), lmax)-1;
    for i=1:l;
        t=data{i};
        mat(i,1:length(t))=t;
    end
end
