# Minishell

A simple shell implementation project from 42 School that recreates basic functionality of bash.

## Description

Minishell is a project that challenges students to create their own Unix shell, implementing core shell features and understanding how command-line interfaces work under the hood. This project provides deep insights into process management, file descriptors, and system calls.

## Features

- **Command execution** with absolute and relative paths
- **Built-in commands**: `echo`, `cd`, `pwd`, `export`, `unset`, `env`, `exit`
- **Pipes** (`|`) for command chaining
- **Redirections**: 
  - Input (`<`) and output (`>`) redirection
  - Append mode (`>>`)
  - Here-doc (`<<`)
- **Environment variables** expansion (`$VAR`)
- **Exit status** handling (`$?`)
- **Signal handling** (Ctrl+C, Ctrl+D, Ctrl+\)
- **Quote handling** (single and double quotes)
- **Command history** navigation

## Technical Implementation

- Written in **C**
- Memory management with no leaks
- Proper error handling and edge cases
- Readline library integration for line editing
- Process creation and management using `fork()`, `execve()`, `wait()`

## Usage

```bash
make
./minishell
