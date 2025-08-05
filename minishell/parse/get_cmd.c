/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_cmd.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zsid-ele <zsid-ele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 16:53:39 by zsid-ele          #+#    #+#             */
/*   Updated: 2025/07/30 16:53:39 by zsid-ele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

void	store_file_name(char *str, char **file_name, int i, t_vars *v)
{
	int	start;
	int	in_q;

	start = i;
	in_q = 0;
	while (str[i])
	{
		if (str[i] == '\'' || str[i] == '\"')
		{
			if (!in_q)
				in_q = str[i];
			else if (in_q == str[i])
				in_q = 0;
		}
		if ((str[i] == ' ' || str[i] == '\t') && !in_q)
			break ;
		i++;
	}
	v->i = i;
	*file_name = ft_substr(str, start, i - start);
}

void	fill_files(t_pipe *p, t_cmds *cmds, t_vars *v)
{
	v->start = v->exit_status - 1;
	if (p->cmds[v->counter][v->exit_status + 1] == '>'
		|| p->cmds[v->counter][v->exit_status + 1] == '<')
	{
		if (p->cmds[v->counter][v->exit_status + 1] == '>')
			cmds[v->counter].outs[v->exit_code].flag = APPEND;
		else if (p->cmds[v->counter][v->exit_status + 1] == '<')
			cmds[v->counter].outs[v->exit_code].flag = HERE_DOC;
		v->exit_status += 2;
	}
	else if (p->cmds[v->counter][v->exit_status] == '>')
	{
		cmds[v->counter].outs[v->exit_code].flag = OUT_FILE;
		v->exit_status++;
	}
	else if (p->cmds[v->counter][v->exit_status] == '<')
	{
		cmds[v->counter].outs[v->exit_code].flag = IN_FILE;
		v->exit_status++;
	}
}

void	save_utils(t_pipe *p, t_cmds *cmds, t_vars *v)
{
	while (p->cmds[v->counter][++v->exit_status])
	{
		if (p->cmds[v->counter][v->exit_status] == '"'
			|| p->cmds[v->counter][v->exit_status] == '\'')
		{
			if (v->quote_char == 0)
				v->quote_char = p->cmds[v->counter][v->exit_status];
			else if (v->quote_char == p->cmds[v->counter][v->exit_status])
				v->quote_char = 0;
		}
		if ((p->cmds[v->counter][v->exit_status] == '>'
			|| p->cmds[v->counter][v->exit_status] == '<')
			&& !v->quote_char)
		{
			fill_files(p, cmds, v);
			store_file_name(p->cmds[v->counter],
				&cmds[v->counter].outs[v->exit_code].file_name,
				v->exit_status + 1, v);
			clean_quotes(cmds[v->counter].outs[v->exit_code].file_name);
			remove_substr(p->cmds[v->counter], v->start, v->i);
			v->exit_status = v->start - 1;
			v->exit_code++;
		}
	}
}

void	call_save_utils(t_pipe *p, t_cmds *cmds, t_vars *v)
{
	v->exit_code = 0;
	v->exit_status = -1;
	save_utils(p, cmds, v);
}

void	files_saving(t_pipe *p, t_cmds **tmp)
{
	t_cmds	*cmds;
	t_vars	v;

	v.start = 0;
	v.quote_char = 0;
	v.helper_index = 0;
	v.counter = -1;
	v.exit_status = 0;
	*tmp = malloc(sizeof(t_cmds) * p->total_cmds);
	cmds = *tmp;
	cmds->cmd_len = p->total_cmds;
	cmds->red_len = 0;
	while (++v.counter < p->total_cmds)
	{
		cmds[v.counter].red_len = redirs_count(p->cmds[v.counter]);
		if (cmds[v.counter].red_len)
			cmds[v.counter].outs = malloc(sizeof(t_redirect)
					* cmds[v.counter].red_len);
		call_save_utils(p, cmds, &v);
		cmds[v.counter].cmd = ft_split(p->cmds[v.counter], ' ');
		v.helper_index = 0;
		while (cmds[v.counter].cmd[v.helper_index])
			clean_quotes(cmds[v.counter].cmd[v.helper_index++]);
		v.helper_index = 0;
	}
}
