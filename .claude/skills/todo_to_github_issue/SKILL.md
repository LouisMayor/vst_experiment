---
name: todo_to_gh_issues
description: generate github issues from todo comments in code
disable-model-invocation: true
---

Analyze and generate github issue(s): $ARGUMENTS.

1. index the project using `codebase-memory` mcp
2. check if asked to look at specific file, or directory
    1. if a directory has been provided ONLY check all files in the directory
        1. DO NOT check every file
    2. if a specific file has been provided ONLY check the specific file
        1. DO NOT check any other file
    3. else, check every file with `.h` and `.cpp` extensions
3. for each todo comment ensure they are not linked to a github issue
    1. they are linked if they contain `(gh_issue: <link>)`
        ```
        // todo: do x (gh_issue: <link>)`
        ```
    2. find the next todo comment
3. if the todo comment is not linked
    1. create a github issue
        1. you MUST use the todo comment content as the title
        2. you MUST assign the issue to @me 
        3. you MUST contextualize the todo and use as the body (file name and line. optional: relevant function)
        4. you MUST double check your markdown will display correctly
        5. cli doc: https://cli.github.com/manual/gh_issue_create
            ```
            gh issue create -t <title> -b <body> -a @me 
            ```
    2. update the todo in code with a link to the newly created issue
        3. the link should have been provided as a response from the issue create cli call
        1. e.g. `(gh_issue: <link>)`
            ```
            // todo: do x (gh_issue: <link>)
            ```
