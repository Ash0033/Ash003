#include <iostream>
using namespace std;
struct Node {//լիստի տարր(node)
    int data;       //տվյալ
    Node* next;     //հղում դեպի հաջորդ տարր
};

class LinkedList {
private:
    Node* head; //առաջին node-ի հասցեն

public:
    LinkedList() {
        head = nullptr; //սկզբում դատարկ ցուցակ
    }

    void append(int value) {// Ավելացնել տարր վերջում
        Node* newNode = new Node();//դինամիկում նոր node կպահի
        newNode->data = value;
        newNode->next = nullptr;

        if (head == nullptr) {
            head = newNode;
        } else {
            Node* temp = head;
            while (temp->next != nullptr) {
                temp = temp->next;
            } //գտնում ենք վերջի node-ը
            temp->next = newNode; 
        } }
    // Տպել լիստը
    void display() {  
        Node* temp = head; //սկսում ենք տպել head-ից
        while (temp != nullptr) //մինչ temp-ը դառնում է nullptr
         {
            cout << temp->data << " ";
            temp = temp->next;
        }
        cout << endl;
    }
// Ջնջել լիստը (հիշողությունից ազատել)
    ~LinkedList() {
        Node* current = head;//պահում ենք առաջինը
        while (current != nullptr) {
            Node* next = current->next;
            delete current; //ջնջում
            current = next; //անցնում հաջորդին
        }
    }
};

int main() {
    LinkedList list; //ստեղծում եմ օբյեկտ
    list.append(10);
    list.append(20);
    list.append(30);
           cout << "List elements: ";
       list.display();
    return 0;
}
