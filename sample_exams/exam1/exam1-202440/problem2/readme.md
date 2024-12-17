# Problem 2

This problem relates to the file `problem2.c`. You are to implement each step
in its corresponding function. For example, to implement the `infantry` step,
edit the `void infnatry()` function in `problem2.c`.

Note that in this problem, the `main` function is located in the `commander.c`
file. You should not need to edit this file at all; it is separated to hide
some unnecessary details from you, but feel free to peek a look if you have
time.

`commander.c` implements a simple shell program that you can issue commands in.
Your job is to implement the specific commands in their appropriate functions
in `problem2.c`, **you do not have to call these functions** from anywhere,
they are already set up for you.

## Preliminaries

Before you get started, if you had not done so yet, please install the required
dependencies as follows:

```sh
sudo apt update
sudo apt install libreadline8 libreadline-dev
```

### `helpers.c`

You have access to the same helper functions from problem 1 (i.e., `pr_log`,
`pr_warn` and `pr_error`) if you need them. Please refer to problem 1 for
details on how to use them.

### Implementation

You can implement the functions in this problem **in any order you prefer**.
They can be implemented completely independently, however, fully testing out
some of them can depend on having others working.

To help you out, I suggest you approaches the problems in the following order,
but that is not necessary in any way. I will list out the instructions in the
order suggested below.

1. `infantry`.
2. `list`.
3. `fire`.
4. `dezombify`.
5. `cavalry`.
6. `armored`.
7. `support`.
8. `nuke`.

### Overview

The shell implemented in this problem represents a commander that can order
troops to perform certain actions. There are three types of soldiers:

  1. _Infantry_ are foot soldiers that have no armor, i.e., they can be easy
     targets for the enemies.
  2. _Cavalry_ have a medium armor and can take up to three hits before
     falling action.
  3. _Armored_ are troops with heavy armor that cannot be destroyed unless air
     support is brought in.

The commander has access to the following actions:

  1. `list` to view the current set of troops (including itself), including
     their status on the battlefield.
  2. `fire` to fire a single shot at a soldier (we'll use the same commander to
     simulate both the commander and its enemy)
  3. `support` to call in air support on a particular unit.
  4. `dezombify` to send in special teams to clean out zombies.
  5. `nuke` to take the last option and destroy all troops, including the
     commander itself.

### Shell helpers

The `commander` shell is equipped with a few helper options:

1. You can use the `tab` key to autocomplete commands (one of the 8 commands
   above). Simply type the first few letters and then press `tab` and the shell
   will autocomplete that command for you.

2. The shell remembers each session's history. You use the `up` arrow key to
   scroll through previously issued commands. This can be very helpful when
   testing a combination of the commands.

## 1. Infantry

In this step, we would like to implement the `infantry` command in the `void
infantry()` function under `problem2.c`.

The `infantry` command will _spawn_ a new process that will _run in the
background_. That new process **should only** run an infinite loop after
printing `Infantry <pid> spawned, will be stationed and ready!` where `<pid>`
is the process ID of the newly spawned process. Writing an infinite loop is as
simple as writing `while(1);`. In other words, the `infantry` process will run
in the background forever.

Here are the requirements:

- The `infantry` unit should run an infinite loop in a separate process.

- The `infantry` unit should run in the background of the `commander` shell.

- The `commander` shell should be free to take on other commands after creating
  an infantry unit.

- The `infantry` unit should print `Infantry <pid> spawned, will be stationed
  and ready!` before going into the infinite loop, where `<pid>` is the process
  ID of the infantry unit.

### Sample run

Here is a sample run of the infantry command on my machine:

```sh
$ ./commander.bin
commander >> infantry
commander >> Infantry 4135461 spawned, will be stationed and ready!

```

Note that it is okay that the printed out messages are on top of each other,
that is a downside of multi-processing that we cannot enforce an ordering on
the printing order. Your printout order will look different. As long as the new
process is running in the background and the commander shell can accept new
commands, you are good!

### Verification

To verify how many processes are running, you can use the
`ps afo pid,command` command in a separate shell as follows:

```sh
$ ps afo pid,command
  PID COMMAND
29983 -zsh
 7646  \_ ./commander.bin
 7647      \_ ./commander.bin
 7648      \_ ps afo pid,command
```

