WINDOWS={'p-s20-10';'p-s30-12';'p-s30-20'};
GRAPHS={'graph-0.5';'graph-0.8'};
FILENAMES={'tbl-func.tsv'};
%FILENAMES={'tbl-func.tsv';'tbl-func-tr.tsv'};

dataPrefix='../../data_adhd/summary/';
figPrefix='../../data_adhd/figure/';

% draw all lines of a single graph configuration
type='eps';
%type='png';
close all
for iw=1:numel(WINDOWS); for ig=1:numel(GRAPHS); for ifn=1:numel(FILENAMES);
	fn=cell2mat([WINDOWS(iw) '/' GRAPHS(ig) '/' FILENAMES(ifn)])
	data=smyLoader([dataPrefix fn]);
	drawRawPRA('minSup','ms',data.minsup,'T',data.theta,'A',data.alpha,data,type,figPrefix,fn)
    close all;
    drawRawPRA('theta','th',data.theta,'M',data.minsup,'A',data.alpha,data,type,figPrefix,fn)
	close all;
end;end;end;
