% given a filename containing tsf data, return a matrix with columns:
% X, Y, S, Channel, Frame
function [data] = importTSFData(fileName)
l = mtsf(fileName);
data(1,:) = l.getX;
data(2,:) = l.getY;
data(3,:) = l.getS;
data(4,:) = l.getChannel;
data(5,:) = l.getPos;
data = data';