function [res,idx] = filterByLength(data, lmin, lmax)
    if nargin==2
        lmax=inf;
    end
    res=cell(0);
    idx=zeros(0);
    j=1;
    lenD=length(data);
    for i=1:lenD
        l=length(data{i});
        if lmin<=l && l<=lmax
            res{j}=data{i};
            idx(j)=i;
            j=j+1;
        end
    end
    res=res';
    idx=idx';
end
