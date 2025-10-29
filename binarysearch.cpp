#include <iostream>
using namespace std;

int binarySearch(int arr[], int n, int target) {
    int low = 0, high = n - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;//միջին ինդեքս
        if (arr[mid] == target)
            return mid;
        else if (arr[mid] < target)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return -1; // չի գտնվել
}

int main() {
    int arr[] = {1, 3, 5, 7, 9, 11, 13};
    int n = sizeof(arr)/sizeof(arr[0]);
    int target;

    cout << "որոնվող թիվը՝ ";
    cin >> target;

    int result = binarySearch(arr, n, target);
    if (result != -1)
        cout << "Թիվը գտնվել է ինդեքսով: " << result << endl;
    else
        cout << "Թիվը զանգվածում չկա։" << endl;

    return 0;
}
