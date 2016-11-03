WINDOWS={'p-s20-10';'p-s30-12';'p-s30-20'};
GRAPHS={'graph-0.5';'graph-0.8'};
FILENAMES={'tbl-func.tsv'};
%FILENAMES={'tbl-func.tsv';'tbl-func-tr.tsv'};

dataPrefix='../../data_adhd/summary/';
figPrefix='../../data_adhd/figure/';

% draw all lines of a single graph configuration
type='eps';
%type='png';
%type='none';
fontSize=18;
printSize=[0 0 4 3]*1.5;
close all
for iw=1:numel(WINDOWS); for ig=1:numel(GRAPHS); for ifn=1:numel(FILENAMES);
	fn=[WINDOWS{iw} '/' GRAPHS{ig} '/' FILENAMES{ifn}]
	data=smyLoader([dataPrefix fn]);
	drawRawPRA('minSup','ms',data.minsup,'T',data.theta,'A',data.alpha,data,figPrefix,fn,type,fontSize,printSize)
    close all;
    drawRawPRA('theta','th',data.theta,'M',data.minsup,'A',data.alpha,data,figPrefix,fn,type,fontSize,printSize)
	close all;
end;end;end;

for iw=1:numel(WINDOWS); for ig=1:numel(GRAPHS); for ifn=1:numel(FILENAMES);
    fn=[WINDOWS{iw} '/' GRAPHS{ig} '/' FILENAMES{ifn}]
	data=smyLoader([dataPrefix fn]);
    keys=drawRawGroupOne(data.theta,'M',data.minsup,'A',data.alpha,data.recall);
    xlim([min(data.theta) max(data.theta)]); ylim([0 1]);
    set(gca,'FontSize', fontSize)
    xlabel('theta'); ylabel('accuracy')
    addLegendOutside(keys,'NorthEastOutside');
    if ~strcmp(type,'none')
        set(gcf,'PaperUnits','inches','PaperPosition',printSize);
        fn=strrep(strrep(fn,'.tsv',''),'tbl-','');
        fn=strrep(strrep(fn, '0.', ''),'/','_');
        saveas(gcf,[figPrefix '/' fn '_th_reca-lgd.eps'],'epsc')
    end
end;end;end;
close all;

% draw a summary figure of all graph configurations
% matlab put lower dimension first
labels=cell(numel(FILENAMES),numel(GRAPHS),numel(WINDOWS));
for iw=1:numel(WINDOWS); for ig=1:numel(GRAPHS); for ifn=1:numel(FILENAMES);
	fn=[WINDOWS{iw} '/' GRAPHS{ig} '/' FILENAMES{ifn}]
	data=smyLoader([dataPrefix fn]);
    datas(ifn,ig,iw)=data;
    s=[WINDOWS{iw} '-' GRAPHS{ig}];
    labels(ifn,ig,iw)=mat2cell(s,1,length(s));
end;end;end;

type='eps';
%type='png';
%type='none';
close all
drawSmyPRA('minsup','minSup','alpha',1,datas,labels,figPrefix,'smy-alpha-1','ms',type,20);
drawSmyPRA('theta','theta','alpha',1,datas,labels,figPrefix,'smy-alpha-1','th',type,20);
drawSmyPRA('alpha','alpha','','all',datas,labels,figPrefix,'smy','al',type,20);
close all


keys=drawEBGroupOne(datas,labels,'theta', 'alpha', 0.5,'recall');
keys=drawEBGroupOne(datas,labels,'theta', 'alpha', 1.5,'recall');
set(gca,'FontSize', 20)
ylim([0 1]);xlabel(lblX); ylabel('recall')
columnlegend(2,keys,'Location','SouthEast');
set(gcf,'PaperUnits','inches','PaperPosition',[0 0 6 4.5])
saveas(gcf,[figPrefix 'smy-alpha-0_5_th_reca.png'],'png')
saveas(gcf,[figPrefix 'smy-alpha-0_5_th_reca.eps'],'epsc')

