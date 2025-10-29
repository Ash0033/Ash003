#include <iostream>
using namespace std;

class vector {
private:
    int* data;  // ցույց է տալիս զանգվածի վրա
    int capacity;  // ամբողջ հասանելի հիշողությունը
    int size; // հիմա քանի տարր կա vector-ում
public:
    vector()  {
        data = new int[1]; // սկզբում պահում ենք 1 տարր տեղ
        capacity = 1;
        size = 0; //դեռ չենք ավելացրե տարր
    }

    void push_back(int value) {
        if (size == capacity) {
            resize(); // եթե լցվելա, մեծացնենք տեղը
//այս ֆ-ն կրկնապատկում է վեկտորի չափը(capacity),ստեղծումա նորը copy անում հինը ու ազատումա հինը
        }
        data[size] = value;//ավելացնում ենք տարր վերջում
        size++; //մեծացնում ենք
    }

    void resize() {
        capacity *= 2; // կրկնապատկում ենք հզորությունը
        int* newData = new int[capacity];
        for (int i = 0; i < size; i++) {
            newData[i] = data[i];
        }
        delete[] data; // ազատում ենք հինը
        data = newData;
    }

    int get(int index) {
        if (index < 0 || index >= size) {
            cout << "Index-ը դուրսա սահմ.-երից";
            return -1;//սխալ
        }
        return data[index]; //ճիշտա արժեքը վեր.
    }

    int getsize() {
        return size;
        //վերադարձնում ենք տարրերի քանակը
    }

    ~vector() { // Destructor-ազատում ենք հիշողությունը
        delete[] data;
    }
};

int main() {
    vector v;

    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

          cout << "Vector size: " << v.getsize() << endl;

    for (int i = 0; i < v.getsize(); i++) {
        cout << v.get(i) << " ";
    }
    cout << endl;
   return 0;
}