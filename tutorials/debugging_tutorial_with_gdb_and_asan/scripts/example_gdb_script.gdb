
# Set breakpoints
# breapoint at start of a function
b main
# breakpoint right before error
b c_error_examples.c:8

# Automate GDB cmds the line right before the error
# "commands 2" tells GDB to attach a list of instructions to a specific breakpoint—in this case, breakpoint number 2
commands 2
    echo \n--- GDB LOG --- Inspect state before error:\n\n
    info args
    info locals

    # print all elements in array pointer
    p *example_local_fn_var @ ((size_t)malloc_usable_size(example_local_fn_var) / sizeof(*example_local_fn_var))
end

# Run the program
# # Run the program with args if your program needs them
# set args arg1 arg2
echo \n--- GDB LOG --- Running Program:\n\n
r

# 1. Define a hook that only prints on abort signal SIGABRT
define hook-stop
    # Only run if the stop reason is a signal (SIGABRT is signal 6)
    if $_thread && $_siginfo.si_signo == 6
        echo \n--- GDB LOG --- Program Completed Successfully:\n
        # echo --- GDB LOG ---     [Note] ignore this specific error: "LeakSanitizer does not work under ptrace (strace, gdb, etc)"\n
        echo --- GDB LOG ---     [Note] ignore error:\n
        echo ==XXXXX==HINT: LeakSanitizer does not work under ptrace (strace, gdb, etc)\n
        echo --- GDB LOG ---     it is a known ptrace conflict between GDB and LSan and is not a memory leak in your code\n\n
        echo --- GDB LOG ---     quitting gdb.\n
        q
    end
end
