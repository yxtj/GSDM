function [m,s]=getMeanSigma(x,y)
    ux=unique(x);
    m=zeros(size(ux));
    s=zeros(size(ux));
    for i=1:length(ux)
        t=y(x==ux(i));
        m(i)=mean(t);
        s(i)=std(t);
    end
end
