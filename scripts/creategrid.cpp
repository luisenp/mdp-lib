#include <cstdlib>
#include <iostream>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

int dx[] = {0, 1, 0, -1};
int dy[] = {-1, 0, 1, 0};

using namespace std;

struct state_t 
{
  int x;
  int y;
  int dir;
} ;


state_t forward(state_t currentState)
{
  state_t nextState;
  nextState.x = currentState.x + dx[currentState.dir];
  nextState.y = currentState.y + dy[currentState.dir];
  nextState.dir = currentState.dir;
  return nextState;
}


state_t left(state_t currentState)
{
  state_t nextState;
  nextState.dir = (currentState.dir - 1) % 4;
  nextState.x = currentState.x + dx[nextState.dir];
  nextState.y = currentState.y + dy[nextState.dir];
  return nextState;
}


state_t right(state_t currentState)
{
  state_t nextState;
  nextState.dir = (currentState.dir + 1) % 4;
  nextState.x = currentState.x + dx[nextState.dir];
  nextState.y = currentState.y + dy[nextState.dir];
  return nextState;
}


vector<state_t> branch(state_t currentState)
{
  vector<state_t> newStates;
  
  double pForward = (rand() + 0.0) / RAND_MAX;
  double pLeft = (rand() + 0.0) / RAND_MAX;
  double pRight = (rand() + 0.0) / RAND_MAX;
  
  cout << pForward << " " << pLeft << " " << pRight << endl;
  
  if (pForward < 0.95)
    newStates.push_back(forward(currentState));
  if (pLeft < 0.2)
    newStates.push_back(left(currentState));
  if (pRight < 0.2)
    newStates.push_back(right(currentState));
  
  return newStates;
}


bool checkLimits(state_t state, int sx, int sy) 
{
  int x = state.x;
  int y = state.y;
  return (x >= 0 && y >= 0 && x < sx && y < sy);
}


int main()
{
  int sx = 21, sy = 21;
  vector< vector<bool> > visited(sx, vector <bool> (sy, false));
  srand(time(NULL));
  
  state_t initialState;
  initialState.x = sx / 2 + 1;
  initialState.y = 0;
  initialState.dir = DOWN;
  
  list<state_t> stack;
  stack.push_back(initialState);
  while (!stack.empty()) {
    state_t currentState = stack.back();
    stack.pop_back();
    if (!checkLimits(currentState, sx, sy) || 
          visited[currentState.x][currentState.y])
      continue;
    visited[currentState.x][currentState.y] = true;
    cout << currentState.x << " " << currentState.y << " " << currentState.dir << endl;
    for (state_t nextState : branch(currentState)) {
      stack.push_back(nextState);
    }
  }
  
  bool goalChosen = false;
  for (int x = 0; x < sx; x++) {
    string line = "";
    for (int y = 0; y < sy; y++) {
      char c = visited[x][y] ? '.' : 'x';
      if (x == initialState.x && y == initialState.y)
        c = 'S';
      double pick = (rand() + 0.0) / RAND_MAX;
      if (c == '.' && y > sy / 2 && !goalChosen && pick < 0.03) {
        c = 'G';
        goalChosen = true;
      }
      line += c;
      line += c;
    }
    cout << line << endl << line << endl;
  }
}
