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
    drawEBGroupOne(datas,labels,fldX, lblX, fldFix, valFix,'precision','precision',1);
    set(gca,'FontSize', 14)
    if ~strcmp(OPTION,'none')
        saveas(gcf,[fnPrefix '_' fnSufX '_prec' EXTENTION],OPTION)
    end

    figure(2)
    drawEBGroupOne(datas,labels,fldX, lblX, fldFix, valFix,'recall','recall',1);
    set(gca,'FontSize', 14)
    if ~strcmp(OPTION,'none')
        saveas(gcf,[fnPrefix '_' fnSufX '_reca' EXTENTION],OPTION)
    end
    
    figure(3)
    drawEBGroupOne(datas,labels,fldX, lblX, fldFix, valFix,'accuracy','accuracy',1);
    set(gca,'FontSize', 14)
    if ~strcmp(OPTION,'none')
        saveas(gcf,[fnPrefix '_' fnSufX '_accu' EXTENTION],OPTION)
    end
     
end
