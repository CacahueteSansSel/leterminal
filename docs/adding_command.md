# Adding a new command

*You can add a new command in the terminal by following these steps :*

Open the `local_commands.h` file, and next to the `LOCAL_COMMANDS` macro, write

```
DEFCMD("(command name)", (command function)) \
```

This should look like that if you want to add a `test` command in the `local_commands.h` : 

```
#define LOCAL_COMMANDS DEFCMD("test", command_test)

void command_test(SecuredStringList* args) {
    Terminal::Screen::writeLn("Hello, World !");
}
```

And *voilà !*, we have our little command up and running.