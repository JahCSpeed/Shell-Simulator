-================ Shell Simulator ================-

Shell will print a '>' prompt and read input from the user. It is able to execute other programs, possibly with command-line options, whether they’re
specified with an absolute path or a relative path. If a command is given with no absolute/relative path (e.g., “ls”), and it’s not a built-in command
the shell should will look in /usr/bin and /bin and execute the first such program found.

Compile with: "make" or "gcc -g -Wall -Wvla -Werror  -fsanitize=address -std=c11 shell.c -o shell"
Execute with "./shell"

-===========================================================================================================================================================-
Built-in commands:

•bg <jobID>
Run a suspended job in the background.

•cd [path]
Change current directory to the given (absolute or relative) path.

•exit
Exit the shell.

•fg <jobID>
Run a suspended or background job in the foreground.

•jobs
List current jobs, including their job ID, process ID, current status, and command. 

•kill <jobID>
Send SIGTERM to the given job.
-===========================================================================================================================================================-