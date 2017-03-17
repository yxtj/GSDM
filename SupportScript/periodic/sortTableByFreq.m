function [res,idx]=sortTableByFreq(mat)
    l=size(mat,1);
    value=zeros(l,1);
    for i=1:l
        d=mat(i,:);
        value(i)=sum(d==1)/sum(d>=0);
    end
    [~,idx]=sort(value);
    res=mat(idx,:);
end
