# severin

A game engine.

## Execution
```
cmake .
make && make Shaders
bin/severin
```

### Windows
1. Open project in Visual Studio
2. Right click `CMakeLists.txt` in the project root directory
3. Click "Build"
4. Open `cmd` and `cd` to the project root directory
5. `cd shaders`
6. `glslangValidator -V {each shader file}`
  * Is there a way to do this with CMake?  Should I just make a batch script?
7. `cd ../bin`
8. `severin.exe`

## TODO
* transition to quaternions
  * currently, all entities use axis-angle rotational representation, except for the player and any items associated with it (i.e. weapons)
* make a MaterialID alias for uint16_t
* un-inherit entities from each other
* make EntityManager
* avoid copying Model objects (unique pointers?)
* collisions - can still pass through walls if going fast enough
  * ray test
* make player class?
* formalize the concept of weapons
  * each player can have 2
  * how to store?
  * how to denote behavior? (lambda, like in buffdog?)
  * how/when to run behavior?
* rotational physics
