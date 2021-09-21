# CS537-Project-1


This is a solution to the [original problem](https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/initial-kv):

### Overview:
1. The data structures used for storing db in memory is fixed size(large prime number) hashmap that uses chaining when hash-collision happens.
2. The program abstraction includes the following stages.
    1. Load
    2. Command execution
    3. Persist
DB is loaded from the file, once during the start of the program, and it is persisted in the file once the program ends. `database.txt` will be created during persistence stage if it did not exist earlier.
3. `main()` function is kept clean deliberately to understand the abstraction better.
4. There are functions for each command to keep things simple and easy to debug. Each command sanitize the input and verify if it is provided with only the information it needs, nothing more, nothing less.


### Future enhancements:
1. HashTable that can be dynamically resized
2. Lazy load of DB if possible. This can be done looking at the operations that we have to do. DB from disk and memory can be merged later.
3. Persist to disk only if there was a modification to data.
