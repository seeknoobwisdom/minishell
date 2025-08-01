#include "execute.h"
/*
 * - is_builtin
 *   Return 1 if `cmd` matches one of: echo, cd, pwd, export, unset, env, exit
 */

 #include <string.h>  // if allowed

 // Check if identifier is valid (basic rule)
static int	is_valid_identifier(const char *str)
{
	int i = 0;

	if (!str || !(ft_isalpha(str[0]) || str[0] == '_'))
		return (0);
	while (str[i] && str[i] != '=')
	{
		if (!(ft_isalnum(str[i]) || str[i] == '_'))
			return (0);
		i++;
	}
	return (1);
}

 int isbuilt_in(t_command data)
 {
	 char *cmp = (data.args && data.args[0]) ? data.args[0] : NULL;
	 char *built[8] = {"echo","cd","pwd","export","unset","env","exit", NULL};
	 int i = 0;
 
	 if (!cmp)
		 return 0;
 
	 while (built[i])
	 {
		 if (ft_strcmp(built[i], cmp) == 0)
			 return 1;
		 i++;
	 }
	 return 0;
 }
 

 int	env_len(char **env)
{
	int len = 0;
	while (env && env[len])
		len++;
	return (len);
}

 int	append_env(t_data *data, char *entry)
{
	int		len = env_len(data->env);
	char	**new_env = malloc(sizeof(char *) * (len + 2));
	int		i = 0;

	if (!new_env)
		return (0);

	while (i < len)
	{
		new_env[i] = ft_strdup(data->env[i]);
		if (!new_env[i])
			return (ft_free_arr(new_env), 0);
		i++;
	}

	new_env[i++] = ft_strdup(entry);
	if (!new_env[i - 1])
		return (ft_free_arr(new_env), 0);
	new_env[i] = NULL;

	ft_free_arr(data->env);
	data->env = new_env;
	return (1);
}


int	set_env_value(const char *key, const char *value, t_data *data)
{
	char	*new_entry;
	int		i;
	int		key_len;
	char	**env = data->env;

	if (!key || !value || !data)
		return (0);

	key_len = ft_strlen(key);
	new_entry = malloc(key_len + ft_strlen(value) + 2); // key + '=' + value + '\0'
	if (!new_entry)
		return (0);

	ft_strlcpy(new_entry, key, key_len + ft_strlen(value) + 2);
	new_entry[key_len] = '=';
	ft_strlcpy(new_entry + key_len + 1, value, ft_strlen(value));

	// Replace if exists
	for (i = 0; env && env[i]; i++)
	{
		if (ft_strncmp(env[i], key, key_len) == 0 && env[i][key_len] == '=')
		{
			free(env[i]);
			env[i] = new_entry;
			return (1);
		}
	}

	// Append if not found
	append_env(data, new_entry);
	free(new_entry); // append_env duplicates it
	return (1);
}

static int is_n_flag(const char *arg)
{
    if (arg[0] != '-')
        return 0;
    int j = 1;
    while (arg[j])
    {
        if (arg[j] != 'n')
            return 0;
        j++;
    }
    return 1;
}

void echo(t_command data, int *last_status)
{
    int i = 1;
    int newline = 1;

    if (!data.args || !data.args[0]) {
        write(1, "\n", 1);
        if (last_status)
            *last_status = SUCCESS;
        return;
    }

    // handle -n flags
    while (data.args[i] && is_n_flag(data.args[i]))
    {
        newline = 0;
        i++;
    }

    // print args
    while (data.args[i])
    {
        write(1, data.args[i], ft_strlen(data.args[i]));
        if (data.args[i + 1])
            write(1, " ", 1);
        i++;
    }

    if (newline)
        write(1, "\n", 1);

    if (last_status)
        *last_status = SUCCESS;
}

void pwd(t_data *data)
{
    char *path;

    if (!data)
        return;
    path = getcwd(NULL, 0);
    if (!path)
    {
        perror("getcwd");
        data->last_status = 1;
        return;
    }
    write(1, path, ft_strlen(path));
    write(1, "\n", 1);
    data->last_status = SUCCESS;
    free(path);
}

void    free_envp(char  **env)
{
    int i;

    i = 0;
    while (env[i])
    {
        free(env[i]);
        i++;
    }
    free(env);
    env = NULL;
}

char **remove_env_var(char **env, const char *name)
{
	int i = 0, j = 0, len = strlen(name);
	int count = 0;
	while (env[count])
		count++;

	char **new_env = malloc(sizeof(char *) * (count + 1));
	if (!new_env)
		return NULL;

	while (env[i])
	{
		if (!(strncmp(env[i], name, len) == 0 && env[i][len] == '='))
			new_env[j++] = strdup(env[i]);
		i++;
	}
	new_env[j] = NULL;
	return new_env;
}



void unset(t_command command, t_data *data)
{
	int i = 1;
	char **new_env;

	while (command.args[i])
	{
		if (is_valid_identifier(command.args[i])) // Optional
		{
			new_env = remove_env_var(data->env, command.args[i]);
			if (!new_env)
			{
				data->last_status = ERR_MALLOC;
				return;
			}
			ft_free_arr(data->env); // You must implement this
			data->env = new_env;
		}
		else
		{
			// Optional: print error like bash does
			fprintf(stderr, "unset: `%s': not a valid identifier\n", command.args[i]);
			data->last_status = ERR_GENERAL;
		}
		i++;
	}
	if (data->last_status != ERR_GENERAL)
		data->last_status = SUCCESS;
}

