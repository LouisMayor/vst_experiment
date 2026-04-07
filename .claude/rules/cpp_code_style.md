---
paths:
  - "src/**/*.{h,cpp}"
---

# C++ Code Style 
- use snake_case
  - functions, variables, etc...
- use tab indentation
- function inputs should be prefixed with `in_`
  ```cpp
  void foo(int in_x) {
      ...
  }
  
  void bar(const std::string& in_str) {
      ...
  }
  ```
- function inputs that are non-const references should be prefixed with `out_`
  ```cpp
  void foo(int& out_x) {
      out_x = 5;
  }
  ```
- try to keep alignment with other variables
  ```cpp
  int           x     = 1;
  std::string   xyz   = 2;
  ```
- use smart pointers
  - never use dumb pointers
- braces should open on the same line
  ```cpp
  int get_random() {
      return 42;
  }
  ```
