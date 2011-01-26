% Positions corrected for stage movement

corrspots = [];
corrspotsall = [];
for spotnr = unique(r(:, 6))'
    spot = r(find(r(:,6) == spotnr), 1:2);
    spot(:,3) = spot(:,1) - stagepos(:, 6);
    spot(:,4) = spot(:,2) - stagepos(:, 7);
    corrspots = [corrspots; spotnr mean(spot(:,3)) mean(spot(:,4)) std(spot(:,3)) std(spot(:,4))]; 
    spot(:,5) = spotnr;
    corrspotsall = [corrspotsall; spot];
end
