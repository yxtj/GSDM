function count=getLengthCount(len)
    count=zeros(0);
    for i=1:length(len);
        x=len(i);
        if x>length(count);
            count(x)=0;
        end;
        count(x)=count(x)+1;
    end
end
