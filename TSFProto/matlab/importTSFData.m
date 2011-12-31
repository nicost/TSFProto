% given a filename containing tsf data, return a matrix with columns:
% X, Y, S, Channel, Frame
function data = importTSFData(fileName)
l = mtsf(fileName);
data = zeros(l.nrSpots, 10);
for i=1:l.nrSpots
    data(i,:) = l.getNextSpot;
end

%data = l.getNextSpot
%data = zeros(5, l.nrSpots);
%data(1,:) = l.getX;
%data(2,:) = l.getY;
%data(3,:) = l.getS;
%data(4,:) = l.getChannel;
%data(5,:) = l.getFrame;
%data = data';