As you can see, we have two processes called `commander.bin`, one is the main
shell and another is the infantry unit running in the background. You can
ignore the other processes that show up there, those are the other ones running
on your system. As long as you can see the `commander.bin` processes, you
should be good.

### Cleanup

In case you run into any issues and you have leftover processes running in the
background, you can use the `pkill commander.bin` shell command (from the Linux
shell) to kill all processes with the name `commander.bin`.

## 2. List

Next, we will add an option to run the `ps afo pid,commad` from the
commander shell, without the need to use a separate Linux shell.

Implement the `list` command in the `void list()` function in `problem2.c`.
This command will spawn a process that will execute the `ps afo pid,command`
command without leaving the commander shell. This command will run in the
**foreground**, i.e., you should not be able to run any new commander shell
commands until `ps afo pid,command` completes.

Here are the requirements:

- `list` should spawn a new, separate, process to run the `ps afo pid,command`
  command **in the foreground**.

- The commander shell should not prompt for any new commands until `ps afo
  pid,command` is done.

- Your code should not generate any orphaned or zombie processes related to the
  `ps afo pid,command` command.

### Sample run

Here is a sample run of the `list` command on my machine before and after
issuing an `infantry` command.

```sh
$ ./commander.bin
commander >> list
  PID COMMAND
29983 -zsh
 7868  \_ ./commander.bin
 7869      \_ ps afo pid,command
commander >> infantry
commander >> Infantry 7870 spawned, will be stationed and ready!
list
  PID COMMAND
29983 -zsh
 7868  \_ ./commander.bin
 7870      \_ ./commander.bin
 7871      \_ ps afo pid,command
commander >>
```

As you can see, in the first `list` command, only the process running the main
commander shell showed up in the output of `list`; that is the _parent_ process
running the shell.

After issuing an `infantry` command and then issuing `list`, you can see we
have two processes now. Both are named `commander.bin`. The main shell is still
there (PID of 7868) while the `infantry` unit also shows up (with PID
7870).

### Cleanup

At this stage, remember to always cleanup after yourself before you change and
recompile the code using `pkill commander.bin`. Check that you have no more
`commander.bin` processes running using `pgrep -a commander.bin` or `ps -a`
from a Linux shell.

## 3. Fire

Next we will implement the `fire` command to allow us to destroy any `infantry`
units out there.

Implement the `fire` command in the `void fire(int pid)` function in
`problem2.c`. The `fire` command accepts a single argument, an integer
representing the PID of the process to fire at. `fire` will attempt to destroy
that process by sending it the **Interrupt** signal. To send a process a
signal, check out the `kill` system call by reading its documentation using
`man 2 kill` in a Linux shell.

Here are the requirements:

- The `fire` command is performed by the commander shell itself, no need to
  spawn new processes.

- `fire` will send the interrupt signal to the process designate by the PID
  argument passed to it.

- In the case of an infantry unit, the infantry unit should silently die after
  receiving the interrupt signal.

- It is okay for now if your code generates zombie processes. We will take care
  of that in the following step.

### Sample run

Here is a sample run on my machine of issuing the `fire` command at an
`infantry` unit.

```sh
$ ./commander.bin
commander >> infantry
commander >> Infantry 7953 spawned, will be stationed and ready!
infantry
commander >> Infantry 7954 spawned, will be stationed and ready!
list
  PID COMMAND
29983 -zsh
 7952  \_ ./commander.bin
 7953      \_ ./commander.bin
 7954      \_ ./commander.bin
 7955      \_ ps afo pid,command
commander >> fire 7953
You shot 7953
commander >> list
  PID COMMAND
29983 -zsh
 7952  \_ ./commander.bin
 7953      \_ [commander.bin] <defunct>
 7954      \_ ./commander.bin
 7956      \_ ps afo pid,command
commander >> fire 7954
You shot 7954
commander >> list
  PID COMMAND
29983 -zsh
 7952  \_ ./commander.bin
 7953      \_ [commander.bin] <defunct>
 7954      \_ [commander.bin] <defunct>
 7957      \_ ps afo pid,command
commander >>
```

Note that after spawning two infantry units, the `list` command shows three
processes: the main shell and the two infantry units running in the background.
After shooting the first one using `fire 7953`, we can see the corresponding
infantry process with PID 4148850 becomes a zombie (shown using the `defunct`
state in the `list` command).

