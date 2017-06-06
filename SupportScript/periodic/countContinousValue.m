function freq=countContinousValue(data, valid)
    freq=zeros(1);
    for i=1:length(data);
        d=data{i};
        state=0;
        for j=1:length(d);
            if d(j)==valid;
                if state~=0;
                    if state>length(freq); freq(state)=0; end
                    freq(state)=freq(state)+1;
                    state=0;
                end
            else
                state=state+1;
            end
            j=j+1;
        end %for j
        if state~=0;
            if state>length(freq); freq(state)=0; end
            freq(state)=freq(state)+1;
        end
    end % for i
end
