nnetInput = [];
nnetOutput = [];
numActions = 9;
window = 2;
actionLength = 5;
successes = 0;
failures = 0;
for state = 1 : numActions : size(QValues, 1)
  for t = actionLength * window + 1 : actionLength : size(QValues, 2)
    realQValues = zeros(numActions,1);
    predictedQValues = zeros(numActions,1);
    for action = 0 : numActions - 1
      previousQvaluesSA = QValues(state + action, t - window * actionLength : t - actionLength);
      % A = [(1:actionLength * (window - 1) + 1)', ones(actionLength * (window - 1) + 1, 1)];
      % b = previousQvaluesSA(:);
      % A = [(1 : window)', ones(window, 1)];
      % b = previousQvaluesSA(1 : actionLength : end)';
      % m = A\b;
      % predictedQValues(action + 1) = QValues(state + action, t - actionLength) + m(1) * actionLength;
      % predictedQValues(action + 1) = QValues(state + action, t - actionLength) + m(1);
      m = QValues(state + action, t - actionLength) - QValues(state + action, t - actionLength - 1);
      predictedQValues(action + 1) = QValues(state + action, t - actionLength) + m(1) * actionLength;
      realQValues(action + 1) = QValues(state + action, t);
    end
    [tmp, bestPredictedAction] = min(predictedQValues);
    [tmp, bestAction] = min(realQValues);
    successes = successes + (bestPredictedAction == bestAction);
    failures = failures + (bestPredictedAction ~= bestAction);
%      if bestPredictedAction ~= bestAction
%        keyboard
%      end
  end
end
