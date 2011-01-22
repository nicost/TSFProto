% given a 2-color tsf array of format:
% X, Y, S, Channel, Frame
% return an array with only spots found in both channels in the form:
% Xch1 Ych1 Xch2 Ych2 Frame

function [matched] = matchChannels(data, maxdiff)
matched = [];
sizem = [];
for frame = 1 : max(data(:,5))
    im1pointCh1 = find(data(:,4) == 1 & data(:,5) == frame);
    im1pointCh2 = find(data(:,4) == 2 & data(:,5) == frame);
    clear dt;
    dt = DelaunayTri(data(im1pointCh2,1:2));
    
    for spot = im1pointCh1'
        pid = nearestNeighbor(dt, data(spot,1:2));
        pid = im1pointCh2(pid);
        % todo: check if there already is a match, if so, take the one
        % closest by
        if (abs (data(pid,1) - data(spot,1)) < maxdiff & abs(data(pid,2) - data(spot,2)) < maxdiff)
            matched = [matched ; data(spot,1) data(spot,2) data(pid,1) data(pid,2) frame];
        end
    end
    g = size(find(matched(:,5) == frame));
    sizem = [sizem; g];
end
sizem

                
    
    
