% Example usage:

fn = '/Volumes/nico/Images/20110127/1pixelsStepsAq.tsf';
data = importTSFData(fn);
mc = matchChannels(data, 1000);
[r, stagepos] = matchFrames(mc, 500);
plot(stagepos(:,1), stagepos(:, 2));
hold;
plot(stagepos(:,3), stagepos(:, 4));