Then, after firing at the second infantry unit, you can see that both infantry
units become zombies and show the `defunct` status.

### No `list`, no problem

If you were not able to implement the `list` command successfully, no problem.
You can always manually issue the `pgrep -a commander.bin` or the `ps afo
pid,command` Linux commands from a separate Linux shell. You should see the
same output except on two different shells.

## 4. Dezombify

As you have seen, the `fire` command generates zombies; that is not ideal and
we would like to send in the "dezombification" teams to cleanup. Implement the
`dezombify` command in the `void dezombify(int pid)` function in `problem2.c`.

The `dezombify` command accepts a single argument, which is the process ID of
the process to dezombify. It should then go in and cleanup the zombie process
and remove it from the system so that it doesn't show up in `list` anymore.

> **NOTE** that the `dezombify` command has a dangerous side-effect. If you
  send in a dezombification team to cleanup a process that is still alive, your
  entire shell will become unusable and become stuck. That is okay, we are not
  to solve this problem in this exam.

Here are the requirements:

- It is the job of the main commander shell to `dezombify` a process.

- A successful dezombification should remove the target zombie process from the
  list of available processes.

- It is okay if your shell hangs and get stuck when trying to dezombify a unit
  that is still alive. **You do not have to test this**.

### Sample run

Here is a sample run showing the dezombify command in practice.

```sh
$ ./commander.bin
commander >> infantry
Infantry 8092 spawned, will be stationed and ready!
commander >> list
  PID COMMAND
29983 -zsh
 8091  \_ ./commander.bin
 8092      \_ ./commander.bin
 8093      \_ ps afo pid,command
commander >> fire 8092
You shot 8092
commander >> list
  PID COMMAND
29983 -zsh
 8091  \_ ./commander.bin
 8092      \_ [commander.bin] <defunct>
 8094      \_ ps afo pid,command
commander >> dezombify 8092
Sending dezombification team to target 8092
Unit 8092 dezombified successfully...
commander >> list
  PID COMMAND
29983 -zsh
 8091  \_ ./commander.bin
 8095      \_ ps afo pid,command
commander >>
```

As you can see, after sending in the first `infantry` unit, the `list` command
shows two processes. We then `fire` at that infantry unit, causing it to become
a zombie and thus show the `defunct` status in the `list` command's output.

After dezombifying that infantry unit, it will be remove from the output of the
`list` command and we are back to only seeing the main shell process show up in
there.

> Again, if `list` is not working, you can use `pgrep -a commander.bin` to
  check on your processes from a separate Linux shell.

## 5. Cavalry

As you might have noticed from the previous runs, an infantry unit directly
dies after being shot at. In this step, we will introduce `cavalry` units that
have a medium armor on them. This means that **it takes three shots** for a
cavalry unit to die, anything less would keep that cavalry unit alive.
Implement the `cavalry` command in the `void cavalry()` function in
`problem2.c`.

Similar to the `infantry` unit, the `cavalry` unit will run in a separate
process **in the background** and will run an infinite loop after printing
`Cavalry unit <pid> spawned, will be stationed and ready`, where `pid` is the
process id of that cavalry unit.

Here are the requirements:

- The `cavalry` unit should run an infinite loop in a separate process.

- The `cavalry` unit should run in the background of the `commander` shell.

- The `commander` shell should be free to take on other commands after creating
  a cavalry unit.

- The `cavalry` unit should print `Cavalry <pid> spawned, will be stationed
  and ready!` before going into the infinite loop, where `<pid>` is the process
  ID of the cavalry unit.

- The `cavalry` unit should only die after being shot at (using the `fire`
  command) **at least** three times.

- You should be able to dezombify a dead cavalry unit in the same way you do
  for an `infantry` unit.

### Sample run

Here is a sample run of that command on my machine:

