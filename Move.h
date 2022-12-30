#ifndef MOVE_H
#define MOVE_H

class Move
{
public:
    Move(int s, int e);
    Move(int s, int e, int type);
    ~Move();
    int start;
    int end;
    int promote;
private:
};

#endif
