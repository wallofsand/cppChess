#ifndef CHESS_STACK_H
#define CHESS_STACK_H

namespace ch_stk
{
    template <class T>
    struct StackNode {
        StackNode* next;
        T* pos;

        StackNode() : next(nullptr), pos(nullptr) {};
        StackNode(T* ch) : next(nullptr), pos(ch) {};
        inline ~StackNode() {
            delete next;
            delete pos;
        };
    };

    template <class T>
    struct ChessStack {
        ChessStack() : top(new StackNode<T>()) {};
        StackNode<T>* top;

        inline bool is_empty() { return top == nullptr; };
        inline void push(T* val) {
            StackNode<T>* node = new StackNode<T>(val);
            node->next = top;
            top = node;
        };
        inline void pop() {
            if (is_empty() || top->next == nullptr) return;
            StackNode<T>* tmp = top;
            top = top->next;
            tmp->next = nullptr;
            delete tmp;
        };
    };
} // namespace ch_stk

#endif
