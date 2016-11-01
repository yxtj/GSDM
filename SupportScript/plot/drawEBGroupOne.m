function drawEBGroupOne(datas, labels, fldX, lblX, fldFix, valFix, fldY, lblY, showLegend)
    % draw error bar for each group
    % datas: struct vector of each graph
    % labels: the legend for the datas
    
    lgnds=cell(numel(labels),1);
    for i=1:numel(labels)
        t=cell2mat(labels(i));
        t=strrep(t,'graph-','');
        t=strrep(t,'p-s','');
        lgnds(i)=mat2cell(t,1,length(t));
    end
    
    if nargin<9
        showLegend=0;
    end

    COLORS=['rgbymck'];
    MARKERS=['o+*.xsd^v><ph'];

    xRange=[1 0];
    clf;
    hold all;
    for id=1:numel(datas)
        data=datas(id);
        FIX=data.(fldFix);
        idx=find(FIX==valFix);
        X=data.(fldX);
        UX=unique(X(idx));
        xRange=[min(xRange(1),UX(1)) max(xRange(2),UX(end))];
        Y=data.(fldY);
        [m,s]=getMeanSigma(X(idx),Y(idx));
        errorbar(UX,m,s,['-' COLORS(id) MARKERS(id)])%,'DisplayName',lgnds{id});
    end
    hold off;
    grid on;
    yRange=ylim;
    yRange=[min(yRange(1),0) max(yRange(2),1)];
    xlim(xRange); ylim(yRange);
    
    set(gca,'FontSize', 20)
    xlabel(lblX); ylabel(lblY);
    
    if showLegend
        %legend('Location','NorthWest');
        %columnlegend(2,lgnds(:),'Location','SouthEast','boxoff');
        columnlegend(2,lgnds(:),'Location','NorthWest','boxoff');
    end

end