void    env(t_command command, t_data *data)
{
	int	i;

	i = 0;
	(void)command;
	while (data->env[i])
	{
		if (ft_strchr(data->env[i],(int)'=') == true)
		{
			printf("%s\n", data->env[i]);
		}
		i++;
	}
	data->last_status = SUCCESS;
}


// Helper to duplicate env with an extra entry
// Remove entry by key (key must be exact before '=')
void	remove_env_entry(t_data *data, const char *key)
{
	int		len, i, j, key_len;
	char	**new_env;

	if (!data || !data->env || !key)
		return;
	len = env_len(data->env);
	new_env = malloc(sizeof(char *) * (len + 1));
	if (!new_env)
		return;
	i = 0;
	j = 0;
	key_len = ft_strlen(key);

	while (data->env[i])
	{
		if (ft_strncmp(data->env[i], key, key_len) == 0
			&& data->env[i][key_len] == '=')
		{
			i++;
			continue;
		}
		new_env[j++] = ft_strdup(data->env[i]);
		i++;
	}
	new_env[j] = NULL;
	ft_free_arr(data->env);
	data->env = new_env;
}

// The main export built-in
void	export(t_command command, t_data *data)
{
	int	i = 1;

	// No arguments: print environment in export format
	if (!command.args[1])
	{
		for (int j = 0; data->env[j]; j++)
			printf("declare -x %s\n", data->env[j]);
		data->last_status = 0;
		return;
	}

	while (command.args[i])
	{
		if (!is_valid_identifier(command.args[i]))
		{
			write(2, "export: `", 9);
			write(2, command.args[i], ft_strlen(command.args[i]));
			write(2, "': not a valid identifier\n", 27);
			data->last_status = 1;
			i++;
			continue;
		}

		char *equal = ft_strchr(command.args[i], '=');
		if (equal)
		{
			char *key = ft_substr(command.args[i], 0, equal - command.args[i]);
			if (!key)
				return ((void)(data->last_status = 1));
			remove_env_entry(data, key);
			free(key);
		}
		if (!append_env(data, command.args[i]))
			return ((void)(data->last_status = 1));
		i++;
	}
	data->last_status = 0;
}

void cd(t_command cmd, t_data *data)
{
	char *oldpwd, *path, *tmp;

	if (!data)
		return;
	oldpwd = getcwd(NULL, 0);
	path = (cmd.args && cmd.args[1]) ? cmd.args[1] : NULL;

	if (!path)
		path = get_env_value("HOME", data->env);
	else if (ft_strcmp(path, "-") == 0)
	{
		tmp = get_env_value("OLDPWD", data->env);
		if (tmp)
		{
			write(1, tmp, ft_strlen(tmp));
			write(1, "\n", 1);
			path = tmp;
		}
	}

	if (!path || chdir(path) != 0)
	{
		write(2, "cd: ", 4);
		if (path)
			write(2, path, ft_strlen(path));
		else
			write(2, "path not found", 14);
		write(2, ": No such file or directory\n", 29);
		if (oldpwd)
			free(oldpwd);
		data->last_status = 1;
		return;
	}

	if (oldpwd)
		set_env_value("OLDPWD", oldpwd, data);
	free(oldpwd);

	tmp = getcwd(NULL, 0);
	if (tmp)
	{
		set_env_value("PWD", tmp, data);
		free(tmp);
	}

	data->last_status = 0;
}



void    exit_builtin(t_command command, t_data  *data)
{
    int digit;
    int i;

    digit = 10;
    i = 0;
    if (!data)
        exit(0);
    if (command.args && command.args[1])
    {
        while (command.args[1][i])
        {
            digit = ft_isdigit(command.args[1][i]);
            if (!digit)
                break;
            i++;
        }
        if (!digit)
        {
            free_envp(data->env);
            printf ("exit: %s: numeric argument required\n", command.args[1]);
            printf("exit\n");
            exit(255);
        }
        else if (command.args[2])
        {
            printf("exit: too many arguments\n");
            data->last_status = 1;
            return ;
        }
        else
        {
            free_envp(data->env);
            printf("exit\n");
            exit(ft_atoi(command.args[1]));
        }
    }
    else
    {
        free_envp(data->env);
        printf("exit\n");
        exit (0);
    }
}

void execute_builtin(t_command command, t_data *data)
{
    char *built[8] = {"echo","cd","pwd","export","unset","env","exit", NULL};
    int i = 0;
    if (!command.args || !command.args[0]) {
        data->last_status = 127;
        return;
    }
    while (built[i] && ft_strcmp(built[i], command.args[0]) != 0)
        i++;
    if (!built[i]) {
        // Print error message to stderr, matching bash
        write(2, command.args[0], ft_strlen(command.args[0]));
        write(2, ": command not found\n", 20);
        data->last_status = 127;
        return;
    }
    if (i == 0)
        echo(command, &data->last_status);
    else if (i == 1)
        cd(command, data);
    else if (i == 2)
        pwd(data);
    else if (i == 3)
        export(command, data);
    else if (i == 4)
        env(command, data);
    else if (i == 5)
        env(command, data);
    else if (i == 6)
        exit_builtin(command, data);
}
