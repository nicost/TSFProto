% given a tsf array of format:
% X1, Y1, X2, Y2, Frame
% returns an array with only those spots found in all frames
% result has the form:
% Xch1 Ych1 Xch2 Ych2 Frame spotNr
% stagepos returns the average positions of each spot at each frame and has
% the form:
% AvgXCh1 AvgYCh1 AvgXCh2 AvgYCh2 Frame CorrAvgXCh1 CorrAvgYCh1 CorrAvgXCh2
% CorrAvgYCh2
% where the "Corr" values are normalized by the first position meaasured 

function [result stagepos] = matchFrames(data, maxdiff)
maxF = max(data(:,5));
minF = min(data(:,5));
mp = find(data(:,5) == minF);
dt = DelaunayTri(data(mp,1:2));
result = data;

for frame = minF : maxF
    pointsFrameN = find(data(:,5) == frame);
    for point = pointsFrameN'
        pid = nearestNeighbor(dt, data(point, 1:2)); 
        result(point,6) = pid;
        if abs (data(pid,1) - data(point,1)) > maxdiff | abs (data(pid,2) - data(point,2)) > maxdiff 
            mp(pid,:) = NaN; 
        end
    end
end

% remove spots with "bad" matches
for x = find(isnan(mp))'
    result(find(result(:,6) == x), :) = [];
end

matched = mp(~isnan(mp));

% also remove spots that were not matched in every frame:
cc = [];
for x = matched'
    cc = [cc; length(find(result(:, 6) == x))];
end
tmp = find(cc(:, 1) ~= max(cc));
for x = tmp'
    result(find(result(:,6) == x), :) = [];
end
            
stagepos = [];
for frame = minF : maxF
    tmp = find(result(:,5)==frame);
    stagepos = [stagepos; mean(result(tmp, 1))  mean(result(tmp, 2)) mean(result(tmp, 3)) mean(result(tmp, 4)) frame];
end

stagepos(:,6) = stagepos(:,1) - stagepos(1,1);
stagepos(:,7) = stagepos(:,2) - stagepos(1,2);
stagepos(:,8) = stagepos(:,3) - stagepos(1,3);
stagepos(:,9) = stagepos(:,4) - stagepos(1,4);

