#include "execute.h"

/*
 * exec_external
 *   Search PATH for cmd->args[0], check access(), and execve().
 *   On failure, print perror and set *last_status to 127.
 */

void ft_free_split(char **arr)
{
	int i = 0;
	if (!arr)
		return;
	while (arr[i])
		free(arr[i++]);
	free(arr);
}


static char *join_path_cmd(const char *dir, const char *cmd)
{
	char	*full;
	size_t	dir_len = ft_strlen(dir);
	size_t	cmd_len = ft_strlen(cmd);
	size_t	total_len = dir_len + cmd_len + 2;

	full = malloc(total_len);
	if (!full)
		return (NULL);

	ft_memcpy(full, dir, dir_len);
	full[dir_len] = '/';
	ft_memcpy(full + dir_len + 1, cmd, cmd_len);
	full[dir_len + 1 + cmd_len] = '\0';

	return (full);
}

/**
  - Find the full path of a command by searching PATH in envp
 */
char *find_command_path(const char *cmd, char **envp)
{
	char	*path_env = NULL;
	char	**paths = NULL;
	char	*full_path = NULL;
	int		i = 0;

	if (!cmd || !*cmd)
		return (NULL);

	// Absolute or relative path
	if (ft_strchr(cmd, '/'))
	{
		if (access(cmd, X_OK) == 0)
			return (ft_strdup(cmd));
		else
			return (NULL);
	}

	// Find PATH in envp
	while (envp[i])
	{
		if (ft_strncmp(envp[i], "PATH=", 5) == 0)
		{
			path_env = envp[i] + 5;
			break;
		}
		i++;
	}

	if (!path_env)
		return (NULL);

	paths = ft_split(path_env, ':');
	if (!paths)
		return (NULL);

	i = 0;
	while (paths[i])
	{
		full_path = join_path_cmd(paths[i], cmd);
		if (full_path && access(full_path, X_OK) == 0)
			break;
		free(full_path);
		full_path = NULL;
		i++;
	}

	// Clean up
	ft_free_split(paths);
	return (full_path);
}

void exec_external(char **args, char **envp, int *last_status, int piped)
{
	char	*path;
	pid_t	pid;
	int		status;

	pid = fork();
	if (pid < 0)
	{
		perror("fork");
		*last_status = 1;
		return;
	}
	else if (pid == 0) // 👶 child
	{
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);

		if (piped != -1)
		{
			if (dup2(piped, STDIN_FILENO) == -1)
			{
				perror("dup2");
				exit(ERR_GENERAL);
			}
			close(piped);
		}

		path = find_command_path(args[0], envp);
		if (!path)
		{
			write(2, args[0], ft_strlen(args[0]));
			write(2, ": command not found\n", 20);
			exit(127);
		}
		execve(path, args, envp);
		perror(args[0]);
		free(path);
		exit(127);
	}
	else // 👨 parent
	{
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);

		waitpid(pid, &status, 0);

		// ✅ Restore shell's signal handlers
		signal(SIGQUIT, SIG_IGN); // bash-like behavior

		if (WIFEXITED(status))
			*last_status = WEXITSTATUS(status);
		else if (WIFSIGNALED(status))
		{
			*last_status = 128 + WTERMSIG(status);
			if (WTERMSIG(status) == SIGINT)
				write(1, "\n", 1);
			else if (WTERMSIG(status) == SIGQUIT)
				write(1, "Quit (core dumped)\n", 19);
		}
	}
}
