
b main
# "commands 1" tells GDB to attach a list of instructions to a specific breakpoint — in this case, breakpoint number 1
commands 1
    echo \n--- GDB LOG --- main function, enter c to continue:\n\n
end


# # breakpoint right before error
# b string_util_full_example.c:151
# commands 2
#     echo \n--- GDB LOG --- string_util_full_example, truncation test:\n\n
#     b string_util.c:_append_inline_truncation_message
# 
#     # IMMEDIATELY define commands for breakpoint created after previous breakpoint ($bpnum)
#     commands $bpnum
# 
#         echo \n--- GDB LOG --- Variable state(s) right before error:\n\n
#         info args
#         info locals
# 
#         # print all elements in array pointer
#         #  *example_local_fn_var @ ((size_t)malloc_usable_size(example_local_fn_var) / sizeof
# 
#     end
# 
#     c
# end

# Run the program
# # Run the program with args if your program needs them
# set args arg1 arg2
echo \n--- GDB LOG --- Running Program:\n\n
r

# 1. Define a hook that only prints on abort signal SIGABRT
define hook-stop
    # Only run if the stop reason is a signal (SIGABRT is signal 6)
    if $_thread && $_siginfo.si_signo == 6
        echo \n--- GDB LOG --- Program Aborted:\n
        # echo --- GDB LOG ---     [Note] ignore this specific error: "LeakSanitizer does not work under ptrace (strace, gdb, etc)"\n
        echo --- GDB LOG ---     [Note] ignore error:\n
        echo ==XXXXX==HINT: LeakSanitizer does not work under ptrace (strace, gdb, etc)\n
        echo --- GDB LOG ---     it is a known ptrace conflict between GDB and LSan and is not a memory leak in your code. It means your code exitted successfully\n\n
        echo --- GDB LOG ---     quitting gdb.\n
        q
    end
end
