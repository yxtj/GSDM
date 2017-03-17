DATASET={'adhd','abide','abide2','adni'};
GRAPH={'p-s20-10/graph-0.7'};
THETA=0.7;
ALPHA=1;
MOTIF_ID=0;

% ----------- ABIDE -----------

[fn,fnSub]=generateFileName(DATASET{2}, GRAPH{1}, THETA, ALPHA, MOTIF_ID);
[id,type,len]=loadSubjectInfo(fnSub);

th=0.7;
data=loadContainTable(fn);
[dth,idx]=filterByTheta(data, th);
%id_th=id(idx);
%type_th=type(idx);
%len_th=len(idx);
count=getLengthCount(len(idx));
bar(count)
% group1: 10, group2; 16-18, group3: 22-23
group=[10,10;16,18;22,23]';

idxes={};
for i=1:size(group,2);
    subplot(2,2,i);
    [~,idxes{i}]=getFrequencyMat(dth, idx, group(1,i), group(2,i), 1);
end

% show with type:
idx_pos=find(type==1);
idx_neg=find(type==2);

[dth_p,idx_p]=filterByTheta(data(idx_pos), th);
%count=getLengthCount(len(idx_p));
%bar(count)
for i=1:size(group,2);
    subplot(2,2,i);
    getFrequencyMat(dth_p, idx_p, group(1,i), group(2,i), 1);
end

[dth_n,idx_n]=filterByTheta(data(idx_neg), th);
for i=1:size(group,2);
    subplot(2,2,i);
    getFrequencyMat(dth_n, idx_n, group(1,i), group(2,i), 1);
end

% show both type together:
[dth_p,idx_p]=filterByTheta(data(idx_pos), th);
[dth_n,idx_n]=filterByTheta(data(idx_neg), th);
for i=1:size(group,2);
    subplot(2,2,i);
    mat_p=getFrequencyMat(dth_p, idx_p, group(1,i), group(2,i), 0);
    mat_n=getFrequencyMat(dth_n, idx_n, group(1,i), group(2,i), 0);
    mat_all=[mat_p; zeros(1,group(2,i))-1; mat_n];
    imagesc(mat_all);
    colormap('hot');
end


