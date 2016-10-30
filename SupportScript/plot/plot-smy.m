WINDOWS=['p-s20-10';'p-s30-12';'p-s30-20'];
GRAPHS=['graph-0.5';'graph-0.8'];
FILENAMES=['tbl-func.tsv'];
%FILENAMES=['tbl-func.tsv';'tbl-func-tr.tsv']

dataPrefix='../../data_adhd/summary/';
figPrefix='../../data_adhd/figure/';

close all
for iw=1:size(WINDOWS,1); for ig=1:size(GRAPHS,1); for ifn=1:size(FILENAMES,1);
	fn=[WINDOWS(iw,:) '/' GRAPHS(ig,:) '/' FILENAMES(ifn,:)]
	data=smyLoader([dataPrefix fn]);
	drawRawPRA('minSup','ms',data.minsup,'T',data.theta,'A',data.alpha,data,figPrefix,fn)
    close all;
    drawRawPRA('theta','th',data.theta,'M',data.minsup,'A',data.alpha,data,figPrefix,fn)
	close all;
end;end;end;

