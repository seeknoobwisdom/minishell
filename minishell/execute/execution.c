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
 * apply_redirections
 *   - Loop over cmd->redirs[], open FDs, dup2() to STDIN/STDOUT, close originals.
 */
// void apply_redirections(t_command *cmd)
// {
//     // TODO: for each cmd->redirs[i]:
//     //   if REDIR_IN  → open(cmd->redirs[i].file, O_RDONLY)
//     // ...
// }

// /*
//  * apply_pipes
//  *   - Given index i and pipes[][] from the parent,
//  *     dup2 read end to STDIN and/or write end to STDOUT.
//  */
// void apply_pipes(int i, int pipes[][2], int cmd_count)
// {
//     // TODO
// }

// /*
//  * close_all_pipes
//  *   - Parent/child should close all pipe FDs after dup2.
//  */
// void close_all_pipes(int pipes[][2], int cmd_count)
// {
//     // TODO
// }

void    execute_piped_external(t_command    command, char *path,t_data    *data)
{

		if (!path)
		{
			write(2, command.args[0], ft_strlen(command.args[0]));
			write(2, ": command not found\n", 20);
			exit(127);
		}
		execve(path, command.args, data->env);
		perror(command.args[0]);
		exit(127);
}

/*
 * launch_pipeline
 *   - Fork each command, call apply_redirections & apply_pipes in child,
 *     then either run builtin or exec_external
 *      Zayed : it will return the last status.
 */
int launch_pipeline(t_data *data)
{
    int i;
    int past_fd;// dont forget if there is pipe had already been there to ignore this past pipe
    int fd[2];
    pid_t   *pid =malloc(sizeof(pid_t) * data->cmd_count);
    char    *path;
    int status;

    i = 0;
    while (i < data->cmd_count)
    {
        pipe (fd);
        if (isbuilt_in(data->commands[0]) == false)
            path = find_command_path(data->commands[i].args[0], data->env);// so i can free it out of from the parent
        else
            path = NULL;
        
        pid[i] = fork();
        if (pid == 0)
        {
        if (past_fd != -1 || data->commands[i].pipe_in != -1)
        {
            if (data->commands[i].pipe_in == -1)
            {
                dup2(STDIN_FILENO, past_fd);
            }
            else
            {
                dup2(STDIN_FILENO, data->commands[i].pipe_in); // we will ignore the pipe input and will use the heredoc input
            }
            if (past_fd != -1)
                close(past_fd);
            if (data->commands[i].pipe_in != -1)
                close(data->commands[i].pipe_in != -1);
            data->commands[i].pipe_in = -1;
            past_fd = -1;
        }

        if (i + 1 < data->cmd_count) // there is still a command after this commamd
        {
            dup2(STDOUT_FILENO, fd[1]);
            close(fd[1]);
        }
        if (isbuilt_in(data->commands[i]) == true)
            execute_builtin(data->commands[i], data);
        else
            execute_piped_external(data->commands[i],path, data);// dup child calling each other 
        }
        else
        {
            if (fd[1] != -1)
                close(fd[1]);
            past_fd = fd[0];
        }
        if (path)
            free(path);
    }
    i = 0;
    while (i < data->cmd_count)
    {
        waitpid(pid[i], &status, 0);
        data->last_status = WIFEXITED(status);
        if (WIFSIGNALED(status))
            data->last_status = 128 + WIFSIGNALED(status);
    }
    close(fd[0]);
    return (true);
}

void    pipe_init(t_data *data)
{
    int i;

    i = 0;
    while (i < data->cmd_count)
    {
        data->commands[0].pipe_in = -1;
        data->commands[0].pipe_out = -1;
        i++;
    }
}

void execute_commands(t_data *data)
{

    if (!data || data->cmd_count <= 0)
        return;

    pipe_init(data);
    // Handle heredocs first (must be done before any forking)
    if (handle_heredocs(data) != 0)
    {
        data->last_status = ERR_GENERAL; // Set error status
        return;
    }

    // Single command case
    if (data->cmd_count == 1)
    {
        // Built-in commands execute in parent process
        if (isbuilt_in(data->commands[0]) == true)
            execute_builtin(data->commands[0], data);
        // External commands fork and execute
        else
            exec_external(data->commands[0].args, data->env, &data->last_status, data->commands[0].pipe_in);

    }
    // // Pipeline case (multiple commands)
    else
        launch_pipeline(data);

    // // Cleanup any temporary files (like heredocs)
    // cleanup_redirections(data);
}