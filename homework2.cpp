#include <iostream>

int main(){
int arr[]={0,1,2,3,0,1,3,7};
int size=sizeof(arr)/sizeof(arr[0]);
int j=0;
for(int i=0; i<size; i++){
    j=j^arr[i];
}
int rightj = j & (-j); //գտնում ենք ամենաջ բիթը
int num1=0, num2=0;

for(int i=0; i<size; i++) {
    if(arr[i] & rightj)
    num1^=arr[i]; //համապատասխան
    else 
    num2^=arr[i]; //չի համապատասխանում  
}

std::cout<<num1<<" և "<<num2<<std::endl;
return 0; 
}