function drawSmyPRA(fldX, lblX, fldFix, valFix, datas, labels, type, folder, fnPre, fnSufX)

    switch (type)
        case {'none' 'disp'}
            EXTENTION='none'; OPTION='none';
        case {'png'}
            EXTENTION='.png'; OPTION='png';
        case {'eps'}
            EXTENTION='.eps'; OPTION='epsc';
        otherwise
            error(['Unsupported type: ' type])
    end
    
    if ~exist(folder,'dir')
        mkdir(folder);
    end
    
    fnPrefix=[folder '/' fnPre];
    
    figure(1)
    keys=drawEBGroupOne(datas,labels,fldX, fldFix, valFix,'precision');
    set(gca,'FontSize', 20)
    xlabel(lblX); ylabel('precision');
    %legend('Location','NorthWest');
    %columnlegend(2,keys(:),'Location','SouthEast','boxoff');
    columnlegend(2,keys(:),'Location','NorthWest','boxoff');
    if ~strcmp(OPTION,'none')
        %set(gcf,'PaperUnits','inches','PaperPosition',[0 0 6 4.5])
        saveas(gcf,[fnPrefix '_' fnSufX '_prec' EXTENTION],OPTION)
    end

    figure(2)
    drawEBGroupOne(datas,labels,fldX, fldFix, valFix,'recall');
    set(gca,'FontSize', 20)
    xlabel(lblX); ylabel('recall');
    %legend('Location','NorthWest');
    %columnlegend(2,keys(:),'Location','SouthEast','boxoff');
    columnlegend(2,keys(:),'Location','NorthEast','boxoff');
    if ~strcmp(OPTION,'none')
        %set(gcf,'PaperUnits','inches','PaperPosition',[0 0 6 4.5])
        saveas(gcf,[fnPrefix '_' fnSufX '_reca' EXTENTION],OPTION)
    end
    
    figure(3)
    drawEBGroupOne(datas,labels,fldX, fldFix, valFix,'accuracy');
    set(gca,'FontSize', 20)
    xlabel(lblX); ylabel('accuracy');
    %legend('Location','NorthWest');
    %columnlegend(2,keys(:),'Location','SouthEast','boxoff');
    columnlegend(2,keys(:),'Location','NorthWest','boxoff');
    if ~strcmp(OPTION,'none')
        %set(gcf,'PaperUnits','inches','PaperPosition',[0 0 6 4.5])
        saveas(gcf,[fnPrefix '_' fnSufX '_accu' EXTENTION],OPTION)
    end
     
end
