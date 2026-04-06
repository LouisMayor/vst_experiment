# Project Overview
This is an experimentation project using Steinberg's Vst3 SDK and AI assistance.

I am using Opencode, and have started using Minimax 2.5 *(Ollama Cloud free, and the free model access from Opencode)*

I opted to use Claude Code's AI setup, as Opencode supports it, and I may switch to Claude at a later point.

# Project Setup
I am using CMAKE as a build system, primarily because I want to support Windows, MacOS and Linux

I have opted to manaully do project management, more about that in the ***AI Assistant workflow***

## AI Assistant workflow
I've opted use a system to work with my AI assistant, which I think is a little unconventional?

This system, can but currently doesn't, use Github issues & project features

### How does it work?

I create a requirements file, which can be a new file per set of requirements. At the time of writing, it is just one file. This contains a high level description of what I want to add.

see `project_management/requirements/startup_requirements.md`

I then ask my assistant to expand on my initial list, and create a task from this.

I have sometimes also had, although not in this project, asked AI to make a proposal based on this document, which expands on each feature, etc, and provides contextual infomation.

This can be code references, documentation, etc.

In this project, I've skipped the proposal step, and asked the assistant to create a task markdown file. I've prompted the AI in a similar way.

This is mostly designed for giving my AI assistant a good starting point when I, later, ask it to implement/work on a given task.
I use the custom skill I created.

see `.claude/skills/create-task/SKILL.md`

The last step, is to get the AI assistant to execute a task. I create a new context, and pass the task id, e.g. task_001

see `project_management/tracking/done/*.md`

*I think using Github to manage this would work better*

I have a custom skill for this. see `.claude/skills/work-on/SKILL.md`
