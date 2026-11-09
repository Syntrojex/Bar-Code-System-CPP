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

Available team branches:

- `ali`
- `farzam`
- `syntrojex`
- `shauzab`

## 3. Make Your Changes

Add or modify the files required for your assigned task.

Follow the existing project structure and coding conventions. Avoid modifying unrelated files.

## 4. Stage and Commit Changes

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

## 5. Push Your Branch

Push your changes to your assigned branch:

```bash
git push origin your-branch
```

## 6. Create a Pull Request

Submit your changes for integration into `main`.

1. Open the project repository on GitHub.
2. Select **Compare & pull request**.
3. Set the base branch to `main`.
4. Select your assigned branch as the compare branch.
5. Add a meaningful title and description.
6. Create the Pull Request.

## 7. Review and Merge

The project lead will review submitted Pull Requests.

- The project lead's approval is required before merging.
- Address any requested changes before approval.
- Team members should not directly push changes to `main`.
- Only reviewed and approved contributions should be merged.

## 8. Automatic Branch Synchronization

Whenever changes are merged into `main`, GitHub Actions automatically attempts to synchronize all team branches with the latest `main`.

- No manual fetching or merging is required after successful synchronization.
- Existing branch commits and contributions are preserved.
- If a merge conflict occurs, the affected branch will be skipped and must be resolved manually.
- Check the GitHub Actions workflow status if synchronization fails.

## General Guidelines

- Always work on your assigned branch.
- Keep commits meaningful and focused.
- Follow the existing C++ coding conventions.
- Test your changes before submitting a Pull Request.
- Keep Pull Requests limited to the assigned task.
- Coordinate with the project lead when necessary.
- Never commit directly to `main`.

Thank you for contributing to the project.
