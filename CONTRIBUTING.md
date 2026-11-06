# Contributing Guidelines

This document explains how team members can contribute to **Bar-Code-System-CPP**.

## 1. Clone the Repository

Clone the repository and navigate into the project directory:

```bash
git clone https://github.com/Syntrojex/Bar-Code-System-CPP.git
cd Bar-Code-System-CPP
```

## 2. Switch to Your Assigned Branch

Switch to your assigned branch. Replace `your-branch` with your actual branch name.

```bash
git checkout your-branch
```

## 3. Synchronize with Main

Before starting a new task, update your assigned branch with the latest changes from `main`.

```bash
git fetch origin
git merge origin/main
```

Resolve any merge conflicts before continuing.

## 4. Make Your Changes

Add or modify the files required for your assigned task.

Follow the existing project structure and coding conventions. Avoid modifying unrelated files.

## 5. Stage and Commit Changes

Check your changes:

```bash
git status
```

Stage your files:

```bash
git add .
```

Commit your changes with a meaningful message:

```bash
git commit -m "Add barcode generation"
```

## 6. Push Your Branch

Push your changes to your assigned branch:

```bash
git push origin your-branch
```

## 7. Create a Pull Request

Submit your changes for integration into `main`.

1. Open the project repository on GitHub.
2. Select **Compare & pull request**.
3. Set the base branch to `main`.
4. Select your assigned branch as the compare branch.
5. Add a meaningful title and description.
6. Create the Pull Request.

## 8. Review and Merge

The project lead will review and merge Pull Requests into `main`.

Team members should not directly push changes to `main`.

## 9. Start Your Next Task

After your Pull Request has been merged, synchronize your assigned branch with the latest `main` before starting another task.

Repeat the synchronization commands from Section 3, then continue with the same workflow.

## General Guidelines

- Always work on your assigned branch.
- Keep commits meaningful and focused.
- Follow the existing C++ coding conventions.
- Test your changes before submitting a Pull Request.
- Keep Pull Requests limited to the assigned task.
- Coordinate with the project lead when necessary.

Thank you for contributing to the project.
