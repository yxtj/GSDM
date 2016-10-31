function drawEBGroupOne(datas, fnDatas, fldX, lblX, fldFix, valFix, fldY, lblY, showLegend)
    % draw error bar for each group
    % datas: struct vector of each graph
    % fnDatas: the legend for the datas
    
    lblDatas=cell(numel(fnDatas),1);
    for i=1:numel(fnDatas)
        t=cell2mat(fnDatas(i));
        t=strrep(t,'graph','');
        t=strrep(t,'p-s','');
        lblDatas(i)=t;
    end
    
    if nargin<9
        showLegend=0;
    end

    COLORS=['rgbymck'];
    MARKERS=['o+*.xsd^v><ph'];

    xRange=zeros(1,2);
    hold on;
    for id=1:numel(datas)
        data=datas(id);
        FIX=data.(fldFix);
        idx=find(FIX==valFix);
        X=data.(fldX);
        UX=unique(X(idx));
        xRange=[min(xRange(1),UX(1)) max(xRange(2),UX(end))];
        Y=data.(fldY);
        [m,s]=getMeanSigma(X(idx),Y(idx));
        errorbar(UX,m,s,[COLORS(id) MARKERS(id)],'DisplayName',lblDatas{id});
    end
    hold off;
    yRange=ylim;
    yRange=[min(yRange(1),0) max(yRange(2),1)];
    xlim([min(X),max(X)]); ylim(yRange);
    
    xlabel(lblX); ylabel(lblY);
    
    if showLegend
        legend('Location','NorthEastOutside');
    end

end

