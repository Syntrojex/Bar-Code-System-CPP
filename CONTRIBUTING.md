# Contributing Guidelines

Welcome to **BarcodeHub**! This document outlines the contribution workflow and development guidelines for all team members working on **Bar-Code-System-CPP**.

Please follow these guidelines to maintain a consistent and organized development workflow.

## 1. Clone the Repository

Clone the repository and navigate to the project directory:

```bash
git clone https://github.com/Syntrojex/Bar-Code-System-CPP.git
cd Bar-Code-System-CPP
```

## 2. Switch to Your Assigned Branch

Each team member has an assigned branch. Switch to your respective branch before making any changes.

```bash
git checkout your-branch
```

### Available Team Branches

| Team Member | Branch |
|---|---|
| Muhammad Mustafa Amir | `mustafa` |
| Ali Abdullah | `ali` |
| Farzam Zeeshan | `farzam` |
| Shauzab Mukhtar | `shauzab` |

**Important:** Always work on your assigned branch. Do not make changes directly on `main`.

## 3. Synchronize Your Branch

Before starting new work, make sure your local branch has the latest changes:

```bash
git pull origin your-branch
```

This helps prevent conflicts and keeps your local development environment up to date.

## 4. Make Your Changes

Implement the changes related to your assigned task.

Follow these practices:

- Maintain the existing project structure.
- Follow the established C++ coding conventions.
- Write clean, readable, and maintainable code.
- Avoid modifying unrelated files.
- Test your changes before committing.

## 5. Stage and Commit Changes

Review your modifications:

```bash
git status
```

Stage the relevant files:

```bash
git add .
```

Create a meaningful commit:

```bash
git commit -m "Add barcode generation functionality"
```

Keep commit messages concise and descriptive. Each commit should represent a meaningful change.

## 6. Push Your Changes

Push your commits to your assigned remote branch:

```bash
git push origin your-branch
```

## 7. Create a Pull Request

Once your changes have been pushed, create a Pull Request to propose merging them into `main`.

1. Open the project repository on GitHub.
2. Select **Compare & pull request**.
3. Set the base branch to `main`.
4. Select your assigned branch as the compare branch.
5. Provide a clear and descriptive title.
6. Explain the changes and their purpose in the description.
7. Create the Pull Request.

## 8. Review and Merge

All Pull Requests must go through the project's review process before being merged.

- The project lead reviews submitted Pull Requests.
- Address any requested changes before merging.
- Ensure the proposed changes do not introduce unnecessary modifications or conflicts.
- Only reviewed and approved contributions should be integrated into `main`.
- Team members must not push directly to `main`.

## 9. Automatic Branch Synchronization

The project uses a GitHub Actions workflow to automatically synchronize team branches with the latest `main` branch.

Whenever changes are pushed to `main`, the workflow:

- Detects existing team branches automatically.
- Attempts to merge the latest `main` changes into each branch.
- Pushes successfully synchronized branches back to GitHub.
- Reports branches that encounter merge conflicts or synchronization failures.

## General Guidelines

- Always work on your assigned branch.
- Keep commits meaningful and focused.
- Follow the established project structure and coding conventions.
- Test your changes before submitting a Pull Request.
- Keep Pull Requests limited to their intended tasks.
- Avoid committing generated build files and unnecessary dependencies.
- Communicate with the project lead when clarification is needed.
- Never commit directly to `main`.
---

Thank you for contributing to **BarcodeHub**!
