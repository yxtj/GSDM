function drawSmyPRA(fldX, lblX, fldFix, valFix, datas, labels, folder, fnPre, fnSufX, type, fontSize, printSize)

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
    
    if ~exist('fontSize','var') || isempty(fontSize)
        fontSize=16;
    end
    if ~exist('printSize','var') || isempty(printSize)
        printSize=[0 0 6 4.5];
    end
    if length(printSize)==2
        printSize=[0;0;printSize(:)]';
    elseif length(printSize)~=4
        error('Wrong print size.')
    end
    
    if ~exist(folder,'dir')
        mkdir(folder);
    end
    
    fnPrefix=[folder '/' fnPre];
    
    figure(1)
    keys=drawEBGroupOne(datas,labels,fldX, fldFix, valFix,'precision');
    set(gca,'FontSize', fontSize)
    xlabel(lblX); ylabel('precision');
    columnlegend(2,keys(:),'Location','NorthWest','boxoff');
    if ~strcmp(OPTION,'none')
        set(gcf,'PaperUnits','inches','PaperPosition',printSize)
        saveas(gcf,[fnPrefix '_' fnSufX '_prec' EXTENTION],OPTION)
    end

    figure(2)
    drawEBGroupOne(datas,labels,fldX, fldFix, valFix,'recall');
    set(gca,'FontSize', fontSize)
    xlabel(lblX); ylabel('recall');
    columnlegend(2,keys(:),'Location','NorthEast','boxoff');
    if ~strcmp(OPTION,'none')
        set(gcf,'PaperUnits','inches','PaperPosition',printSize)
        saveas(gcf,[fnPrefix '_' fnSufX '_reca' EXTENTION],OPTION)
    end
    
    figure(3)
    drawEBGroupOne(datas,labels,fldX, fldFix, valFix,'accuracy');
    set(gca,'FontSize', fontSize)
    xlabel(lblX); ylabel('accuracy');
    columnlegend(2,keys(:),'Location','NorthWest','boxoff');
    if ~strcmp(OPTION,'none')
        set(gcf,'PaperUnits','inches','PaperPosition',printSize)
        saveas(gcf,[fnPrefix '_' fnSufX '_accu' EXTENTION],OPTION)
    end
     
end
