# extension-directories
Provides path to various system directories for Defold.

# Supported platforms

Android, iOS, macOS, Linux, Windows, HTML5.

# URL for Defold
https://github.com/Lerg/extension-directories/archive/master.zip

# `directories.path_for_file()`

This is built to function similar to Corona's `system.pathForFile()` function https://docs.coronalabs.com/api/library/system/pathForFile.html

Generates and returns an absolute path using system-defined directories as the base. An additional optional parameter, `base_directory`, specifies which base directory is used to construct the full path, with its default value being `directories.resource`.

It takes into account `OSX->Bundle Identifier` property in `game.project` and uses it as an `app_id` for directory names on different platforms.

## Syntax

```lua
directories.path_for_file(filename, [base_directory])
```

### filename
String. The name of the file, or a path to the file that is relative to `base_directory`. Alternatively, pass `nil` to get the path to `base_directory`.

### base_directory
Constant. Corresponds to the base directory where the file is located. Default value is `directories.resource` if the parameter is not provided.

## Directories

### directories.application_support
Constant. Path to `Application Support/%app_id%` directory on iOS and macOS. `AppData\Roaming\%app_id%` on Windows. `$HOME/.%app_id%` on Linux.

### directories.caches
Constant. Path for cached files. OS can clear such files to free space.

### directories.documents
Constant. Path for private app's data files. Persistent on restart. Resides in application support directory.

### directories.resource
Constant. Path to application resources.

### directories.temporary
Constant. Path for temporary files.

## Example

```lua
-- Path to a save file.
local path = directories.path_for_file('save.json', directories.documents)
print(path)

-- Path to a temporary file.
local path = directories.path_for_file('data.txt', directories.temporary)
print(path)

-- Path to caches directory.
local path = directories.path_for_file(nil, directories.caches)
print(path)
```

# HTML5 file download

# `directories.download_file()`

Additionaly on HTML5 this extension provides `directories.download_file()` function to download any generated files on the local computer.

## Syntax

```lua
directories.download_file(filename, [download_name])
```

### filename
String. The path to the file which is being downloaded.

### download_name
String. An alternative name for the file to be saved on the local computer. Used instead of the actual name of the file.

## Example

```lua
-- Download file from documents directory.
local path = directories.path_for_file('save.json', directories.documents)
directories.download_file(path)

-- Use an alternative filename for saving.
directories.download_file(path, 'new_name.txt')
```

