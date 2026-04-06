---
name: work-on
description: work on a task
disable-model-invocation: true
---

Analyze and work on a task: $ARGUMENTS.

1. look in `./project_management/tracking/todo/`
    1. check the task I've asked for with the `.md` extension (use `codebase-memory` mcp first)
    1. if this task isn't in `todo` check if it is in `in_progress` or `done`
    2. let me know what state the task is in
2. migrate into `in_progress`
3. understand the requirements
4. search the codebase for relevant files
5. implement the necessary changes to complete the task
6. when finished move to `done`
    1. create a new section in task called `changes list`
    2. write a summary of the changes under this section
7. index the project using `codebase-memory` mcp
