#include <iostream>

int main()
{
    int array[] = { 0 };
    const int count = sizeof(array) / sizeof(int);
    int check = 0;
    for (int i=0; i<count; i++)
        check ^=array[i];
    std::cout << check << std::endl;
    return 0;
}