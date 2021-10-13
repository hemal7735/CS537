# CS537-Project-2b


This is a solution to the [original problem](https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/processes-shell):

### Overview:
1. The wisc shell (or `wish` in short), supports 2 modes. i.e. interactive mode and batch mode. The main functions routes the traffic based on the mode used.
2. The shell supports 4 custom commands described in the above problem link, with one additional command `loop`. `loop` command have 2 usages.

Usage#1 
```
loop 3 echo hello
```
> this will execute `echo hello` 3 times

Usage#2: using `$loop` param, which will be replaced by the counter value.
```
loop 3 echo hello $loop world
```
> this will execute `echo hello 1 world`, `echo hello 2 world` and `echo hello 3 world`.


`parseAndExec` function takes care of routing the given command to it's actual handler.

3. Any other commands provided to `wish` are being treated as `native command` and such commands are handed by native command handler.

4. Native commands are searched using provided `path`, and if found, are executed using `execv` in its child process using `fork`.

