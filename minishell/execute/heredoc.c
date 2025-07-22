#include "execute.h"



// typedef enum e_error
// {
// 	SUCCESS = 0,
// 	ERR_GENERAL = 1,             // General error (e.g., cd: too many args)
// 	ERR_SYNTAX = 2,              // Syntax error (like unexpected token)
// 	ERR_CMD_PERMISSION = 126,    // Command found but permission denied
// 	ERR_CMD_NOT_FOUND = 127,     // Command not found
// 	ERR_SIGINT = 130,            // Ctrl-C (SIGINT)
// 	ERR_SIGQUIT = 131,           // Ctrl-\ (SIGQUIT)
// 	ERR_MALLOC = 255             // Internal error (custom)
// } t_error;
/*
 * handle_heredocs
 *   Iterate all commands’ redirs[], for each REDIR_HEREDOC:
 *     • create pipe
 *     • read lines until delimiter
 *     • write to pipe[1], close write end
 *     • store pipe[0] in a new cmd->heredoc_fd field
 */ 
int found_heredoc(t_command *command, t_data *data, char *limiter)
{
	int fd[2];
	pid_t pid;
	int status;
	char *input;

	if (pipe(fd) == -1)
		return (ERR_GENERAL);

	pid = fork();
	if (pid == -1)
		return (ERR_GENERAL);

	if (pid == 0) // child: write heredoc content
	{

		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_IGN);
		close(fd[0]);

		while (1)
		{
			input = readline("> ");
			if (!input)
				break;

			if (ft_strcmp(input, limiter) == 0)
			{
				free(input);
				break;
			}

			write(fd[1], input, ft_strlen(input));
			write(fd[1], "\n", 1);
			free(input);
		}
		close(fd[1]);
		exit(0);
	}
	else
	{

	// parent
		waitpid(pid, &status, 0);
		close(fd[1]);// why if i clsoed the shell stops and if not the shell keep going but with empty line as cat
		if (WIFSIGNALED(status))
		{
			close(fd[0]);
			data->last_status = 128 + WTERMSIG(status);
			return (data->last_status);
		}
}

	command->pipe_in = fd[0];
	return (SUCCESS);
}

int handle_heredocs(t_data *data)
{
    int i;
    int j;

    if (!data || data->cmd_count == 0)
        return (SUCCESS);

    i = 0;
    while (i < data->cmd_count)
    {
        j = 0;
        while (j < data->commands[i].redir_count)
        {
            if (data->commands[i].redirs[j].type == REDIR_HEREDOC)
            {
                data->last_status = found_heredoc(
                    &data->commands[i], data, data->commands[i].redirs[j].file);
                if (data->last_status != SUCCESS)
                    return (data->last_status);
            }
            j++;
        }
        i++;
    }
    return (SUCCESS);
}
