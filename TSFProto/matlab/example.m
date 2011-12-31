% Example usage:

fn = '/Users/nico/Images/GridData/GridA/spotlist.tsf';
data = importTSFData(fn);
mc = matchChannels(data, 1000);
[r, stagepos] = matchFrames(mc, 500);
plot(stagepos(:,1), stagepos(:, 2), 'b');
hold;
plot(stagepos(:,3), stagepos(:, 4), 'r');
hold off;

results = [];
input_points = mc(find(mc(:,5) == 1), 3:4);
base_points = mc(find(mc(:,5) == 1), 1:2);
t = cp2tform(input_points, base_points, 'lwm');
for tp=min(mc(:,5)) : max(mc(:,5))
    d = mc(find(mc(:,5) == tp), 1:2);
    e = mc(find(mc(:,5) == tp), 3:4);
    c = tforminv(t, d);
    results = [results; mean(e-c) std(e-c)];
end
mean(results)
std(results)

figure;
plot (results(:,2),'r');
hold;
Current plot held
plot (results(:,1),'b');
hold off;