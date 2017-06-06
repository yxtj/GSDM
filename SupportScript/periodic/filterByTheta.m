function [res,idx] = filterByTheta(data, theta)
    res=cell(0);
    idx=zeros(0);
    j=1;
    for i=1:length(data)
        t=ceil(length(data{i})*theta);
        s=sum(data{i});
        if s>=t
            res{j}=data{i};
            idx(j)=i;
            j=j+1;
        end
    end
    res=res';
    idx=idx';
end
