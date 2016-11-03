function addLegendOutside(keys, loc)
    if isempty(strfind(lower(loc),'outside'))
        loc=[loc 'outside'];
    end
    h=legend(keys(:), 'Location', loc);
    if isempty(strfind(lower(loc),'south')), %north or default
        while h.Position(2)<=0.01;
            h.FontSize=h.FontSize-1;
        end
    else % south
        while h.Position(2)+h.Position(4)>=1;
            h.FontSize=h.FontSize-1;
        end
    end
end
