# mtinfo
`mtinfo` is meant as a replacement for `terminfo`'s outdated binary format with something more modern, like JSON

The plan is to keep compatibility with `terminfo` by converting `mtinfo` files into `terminfo`, but make `mtinfo` the primary source of terminal capabilities while making the files:

- Easier to read
- Easier to write / maintain
- Easier to parse without complex libraries
- Truly cross-platform *(there's like 2 libraries that parse terminfo and support windows)*

And will also enable things like:

- Custom capabilties outside of the standard *(looking at you `truecolor`)*
