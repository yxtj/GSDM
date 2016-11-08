function keys=drawNMotif(X, lblGP, GP, Y, flagCLF)

    COLORS=['rgbymck'];
    MARKERS=['o+*.xsd^v><ph'];
    lenc=length(COLORS);
    lenm=length(MARKERS);

    uGP=unique(GP);

    if exist('flagCLF','var') && flagCLF
        clf;
    end
    keys=cell(length(uGP),1);
    hold all
    for i=1:length(uGP)
        idx=find(GP==uGP(i));
        [~,tmp]=sort(X(idx));
        idx=idx(tmp);
        pc=mod(i,lenc); if pc==0; pc=lenc; end;
        pm=mod(i,lenm); if pm==0; pm=lenm; end;
        plot(X(idx),Y(idx),[COLORS(pc), '-', MARKERS(pm)]);
        keys(i)=cellstr([lblGP,num2str(uGP(i))]);
    end
    hold off;
    grid on;

end
