# Operating Systems K22 - Project 4

## Contributors:

```
- Fotis Nikolidais: sdi1900132
- Fykas Athanasios: sdi2000282 
```
## Compiling/For clean
```
make
make clear
make clear_test_dir
```

## Running
```
./cmpcat -d dirA dirB
./cmpcat -d dirA dirB -s dirC
```

## Notes

### The asssignment is fully implemented

```
Ακολουθήσαμε τις οδηγίες της εκφώνησης για την υλοποίηση του project.

    - Κάνουμε 2 DFS για να βρούμε τα διαφορετικά στοιχεία του κάθε directory.
    - Όταν κάνουμε merge βάζουμε τα κοινά στοιχεία των δύο directory καθώς γίνεται το πρώτο DFS.
    - Τα links όταν δημιουργούνται θα πρέπει να δείχνουν στα αρχεία με το path από το home directory.
    - H traverse differences κάνει DFS ξεκινώντας από directory που υπάρχει μονο σε ένα από τα δύο file system trees.
    - Η print_differences_and_merge_rec κάνει DFS σε directories που υπάρχουν και στις δύο ιεραρχίες, μέχρι
    να βρει κάποιο που δεν είναι ίδιο καλώντας τότε την traverse differences
    - Η print_differences_and_merge κάνει αρχικοποίηση και καλεί την αναδρομική εκδοχή της δύο φορες, για να κάνει 2 φορές DFS

Τέλος χρησιμοποιήσαμε το valgrind για τυχόν memory leaks.
```