#ifndef MOVE_H
#define MOVE_H

class Move
{
public:
    Move(int s, int e);
    ~Move();
    int start;
    int end;
    int flag;
    int get_flag(int start, int end);
private:
};

#endif
