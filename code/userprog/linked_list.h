#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include "copyright.h"
#include <functional>


template<typename T>
class LinkedList {
    class Node {
        public:
            T* value;
            Node* next;

            Node(T* v) { this->value = v; }

            static void FreeNode(Node* elem) {
                if (elem == nullptr) { return; }
                FreeNode(elem->next);
                delete elem;
            }
    };

    Node* head;
    Node* end;

    public:
        LinkedList() { head = end = nullptr; }
        ~LinkedList() { Node::FreeNode(head); }

        template<typename X>
        T* FindInList(std::function<bool (T* , X)> fun, X value) {
            for (Node* elem = head; elem != nullptr; elem = elem->next) {
                if (fun(elem->value, value)) {  return elem->value; }
            }
            return nullptr;
        }

        T* FindInList(T* toFind) {
            for (Node* elem = head; elem != nullptr; elem = elem->next) {
                if (elem->value == toFind) { return toFind; }
            }
            return nullptr;
        }

        bool AddInList(T* elem) {
            if (end != nullptr) {
                end->next = new Node(elem);
                end = end->next;
                return true;
            }
            head = end = new Node(elem);
            return true;
        }

        bool IsEmpty() { return head == nullptr; }

        bool OnlyOneElement() { return head != nullptr && head == end; }

        bool RemoveInList(T* elem) {
            if (head == nullptr){ return false; }

            if (head->next == nullptr) {
                if (head->value == elem) {
                    delete head;
                    head = end = nullptr;
                    return true;
                }
                return false;
            }

            for (Node* current = head; current->next != nullptr; current = current->next) {
                if (current->next->value == elem) {
                    Node* tmp = current->next->next;
                    if (end == current->next) { end = current; }
                    delete current->next;
                    current->next = tmp;
                    return true;
                }
            }
            return false;
        }

        T* RemoveFront() {
            if (head == nullptr) { return nullptr; }

            Node* tmp = head;
            T* value = head->value;
            head = head->next;
            if (end == tmp) { end = nullptr; }
            delete tmp;
            return value;
        }
};

#endif // LINKED_LIST_H
