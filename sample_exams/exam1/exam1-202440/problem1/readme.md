# Problem 1

This problem relates to the file `problem1.c`. You are to implement each step
in its corresponding function. For example, implement step 1 in the `void
step1(const char *op, const char *message)` function, and so on.

In this problem, we would like to communicate with a separete program,
defined in `caesar.c`, that implemets a caesaer cipher supporting both
encryption and decryption.

## Preliminaries

Before you get started, if you had not done so yet, please install the required
dependencies as follows:

```sh
sudo apt update
sudo apt install libreadline8 libreadline-dev
```

### Understanding `caesar.c`

Let's first explore what `caesar.c` does. First, compile it using `make` and
you should see the `ceasar.bin` binary show.

`caesar.bin` operates in one of several modes:

1. `caesar.bin op message` where `op` is one of `encrypt` or `decrypt`, and
   `message` is a custom message you'd like to encrypt/decrypt. In this mode,
   `caesar.bin` will simply apply the operation on the presented message, print it
   out and exit.

   Here's an example of running it in this mode:
   ```sh
   $ ./caesar.bin encrypt "hello world"
   Running caesar cipher in encrypt mode on argument message!
   [./caesar.bin:4074145] Output of encrypt operation: pmttw ewztl
   ```

   and

   ```sh
   $ ./caesar.bin decrypt "pmttw ewztl"
   Running caesar cipher in decrypt mode on argument message!
   [./caesar.bin:4075968] Output of decrypt operation: hello world
   ```

2. `caesar.bin op readfd` where `op` is one of `encrypt` or `decrypt` and
   `readfd` is a file descriptor (an integer) that represents a reading end of
   a pipe to read from.

   For example, `./caesar.bin encrypt 3` will read the message to encrypt from
   file descriptor 3 and will then print out the message to the console.

   To test this out, you can ask `caesar.bin` to read from `stdin` (i.e., the
   standard input or the keyboard at the console) and then print out the
   encrypted message.

   ```sh
   Running caesar cipher in encrypt mode with readfd = 0!
   hello                    // I typed this message here
   [./caesar.bin:4078321] Output of encrypt operation: pmttw
                            // Extra blank lines here

   ```

3. `caesar.bin op readfd writefd` where `op` is one of `encrypt` or `decrypt`
   and `readfd` is a file descriptor that represents a reading end of a pipe
   and `writefd` is a file descriptor that represents a writing end of a pipe.

   For example, `./caesar.bin encrypt 3 4` will read the message to encrypt
   from file descriptor 3 and will write the encrypted message to file
   descriptor 4.

   To test this out, you can pass `0` as the read file descriptor to read from
   standard input and `1` as write file descriptor to write to standard output
   (the console in our case).

   ```sh
   $ ./caesar.bin encrypt 0 1
   Running ceasar cipher in encrypt mode with readfd = 0 and writefd = 1
   hello              // I typed this message here
   pmttw
   ```

### Helper functions

This exam comes with a set of helper functions that you can find in `helpers.h`
and `helpers.c`. You do not need to understand these functions but they can be
helpful to discernate printing message (assuming your terminal emulator allows
it).

In `helpers.h` you will find three helpful macros that you can use to replace
`printf`. They essentially are just wrappers around `printf` that you can use
to add colors to your printouts (again, assuming your terminal emulator
supports it - most do unless you are using Gitbash or something the like).

1. `pr_log`: This prints a log message. You can call this in the same way as
   you do `printf` (for example, `pr_log("%d: Hello from parent process\n",
   getpid());` and the message will show up in **green color**.

2. `pr_warn`: This prints a warning message. Calling this in the same way you
   do `printf` will print out in yellow color.

3. `pr_error`: This prints an error message. Calling this in the same way you
   do `printf` will print out in red color.

Feel free to use these as you see fit, or to completely skip them - they are
helpers but **are not necessary** to complete this exam. You can use plain old
`printf` and still do perfectly well on the exam.

### Running `problem1.bin`

First, if you have not done so, compile the code using `make` to generate the
needed binaries. You should find `problem1.bin` show up. Right now, it will not
do anything useful.

For the rest of this problem, you can run `problem1.bin` by providing **two
command line arguments**:
  1. The operation you'd like to do, that being `encrypt` or `decrypt`.
  2. The message you'd like to `encrypt` or `decrypt`.

For example, `./problem1.bin encrypt "hello world"` would attempt to use
`problem1` to _encrypt_ the string `"hello world"`.

> **NOTE**: Do not use special characters in your message (e.g., `!`). Those
  have special meanings in bash and must be escaped. Stick to plain letters and
  spaces for now, unless you are familiar with bash scripting.

## Step 1:

Implement this part in the `step1` function at the top of `problem1.c`.

In this step, we would like to fork a child that will execute `caesar.bin` with
the requested operation and the message we'd like to operate on. You should aim
to execute `ceasar.bin` in the first mode (i.e., by passing two arguments, the
operationg and the message itself).

Here are the requirements:

1. You code should fork a new child process to execute `caesar.bin`.

2. The operation that `caesar.bin` performs should be the same as the one
   passed to `problem1.bin`.

3. The message that `caesar.bin` operates on should be the same one passed to
   `problem1.bin`.

4. You code should not generate any zombies or orphaned processes.

### Sample run

Here's a sample run on my machine, with some custom print out messages (feel
free to use the same message as you see fit):

