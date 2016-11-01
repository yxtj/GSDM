function drawRawGroupOne(X, lblGP1, GP1, lblGP2, GP2, Y, showLegend)

    if nargin<7
        showLegend=0;
    end

    COLORS=['rgbymck'];
    MARKERS=['o+*.xsd^v><ph'];

    uGP1=unique(GP1);
    uGP2=unique(GP2);

    clf;
    hold all
    for i1=1:length(uGP1)
        for i2=1:length(uGP2)
            idx=find(GP1==uGP1(i1) & GP2==uGP2(i2));
            plot(X(idx),Y(idx),[COLORS(i1), '-', MARKERS(i2)],'DisplayName', ...
                [lblGP1,num2str(uGP1(i1)),'-',lblGP2,num2str(uGP2(i2))]);
        end
    end
    hold off;
    grid on;
    %xlim([min(X),max(X)]); ylim([0 1]);
    if showLegend
        legend('Location','NorthEastOutside');
    end

end
