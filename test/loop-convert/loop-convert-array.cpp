// RUN: rm -rf %t.cpp
// RUN: grep -Ev "//\s*[A-Z-]+:" %s > %t.cpp
// RUN: loop-convert . %t.cpp -- -I %S/Inputs \
// RUN:         && FileCheck -input-file=%t.cpp %s
// RUN: grep -Ev "//\s*[A-Z-]+:" %s > %t.cpp
// RUN: cp %t.cpp %t.base
// RUN: loop-convert -count-only . %t.cpp -- -I %S/Inputs > %T/out \
// RUN:         && FileCheck -check-prefix=COUNTONLY -input-file=%T/out %s \
// RUN:         && diff %t.cpp %t.base

const int N = 6;
const int NMinusOne = N - 1;
int arr[N] = {1, 2, 3, 4, 5, 6};
int (*pArr)[N] = &arr;
#include "structures.h"
void f() {
  int sum = 0;
  // Update the number of correctly converted loops as this test changes:
  // COUNTONLY: 14 converted
  // COUNTONLY-NEXT: 0 potentially conflicting
  // COUNTONLY-NEXT: 0 change(s) rejected

  for (int i = 0; i < N; ++i) {
    sum += arr[i];
  }
  // CHECK: for (auto & [[VAR:[a-z_]+]] : arr) {
  // CHECK-NEXT: sum += [[VAR]];
  // CHECK-NEXT: }

  for (int i = 0; i < N; ++i) {
    printf("Fibonacci number is %d\n", arr[i]);
    sum += arr[i] + 2;
  }
  // CHECK: for (auto & [[VAR:[a-z_]+]] : arr)
  // CHECK-NEXT: printf("Fibonacci number is %d\n", [[VAR]]);
  // CHECK-NEXT: sum += [[VAR]] + 2;

  for (int i = 0; i < N; ++i) {
    int x = arr[i];
    int y = arr[i] + 2;
  }
  // CHECK: for (auto & [[VAR:[a-z_]+]] : arr)
  // CHECK-NEXT: int x = [[VAR]];
  // CHECK-NEXT: int y = [[VAR]] + 2;

  for (int i = 0; i < N; ++i) {
    int x = N;
    x = arr[i];
  }
  // CHECK: for (auto & [[VAR:[a-z_]+]] : arr)
  // CHECK-NEXT: int x = N;
  // CHECK-NEXT: x = [[VAR]];

  for (int i = 0; i < N; ++i) {
    arr[i] += 1;
  }
  // CHECK: for (auto & [[VAR:[a-z_]+]] : arr) {
  // CHECK-NEXT: [[VAR]] += 1;
  // CHECK-NEXT: }

  for (int i = 0; i < N; ++i) {
    int x = arr[i] + 2;
    arr[i] ++;
  }
  // CHECK: for (auto & [[VAR:[a-z_]+]] : arr)
  // CHECK-NEXT: int x = [[VAR]] + 2;
  // CHECK-NEXT: [[VAR]] ++;

  for (int i = 0; i < N; ++i) {
    arr[i] = 4 + arr[i];
  }
  // CHECK: for (auto & [[VAR:[a-z_]+]] : arr)
  // CHECK-NEXT: [[VAR]] = 4 + [[VAR]];

  for (int i = 0; i < NMinusOne + 1; ++i) {
    sum += arr[i];
  }
  // CHECK: for (auto & [[VAR:[a-z_]+]] : arr) {
  // CHECK-NEXT: sum += [[VAR]];
  // CHECK-NEXT: }

  for (int i = 0; i < N; ++i) {
    printf("Fibonacci number %d has address %p\n", arr[i], &arr[i]);
    sum += arr[i] + 2;
  }
  // CHECK: for (auto & [[VAR:[a-z_]+]] : arr)
  // CHECK-NEXT: printf("Fibonacci number %d has address %p\n", [[VAR]], &[[VAR]]);
  // CHECK-NEXT: sum += [[VAR]] + 2;

  Val teas[N];
  for (int i = 0; i < N; ++i) {
    teas[i].g();
  }
  // CHECK: for (auto & [[VAR:[a-z_]+]] : teas) {
  // CHECK-NEXT: [[VAR]].g();
  // CHECK-NEXT: }
}

struct HasArr {
  int Arr[N];
  Val ValArr[N];
  void implicitThis() {
    for (int i = 0; i < N; ++i) {
      printf("%d", Arr[i]);
    }
    // CHECK: for (auto & [[VAR:[a-z_]+]] : Arr) {
    // CHECK-NEXT: printf("%d", [[VAR]]);
    // CHECK-NEXT: }

    for (int i = 0; i < N; ++i) {
      printf("%d", ValArr[i].x);
    }
    // CHECK: for (auto & [[VAR:[a-z_]+]] : ValArr) {
    // CHECK-NEXT: printf("%d", [[VAR]].x);
    // CHECK-NEXT: }
  }

  void explicitThis() {
    for (int i = 0; i < N; ++i) {
      printf("%d", this->Arr[i]);
    }
    // CHECK: for (auto & [[VAR:[a-z_]+]] : this->Arr) {
    // CHECK-NEXT: printf("%d", [[VAR]]);
    // CHECK-NEXT: }

    for (int i = 0; i < N; ++i) {
      printf("%d", this->ValArr[i].x);
    }
    // CHECK: for (auto & [[VAR:[a-z_]+]] : this->ValArr) {
    // CHECK-NEXT: printf("%d", [[VAR]].x);
    // CHECK-NEXT: }
  }
};