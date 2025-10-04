#include <iostream>

int main(){
int arr[]={0,1,2,3,0,1,3};
int size=sizeof(arr)/sizeof(arr[0]);
int j=0;
for(int i=0; i<size; i++){
    j=j^arr[i];
}
std::cout<<j<<std::endl;
return 0;
}