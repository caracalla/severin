# severin

A game engine.

## Execution
```
cmake .
make && make Shaders
bin/severin
```

## TODO
* make a ModelID alias for uint16_t
* un-inherit entities from each other
* make EntityManager
* make the position for any entity its circumcenter (is that wise?)
* avoid copying Model objects (unique pointers?)
* collisions - can still pass through walls if going fast enough
  * ray test?
* make player class?
