function keys=drawRawGroupOne(X, lblGP1, GP1, lblGP2, GP2, Y, flagCLF)

    COLORS=['rgbymck'];
    MARKERS=['o+*.xsd^v><ph'];
    lenc=length(COLORS);
    lenm=length(MARKERS);

    uGP1=unique(GP1);
    uGP2=unique(GP2);

    if exist('flagCLF','var') && flagCLF
        clf;
    end
    keys=cell(length(uGP2),length(uGP1));
    hold all
    for i1=1:length(uGP1)
        for i2=1:length(uGP2)
            idx=find(GP1==uGP1(i1) & GP2==uGP2(i2));
            pc=mod(i1,lenc); if pc==0; pc=lenc; end;
            pm=mod(i2,lenm); if pm==0; pm=lenm; end;
            plot(X(idx),Y(idx),[COLORS(pc), '-', MARKERS(pm)]);
            %matlab put lower dimension first
            keys(i2,i1)=cellstr([lblGP1,num2str(uGP1(i1)),'-',lblGP2,num2str(uGP2(i2))]);
        end
    end
    hold off;
    grid on;

end
