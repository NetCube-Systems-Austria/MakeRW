# MakeRW

MakeRW is a utility tool designed to make a directory writable by copying it to a writable location and binding it to the original location. This can be particularly useful in scenarios where you need to modify files in a read-only directory.

## Features

- **Copy and Bind**: Copies the specified directory to a writable location and binds it to the original location.
- **Custom Writable Directory**: Allows specifying a custom writable directory instead of the default `/tmp`.
- **Unmount Option**: Provides an option to unmount the directory.
- **Verbose Output**: Offers verbose output for detailed operation logs.

## Usage

```sh
MakeRW v1.0

Usage: makerw [-uv] [-t DIR] DIRECTORY

Make DIRECTORY writable by copying it to a writable location and binding it to the original location.

    -t DIR  Use DIR instead of /tmp as the writable directory
    -u      Unmount the directory
    -v      Verbose
```

## Example

To make a directory writable:

```sh
./makerw -v -t /custom/tmp /path/to/directory
```

To unmount a directory:

```sh
./makerw -u /path/to/directory
```

## Dependencies

- Linux operating system
- Standard C libraries
- `libgen.h` for `basename`
- `linux/limits.h` for `PATH_MAX`
- `sys/mount.h` for `mount` and `umount2`

## Build

Compile the source code using `gcc`:

```sh
gcc -o makerw makerw.c
```

## License

This project is licensed under the GNU GPLv3 License. See the LICENSE file for details.