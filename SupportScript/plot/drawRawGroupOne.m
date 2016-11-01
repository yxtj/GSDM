function keys=drawRawGroupOne(X, lblGP1, GP1, lblGP2, GP2, Y)

    COLORS=['rgbymck'];
    MARKERS=['o+*.xsd^v><ph'];

    uGP1=unique(GP1);
    uGP2=unique(GP2);

    clf;
    keys=cell(length(uGP2),length(uGP1));
    hold all
    for i1=1:length(uGP1)
        for i2=1:length(uGP2)
            idx=find(GP1==uGP1(i1) & GP2==uGP2(i2));
            plot(X(idx),Y(idx),[COLORS(i1), '-', MARKERS(i2)]);
            %matlab put lower dimension first
            keys(i2,i1)=cellstr([lblGP1,num2str(uGP1(i1)),'-',lblGP2,num2str(uGP2(i2))]);
        end
    end
    hold off;
    grid on;

end
