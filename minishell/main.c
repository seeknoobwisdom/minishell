/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nakhalil <nakhalil@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/27 14:53:55 by nakhalil          #+#    #+#             */
/*   Updated: 2025/05/21 20:35:05 by nakhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// static void debug_print_tokens(t_data *data)
// {
//     printf("-----------------------------------------------------------------\n");
//     printf("→ [debug] token_count = %d\n", data->token_count);
//     for (int i = 0; i < data->token_count; i++)
//     {
//         t_token *tok = &data->tokens[i];
//         printf("   token[%d]: type=%d, quote=%d, value=\"%s\"\n",
//                i, tok->type, tok->quote, tok->value ? tok->value : "NULL");
//     }
// }

// static void debug_print_commands(t_data *data)
// {
//     printf("→ [debug] cmd_count   = %d\n", data->cmd_count);
//     for (int c = 0; c < data->cmd_count; c++)
//     {
//         t_command *cmd = &data->commands[c];
//         printf("   command[%d]:\n", c);

//         if (cmd->args)
//         {
//             for (int j = 0; cmd->args[j]; j++)
//                 printf("      arg[%d] = \"%s\"\n", j, cmd->args[j]);
//         }
//         else
//             printf("      (no args)\n");

//         for (int r = 0; r < cmd->redir_count; r++)
//         {
//             printf("      redir[%d]: type=%d, file=\"%s\"\n",
//                    r, cmd->redirs[r].type, cmd->redirs[r].file);
//         }
//     }
//     printf("-----------------------------------------------------------------\n");
// }

static void	cleanup_and_exit(t_data *data, int exit_code)
{
	free_data(data);
	clear_history();
	exit(exit_code);
}

char	**copy_envp(char **copy)
{
	int	i;
	char	**new;

	i = 0;
	while (copy[i])
		i++;
	new = malloc(sizeof(char *) * (i + 1));
	new[i] = NULL;
	if (!new)
		return (NULL);
	i = 0;
	while(copy[i])
	{
		new[i] = ft_strdup(copy[i]);
		i++;
	}
	new[i] = NULL;
	return(new);
}

int	main(int argc, char **argv, char **envp)
{
	t_data	data;
	char	*input;
	t_error	err;

	(void)argc;
	(void)argv;
	ft_bzero(&data, sizeof(data));
	//data.env = envp; we need to copy (store) the envp in allocated memory not the original one becouse i had problemes dealing with it zayed
	data.env = copy_envp(envp);
	setup_signals();

while (1)
{
	data.syntax_error = 0;

	// ⛔ Skip readline if signal (e.g., Ctrl+C in heredoc)
	if (g_signal)
	{
		data.last_status = g_signal;
		g_signal = 0;
		continue;  // ✅ Skip prompt for this loop
	}

	input = readline("minishell> ");
	if (!input)
	{
		write(1, "exit\n", 5);
		break;
	}

	if (*input)
	{
		add_history(input);

		// Step 1: Tokenize
		err = tokenize_input(input, &data);
		if (err != SUCCESS)
			handle_error(err, &data, "tokenization");

		// Step 2: Syntax validation
		else if ((err = validate_syntax(&data)) != SUCCESS)
			handle_error(err, &data, NULL);

		// Step 3: Expand variables
		else if ((err = expand_tokens(&data)) != SUCCESS)
			handle_error(err, &data, "expand");

		// Step 4: Parse & execute
		else if ((err = parse_tokens(&data)) != SUCCESS)
			handle_error(err, &data, "parse");

		else
			execute_commands(&data);
	}

	free(input);
	free_data(&data);
}


	cleanup_and_exit(&data, data.last_status);
}
