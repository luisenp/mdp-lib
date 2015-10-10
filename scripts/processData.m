clear all
clc

data = dlmread('../results-medium.txt');
numIterations = size(data, 1);
for i = 1 : numIterations
  for j = 1 : 3 : size(data, 2)    
    stateActionIndex = data(i, j);
    if stateActionIndex == -1
      break;
    end
    timeStep = data(i, j + 1);
    value = data(i, j + 2);
    QValues(stateActionIndex, timeStep : numIterations) = value;
  end
end
