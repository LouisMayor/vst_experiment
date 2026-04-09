---
paths:
  - "src/**/*.{h,cpp}"
---

Do not apply the following code style to third party library code.

# C++ Code Style
- do not use `using namespace ...`
- always use fully qualified namespaces
- use snake_case
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
- use "K&R" brace style
  ```cpp
  int get_random() {
      return 42;
  }
  ```
