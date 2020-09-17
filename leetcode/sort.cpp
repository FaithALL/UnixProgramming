#include <iostream>
#include <algorithm>
using namespace std;


template <typename type>
void bubbleSort(type arr[], int n) {
    for (int i = 0; i < n - 1; ++i) 
        for (int j = 0; j < n - 1 - i; ++j) 
            if (arr[j] > arr[j + 1]) 
                std::swap(arr[j], arr[j + 1]);
}

template <typename type>
void selectSort(type arr[], int n) {
    for (int i = 0; i < n - 1; ++i) {
        int imin = i;
        for (int j = i + 1; j < n; ++j) {
            if (arr[j] < arr[imin]) imin = j;
        }

        std::swap(arr[i], arr[imin]);
    }
}

template <typename type>
void insertSort(type arr[], int n) {
    for (int i = 1; i < n; ++i) 
        for (int j = i; j > 0 && arr[j] < arr[j - 1]; --j) 
            std::swap(arr[j], arr[j - 1]);
}


template <typename type>
void quickSort(type arr[], int start, int end) {
    if (start < end) {
        int refer = arr[start];
        int left = start;
        int right = end;
        int cur = start + 1;

        while (cur < right) {
            if (arr[cur] < refer) arr[left++] = arr[cur++];
            else std::swap(arr[cur], arr[right--]);
        }

        arr[cur] = refer;
        quickSort(arr, start, cur - 1);
        quickSort(arr, cur + 1, end);
    }
}

template <typename type>
void quickSort(type arr[], int n) {
    quickSort(arr, 0, n - 1);
}

template <typename type>
void mergeSort(type arr[], int n) {

}

int main() {
    int arr[100];
    srand(time(0));

    for (int i = 0; i < 100; ++i) {
        arr[i] = rand() % 1000;
    }

    
    //bubbleSort(arr, 100);
    //selectSort(arr, 100);
    //insertSort(arr, 100);
    //quickSort(arr, 100);
    mergeSort(arr, 100);
    for (auto x : arr) {
        cout << x  << " ";
    }
    cout << endl;
    return 0;
}
