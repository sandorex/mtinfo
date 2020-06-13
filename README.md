# mtinfo

> **WARNING** This is the `dev` branch
>
> This code may contain an evil AI that will destroy the world
>
> **PROCEED AT YOUR OWN RISK**

`mtinfo` is a modern cross-platform C++17 library to read `terminfo` and [`mtinfo`](spec/README.md) files

**This library will have official wrappers for most popular languages in the future**

### Differences compared to terminfo standard
There are couple differences when reading terminfo, while they are not that important i wanted to point them out

- Bools are missing if the value is `-1`, but are false if they are less than or equal to `0` but true if they are more than `0`

- Numbers are missing if their value is below `0`
