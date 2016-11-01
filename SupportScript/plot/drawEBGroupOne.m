function keys=drawEBGroupOne(datas, labels, fldX, fldFix, valFix, fldY)
    % draw error bar for each group
    % datas: struct array of each graph
    % labels: the legend for the datas
    
    keys=cell(numel(labels),1);
    for i=1:numel(labels)
        t=cell2mat(labels(i));
        t=strrep(t,'graph-','');
        t=strrep(t,'p-s','');
        keys(i)=mat2cell(t,1,length(t));
    end

    COLORS=['rgbymck'];
    MARKERS=['o+*.xsd^v><ph'];

    xRange=[1 0];
    clf;
    hold all;
    for id=1:numel(datas)
        data=datas(id);
        X=data.(fldX);
        if ischar(valFix) && strcmp(valFix,'all')
            idx=1:length(X);
        else
            FIX=data.(fldFix);
            idx=find(FIX==valFix);
        end
        UX=unique(X(idx));
        xRange=[min(xRange(1),UX(1)) max(xRange(2),UX(end))];
        Y=data.(fldY);
        [m,s]=getMeanSigma(X(idx),Y(idx));
        errorbar(UX,m,s,['-' COLORS(id) MARKERS(id)])%,'DisplayName',keys{id});
    end
    hold off;
    grid on;
    yRange=ylim;
    yRange=[min(yRange(1),0) max(yRange(2),1)];
    xlim(xRange); ylim(yRange);
 
end

