# Build the documentation

## Build the C++ documentation

You need doxygen installed, e.g.:

```
sudo apt install doxygen
```

Then go to this directory and simply call:

```
doxygen dox.conf
```


## Build the Lua documentation

You need LDoc installed, and a few other Lua packages. This requires first installing lua and luarocks (Lua's package manager):

```
sudo apt install lua5.2 luarocks
```

Then the Lua packages we need:

```
sudo luarocks install ldoc
sudo luarocks install markdown
```

Then go to this directory and simply call:

```
ldoc .
```
