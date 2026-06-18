## Generation of Files

To generate the component files, use the following command:

```sh
make
```

*This generates the `.c` and `.h` files for the various components declared in the `NMPS` variable of the `Makefile`, and the `.elf` files in the `samples` folder. The `.c` and `.h` files are automatically moved to the main directory of `csim`.*

---

## Generation of Test Files for `portd`

Go to the `samples` folder and execute:

```sh
cd samples
make
```

*This generates the test files for `portd`.*

---

## Testing with the csim Environment

> **Note:**  
> You need to add the declarations of the generated files for the component (e.g., `portd`):  
> Main Makefile *(line 7)*: `portd.c`  
> Main Makefile *(line 63)*: `portd.h`  
> csim-run.c *(line 41)*: `portd.h`  
> loader.c   *(line 38)*: `portd.h`  
> loader.c   *(line 47)*: `portd_component`

Switch to the current folder:

```sh
./csim-run atmega328p/samples/test_portd.elf -b atmega328p/samples/test_portd.yaml
```

## Resetting the `atmega328p` Folder

To clean up the generated files and restore the initial state of the folder, use:

```sh
make clean
```