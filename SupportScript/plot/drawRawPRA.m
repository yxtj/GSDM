function drawRawPRA(lblX, fnSufX, X, lblGP1, GP1, lblGP2, GP2, data, type, folder, fn)
    %draw complete figures for Precision, Recall and Accuracy relavent to X
    %fnSufX: file name suffix for X
    
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
    
    fnPrefix=strrep(strrep(fn,'.tsv',''),'tbl-','');
    fnPrefix=strrep(strrep(fnPrefix, '0.', ''),'/','_');
    
    fnPrefix=[folder '/' fnPrefix];
    
    % ms
    figure(1)
    drawRawGroupOne(X, lblGP1, GP1, lblGP2, GP2, data.precision,0);
    ylim([0 1]);
    xlabel(lblX); ylabel('precision')
    set(gca,'FontSize', 13)
    saveas(gcf,[fnPrefix '_' fnSufX '_prec' EXTENTION],OPTION)

    % th
    figure(2)
    drawRawGroupOne(X, lblGP1, GP1, lblGP2, GP2, data.recall,0);
    ylim([0 1]);
    xlabel(lblX); ylabel('recall')
    set(gca,'FontSize', 13)
    saveas(gcf,[fnPrefix '_' fnSufX '_reca' EXTENTION],OPTION)


    figure(3)
    drawRawGroupOne(X, lblGP1, GP1, lblGP2, GP2, data.accuracy,1);
    ylim([0 1]);
    xlabel(lblX); ylabel('accuracy')
    set(gca,'FontSize', 13)
    saveas(gcf,[fnPrefix '_' fnSufX '_accu' EXTENTION],OPTION)

end
