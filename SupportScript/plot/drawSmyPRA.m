function drawSmyPRA(fldX, lblX, fldFix, valFix, datas, fnds, type, folder, fnPre)

    if strcmp(type,'png')
        EXTENTION='.png'; OPTION='png';
    elseif strcmp(type,'eps')
        EXTENTION='.eps'; OPTION='epsc';
    else
        error(['Unsupported type: ' type])
        return;
    end
    
    if ~exist(folder,'dir')
        mkdir(folder);
    end
    
    fnPrefix=[folder '/' fnPre];
    
    figure(1)
    drawEBGroupOne(datas,fnds,fldX, lblX, fldFix, valFix,'precision','precision',0);
    set(gca,'FontSize', 13)
    saveas(gcf,[fnPrefix '_' fnSufX '_prec' EXTENTION],OPTION)

    figure(2)
    drawEBGroupOne(datas,fnds,fldX, lblX, fldFix, valFix,'recall','recall',0);
    set(gca,'FontSize', 13)
    saveas(gcf,[fnPrefix '_' fnSufX '_reca' EXTENTION],OPTION)
    
    figure(3)
    drawEBGroupOne(datas,fnds,fldX, lblX, fldFix, valFix,'accuracy','accuracy',1);
    set(gca,'FontSize', 13)
    saveas(gcf,[fnPrefix '_' fnSufX '_accu' EXTENTION],OPTION)
    
    
end
