function [mat1,idx1]=getFrequencyMat(dth, idx, lmin, lmax, show)
    [d1,idx1]=filterByLength(dth, lmin, lmax);
    idx1=idx(idx1);
    mat1=paddingContainTable(d1, lmax);
    
    [mat1,idx1m]=sortTableByFreq(mat1);
    idx1=idx1(idx1m);
    
    if exist('show','var') && show
        imagesc(mat1);
        caxis([-1,1])
        colormap('hot');
    end
end