```sh
$ ./commander.bin
commander >> cavalry
Cavalry unit 8577 spawned, will be stationed and ready!
commander >> list
  PID COMMAND
29983 -zsh
 8576  \_ ./commander.bin
 8577      \_ ./commander.bin
 8578      \_ ps afo pid,command
commander >> infantry
Infantry 8579 spawned, will be stationed and ready!
commander >> list
  PID COMMAND
29983 -zsh
 8576  \_ ./commander.bin
 8577      \_ ./commander.bin
 8579      \_ ./commander.bin
 8580      \_ ps afo pid,command
commander >> fire 8577
You shot 8577
commander >> Cavalry unit (8577) hit 1 time(s), armor still operational!
fire 8577
You shot 8577
Cavalry unit (8577) hit 2 time(s), armor still operational!
commander >> fire 8579
You shot 8579
commander >> list
  PID COMMAND
29983 -zsh
 8576  \_ ./commander.bin
 8577      \_ ./commander.bin
 8579      \_ [commander.bin] <defunct>
 8581      \_ ps afo pid,command
commander >> fire 8577
You shot 8577
Cavalry unit (8577)'s armor is broken, falling in action
commander >> list
  PID COMMAND
29983 -zsh
 8576  \_ ./commander.bin
 8577      \_ [commander.bin] <defunct>
 8579      \_ [commander.bin] <defunct>
 8582      \_ ps afo pid,command
commander >> dezombify 8577
Sending dezombification team to target 8577
Unit 8577 dezombified successfully...
commander >> dezombify 8579
Sending dezombification team to target 8579
Unit 8579 dezombified successfully...
commander >> list
  PID COMMAND
29983 -zsh
 8576  \_ ./commander.bin
 8583      \_ ps afo pid,command
commander >>
```

In this example, we first spawned a cavalry unit and an infantry unit. You can
see both are alive using the `list` command. After that we fire at the cavalry
unit once, you can see it printing a helpful message about its armor taking
damage. We then also fire at the cavalry unit a second time (generating a
second message) and we fire at the infantry unit once.

After that, issuing the `list` command will show that the cavalry unit is still
alive, while the infantry unit has terminated and became a zombie.

After the third shot at the cavalry unit, you can see that it prints that its
armor is broken and it will fall in action. This confirmed by the list command
showing both infantry and cavalry units as `defunct`.

Finally, you can see that we were able to dezombify both fallen units, putting
us back to only having the main shell in the output of the `list` command.

## 6. Armored

Our last version of unit is the `armored` unit. This unit **cannot** be
destroyed using the `fire` command, _no matter how many times you fire at it_.
Similar to the infantry and the cavalry units, the armored unit should run an
infinite loop in the background.

Implement the `armored` command in the `void armored()` function in
`problem2.c`.

Here are the requirements:

- The `armored` unit should run an infinite loop in a separate process.

- The `armored` unit should run in the background of the `commander` shell.

- The `commander` shell should be free to take on other commands after creating
  a armored unit.

- The `armored` unit should print `Armored <pid> spawned, will be stationed
  and ready!` before going into the infinite loop, where `<pid>` is the process
  ID of the armored unit.

- The `armored` unit should not be killed by the `fire` command, no matter how
  many times it is shot at.

### Sample run

Here is a sample run of the armored command on my machine:

```sh
$ ./commander.bin
commander >> armored
Armored unit 8702 spawned, will be stationed and ready!
commander >> list
  PID COMMAND
29983 -zsh
 8701  \_ ./commander.bin
 8702      \_ ./commander.bin
 8703      \_ ps afo pid,command
commander >> fire 8702
You shot 8702
commander >> fire 8702
You shot 8702
commander >> fire 8702
You shot 8702
commander >> fire 8702
You shot 8702
commander >> fire 8702
You shot 8702
commander >> fire 8702
You shot 8702
commander >> fire 8702
You shot 8702
commander >> fire 8702
You shot 8702
commander >> list
  PID COMMAND
29983 -zsh
 8701  \_ ./commander.bin
 8702      \_ ./commander.bin
 8704      \_ ps afo pid,command
commander >>
```

You can see that after spawning the `armored` unit, we have two processes in
the output of `list`. Even though we tried to `fire` at the `armored` unit 8
times, it is still alive and well, as evidenced by the output of the `list`
command at the end of the same run.

### Cleanup

Remember to always cleanup after yourself using `pkill commander.bin` between
sample runs to avoid having interference in the `list` command.

## 7. Support

Finally, the only way to kill an `armored` unit is to call in air support.
Implement the `support` command in the `void support(int pid)` function in
`problem2.c`.

`support` accepts a single argument, which is the process ID of the unit you
would like to call air support on. The `support` command should use a way (that
you can think of) to kill even `armored` units; in other words, we need to do
something that even `armored` units cannot _override or ignore_.

