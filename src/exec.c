#include "exec.h"

#include "builtin.h"
#include "history.h"
#include "parse.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*

    Modify to allow: 
        longer pipe chaining 
        && the execution of the next command only if the previous passes
        ;  the execution of the next command regardless if the previous passes or not
        << 
        >>  
        <   
        >   

*/

int
shsh_execute (char** args, History* h)
{
    if (!args || !args[0])
    {
        return 1;
    }

    if (strcmp(args[0], "exit") == 0)
    {
        return 0;
    }

    char* original = concatenate_strings(args); // combines char* args[] and appends spaces between indexes
    int flag;

    int pipe_index = -1;
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "|") == 0)
        {
            pipe_index = i;
            break;
        }
    }

    if (pipe_index >= 0)
    {
        execute_piped_commands(args, pipe_index, h);
        goto history;
    }

    int output_redirect_index = -1;
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], ">>") == 0)
        {
            output_redirect_index = i;
            break;
        }
    }

    if (output_redirect_index >= 0)
    {
        int eor_code = execute_output_redirection(args, output_redirect_index, h);
        goto history;
    }

    flag = execute_single_command(args, h);

    history: // TODO lazy fix
    history_append(original, h); // adds the concatenated "original" command to the history list

    return 1;
}

int
execute_single_command (char** args, History* h)
{
    pid_t cpid;
    int status = 1;
    int flag = get_internal_command(args); // currently checks to see if args is either "!!", "cd", "history"

    if (flag)
    {
        return exec_internal_command(args, flag, h);
    }

    flag = 1;
    cpid = fork();
    if (cpid == 0)
    {
        int t = execvp(args[0], args);
        if (t < 0)
        {
            perror("execvp");
            flag = 0;
            exit(EXIT_FAILURE);
        }

    }
    else if (cpid < 0)
    {
        perror("fork");
        flag = 0;
        exit(EXIT_FAILURE);
    }

    waitpid(cpid, &status, 0);

    return flag;
}

int
execute_piped_commands (char** args, int pipe_index, History* h)
{
    // TODO make more dynamic
    // only allows for command 1 | command 2
    // instead of "unlimited" chaining

    args[pipe_index] = NULL;
    char** command1 = args;
    char** command2 = args + pipe_index + 1;
    int t, flag;

    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return 0;
    }

    pid_t pid1, pid2;
    int status;

    pid1 = fork();
    if (pid1 == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        flag = get_internal_command(command1);
        if (flag)
        {
            exec_internal_command(command1, flag, h);
            exit(EXIT_SUCCESS);
        }

        t = execvp(command1[0], command1);
        if (t < 0)
        {
            exit(EXIT_FAILURE);
        }

    }
    else if (pid1 < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    pid2 = fork();
    if (pid2 == 0)
    {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        flag = get_internal_command(command2);
        if (flag)
        {
            exec_internal_command(command2, flag, h);
            exit(EXIT_SUCCESS);
        }

        t = execvp(command2[0], command2);
        if (t < 0)
        {
            exit(EXIT_FAILURE);
        }

    }
    else if (pid2 < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);

    return 1;
}

int
execute_output_redirection (char** args, int redirect_index, History* h)
{
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return 1;

    }

    else if (pid == 0)
    {
        int output_fd = open(args[redirect_index + 1], O_WRONLY | O_CREAT | O_APPEND, 0666);

        if (output_fd == -1)
        {
            perror("open");
            return 1;
        }

        if (dup2(output_fd, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            return 1;
        }

        if (close(output_fd) == -1)
        {
            perror("close");
            return 1;
        }

        args[redirect_index] = NULL;
        if (execvp(args[0], args) == -1)
        {
            perror("execvp");
            return 1;
        }

    }

    else
    {
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
            return 1;
        }

        if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
        else
        {
            // abnormal child termination
            return 1;
        }
    }

    return 0;
}

int
exec_internal_command (char** args, int command, History* h)
{
    switch (command)
    {
        case CD:
            return cd(args);
        case DOUBLE_BANG:
            return shsh_execute(double_bang(h), h);
        case HISTORY:
            display_command_history(h);
            return 1;
        default:
            return 0;
    }
}

void
display_command_history (History* h)
{
    for (int i = 0; i != h->index; i++)
    {
        printf("%s\n", h->list[i]);
    }
}
