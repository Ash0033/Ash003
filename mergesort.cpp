#include <iostream>
using namespace std;

// Ֆունկցիա, որը միավորում է երկու տեսակավորված մասերը
void merge(int arr[], int left, int mid, int right) {
    int n1 = mid - left + 1;  // ձախ մասի չափը
    int n2 = right - mid;  // աջ մասի չափը

    int L[n1], R[n2]; // ժամանակավոր զանգվածներ

    // Պատճենում ենք տվյալները ձախ և աջ մասերի մեջ
    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;

    // Միավորում ենք երկու մասերը՝ ըստ փոքր արժեքների
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Եթե ձախ մասում էլեմենտներ են մնացել՝ ավելացնում ենք
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Եթե աջ մասում էլեմենտներ են մնացել՝ ավելացնում ենք
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

// Ռեկուրսիվ Merge Sort ֆունկցիա
void mergeSort(int arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;  // գտնում ենք կեսը

        // Տեսակավորում ենք ձախ ու աջ մասերը
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);

        // Միավորում ենք տեսակավորված մասերը
        merge(arr, left, mid, right);
    }
}

int main() {
    int arr[] = {8, 3, 5, 4, 7, 6, 1, 2};
    int n = sizeof(arr) / sizeof(arr[0]);

    cout << "Սկզբնական զանգվածը: ";
    for (int i = 0; i < n; i++) cout << arr[i] << " ";
    cout << endl;

    mergeSort(arr, 0, n - 1); // Կանչում ենք merge sort-ը

    cout << "Տեսակավորված զանգվածը: ";
    for (int i = 0; i < n; i++) cout << arr[i] << " ";
    cout << endl;

    return 0;
}
