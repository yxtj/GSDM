function drawRawPRA(lblX, fnSufX, X, lblGP1, GP1, lblGP2, GP2, data, folder, fn, type, fontSize, printSize)
    %draw complete figures for Precision, Recall and Accuracy relavent to X
    %fnSufX: file name suffix for X
    
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
        fontSize=14;
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
    
    fnPrefix=strrep(strrep(fn,'.tsv',''),'tbl-','');
    fnPrefix=strrep(strrep(fnPrefix, '0.', ''),'/','_');
    
    fnPrefix=[folder '/' fnPrefix];
    
    % precision
    figure(1)
    keys=drawRawGroupOne(X, lblGP1, GP1, lblGP2, GP2, data.precision,1);
    ylim([0 1]);
    xlabel(lblX); ylabel('precision')
    set(gca,'FontSize', fontSize)  
    %legend('Location','NorthEastOutside');
    %columnlegend(size(keys,2),keys(:),'Location','SouthEast');
    if ~strcmp(OPTION,'none')
        set(gcf,'PaperUnits','inches','PaperPosition',printSize)
        saveas(gcf,[fnPrefix '_' fnSufX '_prec' EXTENTION],OPTION)
    end

    % recall
    figure(2)
    keys=drawRawGroupOne(X, lblGP1, GP1, lblGP2, GP2, data.recall,1);
    ylim([0 1]);
    xlabel(lblX); ylabel('recall')
    set(gca,'FontSize', fontSize)
    %legend('Location','NorthEastOutside');
    %columnlegend(size(keys,2),keys,'Location','SouthEast');
    if ~strcmp(OPTION,'none')
        set(gcf,'PaperUnits','inches','PaperPosition',printSize)
        saveas(gcf,[fnPrefix '_' fnSufX '_reca' EXTENTION],OPTION)
    end

    % accuracy
    figure(3)
    keys=drawRawGroupOne(X, lblGP1, GP1, lblGP2, GP2, data.accuracy,1);
    ylim([0 1]);
    xlabel(lblX); ylabel('accuracy')
    set(gca,'FontSize', fontSize)
    addLegendOutside(keys,'NorthEastOutside');
    %[h,~,~,~]=columnlegend(size(keys,2),keys,'Location','SouthEast');
    if ~strcmp(OPTION,'none')
        set(gcf,'PaperUnits','inches','PaperPosition',printSize)
        saveas(gcf,[fnPrefix '_' fnSufX '_accu' EXTENTION],OPTION)
    end

end