> **Note** that the `support` command also works on infantry and cavalry units,
  with the difference that the `support` command destroys `cavalry` units in one
  shot (rather than 3 in the case of the `fire` command).

Here are the requirements:

- The `support` command is performed by the commander shell itself, no need to
  spawn new processes.

- `support` should be able to destroy any unit (including `armored` units) in
  **a single shot**.

### Sample run

Here is a sample run of the `support` command on my machine showing its impact
on both the `cavalry` and the `armored` units.

```sh
$ ./commander.bin
commander >> cavalry
commander >> Cavalry unit 8822 spawned, will be stationed and ready!
armored
Armored unit 8823 spawned, will be stationed and ready!
commander >> list
  PID COMMAND
29983 -zsh
 8821  \_ ./commander.bin
 8822      \_ ./commander.bin
 8823      \_ ./commander.bin
 8824      \_ ps afo pid,command
commander >> fire 8823
You shot 8823
commander >> list
  PID COMMAND
29983 -zsh
 8821  \_ ./commander.bin
 8822      \_ ./commander.bin
 8823      \_ ./commander.bin
 8825      \_ ps afo pid,command
commander >> support 8823
You called air support on 8823
commander >> support 8822
You called air support on 8822
commander >> list
  PID COMMAND
29983 -zsh
 8821  \_ ./commander.bin
 8822      \_ [commander.bin] <defunct>
 8823      \_ [commander.bin] <defunct>
 8826      \_ ps afo pid,command
commander >> dezombify 8822
Sending dezombification team to target 8822
Unit 8822 dezombified successfully...
commander >> dezombify 8823
Sending dezombification team to target 8823
Unit 8823 dezombified successfully...
commander >> list
  PID COMMAND
29983 -zsh
 8821  \_ ./commander.bin
 8827      \_ ps afo pid,command
commander >>
```

In this run, we first spawn a cavalry and an armored units. We try to `fire` at
the armored unit to no avail, as shown in the output of the `list` command,
still showing all three processes.

After calling `support` on both units, you can see they turn into zombies, as
evidenced by the output of the `list` command.

Finally, we can call `dezombify` on both units to clean them up and go back to
only having the main shell in the output of the `list` command.

## 8. Nuke

> **NOTE** This problem is left for you to design. It can be completed in
  multiple ways and I will enforce one way or another. You can do it in one
  line or in 100. As long as you satisfy the requirements below, you are good!

The last command we would like to support is the `nuke` command. This is a last
ditch effort by the commander when they see that they have no more hope of
winning. Implement `nuke` in the `void nuke()` function in `problem2.c`.

The `nuke` command will cause the destruction of all processes related to the
main commander shell, **INCLUDING THE COMMANDER SHELL ITSELF**. In other words,
after issuing the `nuke` command, your commander shell will die and you will be
back into the Linux shell.

Here are the requirements:

- The requirements are simple, **destroy everything**.

### Sample run

```sh
$ ./commander.bin
commander >> armored
commander >> Armored unit 8981 spawned, will be stationed and ready!
infantry
Infantry 8982 spawned, will be stationed and ready!
commander >> cavalry
Cavalry unit 8983 spawned, will be stationed and ready!
commander >> armored
Armored unit 8984 spawned, will be stationed and ready!
commander >> list
  PID COMMAND
29983 -zsh
 8980  \_ ./commander.bin
 8981      \_ ./commander.bin
 8982      \_ ./commander.bin
 8983      \_ ./commander.bin
 8984      \_ ./commander.bin
 8985      \_ ps afo pid,command
commander >> nuke
...                                 // IT IS OKAY IF STUFF PRINTS OUT HERE
$ pgrep -a commander.bin            // THIS IS BACK TO THE LINUX SHELL
                                    // NO OUTPUT FROM pgrep
$ ps afo pid,command
  PID COMMAND
29983 -zsh
 9024  \_ ps afo pid,command
```

You can see here that no matter how many units we had spawned, issuing the
`nuke` command will destroy all of them, including the `commander` shell. It
puts us back into the Linux shell after that (it's okay if some stuff print out
before that, it depends on what and how you implement it).

From the Linux shell, issuing the `pgrep -a commander.bin` command shows no
`commander.bin` processes running.

> Again, there are multiple ways to implement this, ranging from a one-liner to
  complicated ways. Think about the possibilities and implement the one that
  suits you best.