```sh
$ ./problem1.bin encrypt "hello world"
Parent running step 1
Running caesar cipher in encrypt mode on argument message!
[./caesar.bin:4091866] Output of encrypt operation: pmttw ewztl
Parent done after successfully executing ceaser cipher!



```

You should try it out in both `encrypt` and `decrypt` mode to make sure that
everything is working correctly.

> Please remember that your process ids will look completely different than
  mine. That is expected.

## Step 2:

Implement this part in the `step2` function at the top of `problem1.c`.

In this step, we would like to fork a child that will execute `caesar.bin` in
the second mode above, i.e., with an operation and a reading end of a pipe.
This means that the parent process should write the user's message (passed to
`problem1.bin`) into a pipe that `caesar.bin` will read from. It is the job of
`caesar.bin` to print out the message after reading it from the pipe.

Here are the requirements:

1. You code should fork a new child process to execute `caesar.bin`.

2. The operation that `caesar.bin` performs should be the same as the one
   passed to `problem1.bin`.

3. The message that `caesar.bin` operates on should be the same one passed to
   `problem1.bin`, passed through an appropriate means of inter-process
   communication.

4. You code should not generate any zombies or orphaned processes.

### Sample run

Here's a sample run on my machine, with some custom print out messages (feel
free to use the same message as you see fit):

```sh
$ ./problem1.bin encrypt "hello world"
Parent running step 2
Running caesar cipher in encrypt mode with readfd = 3!
[./caesar.bin:4096445] Output of encrypt operation: pmttw ewztl
Parent done after successfully executing ceaser cipher!
```

You should try it out in both `encrypt` and `decrypt` mode to make sure that
everything is working correctly.

> Please note that you might see a different file descriptor number for
  `readfd`; that is totally okay.

> Please remember that your process ids will look completely different than
  mine. That is expected.


## Step 3:

Implement this part in the `step3` function at the top of `problem1.c`.

In this step, we would like to fork a child that will execute `caesar.bin` in
the third mode above, i.e., with an operation, a reading end of a pipe, and a
writing end of a pipe. This means that the parent process should write the
user's message (passed to `problem1.bin`) into a pipe that `caesar.bin` will
read from. Then, `ceasar.bin` will write back the encrypted or decrypted
message into a pipe that the parent will read from. **It is the job of the
parent to print out the encrypted/decrypted message after obtaining it from the
pipe**.

Here are the requirements:

1. You code should fork a new child process to execute `caesar.bin`.

2. The operation that `caesar.bin` performs should be the same as the one
   passed to `problem1.bin`.

3. The message that `caesar.bin` operates on should be the same one passed to
   `problem1.bin`, passed through an appropriate means of inter-process
   communication.

4. The parent must print the encrypt/decrypted message, not `caesar.bin`.

5. You code should not generate any zombies or orphaned processes.

### Sample run

Here's a sample run on my machine, with some custom print out messages (feel
free to use the same message as you see fit):

```sh
$ ./problem1.bin encrypt "hello world"
Parent running step 3
Running ceasar cipher in encrypt mode with readfd = 3 and writefd = 6
Parent received message back from the caesar program: pmttw ewztl       // this is printed by parent
Parent done after successfully executing ceaser cipher!
```

You should try it out in both `encrypt` and `decrypt` mode to make sure that
everything is working correctly.

> Please note that you might see a different file descriptor number for
  `readfd` and `writefd`; that is totally okay.

> Please remember that your process ids will look completely different than
  mine. That is expected.

