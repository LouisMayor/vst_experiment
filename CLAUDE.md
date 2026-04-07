# Rules
- `.claude/rules/cpp_code_style.md`
- `.claude/rules/code_design.md`

# Workflow
- if you need a dependency to be installed, always ask for permission first.
  - use `gh` or `git` to get dependencies from github
  - use `brew`, `winget` or the system package manager to install non-github dependencies

# Project navigation
- use the `codebase-memory` mcp to navigate code files, and search for code, etc...

# Build system
- we are using CMAKE 4.3
- we are supporting multi-platform (Windows, MacOS and Linux)
  - primarily developed on MacOS (15.7.1) at this current time

# Project Management
- we do not have any bug/issue/task tracking software
  - in lieu, we should do this locally within a directory
    - `./project_management/tracking/`
- project requirements will be written in markdown files
  - they will be located in `./project_management/requirements/`

# Testing
- we will be writing unit tests to verify feature, etc, work
- we currently do not have a unit testing framework
  - while this is true, any features we do implement, keep in my what and why we should write unit test for them.

# Project stucture
- `./src` is where all source code lives
- `./src/thirdparty` is where any third party dependencies will be go

# Third party libs
- Steinberg VST3
  - sdk github: `https://github.com/steinbergmedia/vst3sdk`
  - sdk webpage: `https://steinbergmedia.github.io/vst3_doc/vstsdk/index.html`
  - documentation: `https://steinbergmedia.github.io/vst3_dev_portal/pages/index.html`
