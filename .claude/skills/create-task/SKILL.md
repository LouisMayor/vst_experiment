---
name: create-task
description: create a task for a requirement
disable-model-invocation: true
---

Analyze and create a task: $ARGUMENTS

1. look in `./project_management/requirements/` 
    1. check any of the `.md` files that will contain the task
2. ensure the requirement isn't already an existing task
    1. it should have `task: task_number`
3. understand the requirement description
4. index the project using `codebase-memory` mcp
5. search the codebase for relevant files
6. add task into `./project_management/tracking/todo/`
7. follow the file structure
  ```md
  # TASK/BUG NAME AND BRIEF SUMMARY
  ## DESCRIPTION
    - ...
  ## RELEVANT INFOMATION
    - code files
    - considerations
  ```
8. append the newly created task name to the requirement header
    1. `task: task_number`
