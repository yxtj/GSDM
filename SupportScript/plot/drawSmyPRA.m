function drawSmyPRA(fldX, lblX, fldFix, valFix, datas, labels, fnPrefix, locations, numCol, type, fontSize, printSize)

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
    
    if ~exist('locations','var') || length(locations)<3
        for i=1:3
            if isempty(locations{i})
                locations{i}='NorthEast';
            end
        end
    end
    
    if ~exist('numCol','var') || ~isnumeric(numCol)
        numCol=[1;1;1];
    elseif length(numCol)==1
        numCol=[numCol;numCol;numCol];
    elseif length(numCol)==2
        numCol=[numCol(:);1];
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
        
    figure(1)
    keys=drawEBGroupOne(datas,labels,fldX, fldFix, valFix,'precision',1);
    set(gca,'FontSize', fontSize)
    xlabel(lblX); ylabel('precision');
    if length(keys(:))==1
            legend(keys(:));
            legend('Location','NorthWest')
            legend('boxoff')
    else
        columnlegend(numCol(1),keys(:),'Location',locations{1},'boxoff');
    end
    if ~strcmp(OPTION,'none')
        set(gcf,'PaperUnits','inches','PaperPosition',printSize)
        saveas(gcf,[fnPrefix '_prec' EXTENTION],OPTION)
    end

    figure(2)
    drawEBGroupOne(datas,labels,fldX, fldFix, valFix,'recall',1);
    set(gca,'FontSize', fontSize)
    xlabel(lblX); ylabel('recall');
    if length(keys(:))==1
            legend(keys(:));
            legend('Location','NorthEast')
            legend('boxoff')
    else
        columnlegend(numCol(2),keys(:),'Location',locations{2},'boxoff');
    end
    if ~strcmp(OPTION,'none')
        set(gcf,'PaperUnits','inches','PaperPosition',printSize)
        saveas(gcf,[fnPrefix '_reca' EXTENTION],OPTION)
    end
    
    figure(3)
    drawEBGroupOne(datas,labels,fldX, fldFix, valFix,'accuracy',1);
    set(gca,'FontSize', fontSize)
    xlabel(lblX); ylabel('accuracy');
    if length(keys(:))==1
            legend(keys(:));
            legend('Location','NorthWest')
            legend('boxoff')
    else
        columnlegend(numCol(3),keys(:),'Location',locations{3},'boxoff');
    end
    if ~strcmp(OPTION,'none')
        set(gcf,'PaperUnits','inches','PaperPosition',printSize)
        saveas(gcf,[fnPrefix '_accu' EXTENTION],OPTION)
    end
     
end
