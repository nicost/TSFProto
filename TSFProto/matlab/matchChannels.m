% given a 2-color tsf array of format:
% X, Y, S, Channel, Frame
% return an array with only spots found in both channels in the form:
% Xch1 Ych1 Xch2 Ych2 Frame

function [matched] = matchChannels(data, maxdiff)
matched = [];
for frame = 1 : max(data(:,5))
    im1pointCh1 = find(data(:,4) == 1 & data(:,5) == frame);
    im1pointCh2 = find(data(:,4) == 2 & data(:,5) == frame);
    
    dt = DelaunayTri(data(im1pointCh2,1), data(im1pointCh2,2));
    
    for spot = im1pointCh1'
        pid = nearestNeighbor(dt, [data(spot,1) data(spot, 2)]);
        pid = pid + im1pointCh2(1);
        if (abs (data(pid,1) - data(spot,1)) < maxdiff & abs(data(pid,2) - data(spot,2)) < maxdiff)
            matched = [matched ; data(spot,1) data(spot,2) data(pid,1) data(pid,2) frame];
        end
    end
end

                
    
    
