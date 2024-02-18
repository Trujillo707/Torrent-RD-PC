# Torrent-RD-PC

Basically what the description says. Open some torrent file via command line argument, then it will download it somewhere on your PC.

# How to use the executable
Download Torrent-RD-PC.exe from the release page and follow step 2 and onwards in the [Build Instructions](#Build-Instructions) section.

# Requirements
* cURL
* A relatively modern copy of a C++ compiler that supports C++20 if you wish to compile source code (I use std::erase from C++20)
* A Windows computer (feel free to port this to Linux if you want)
* Real-Debrid Premium account
    * Get your API token here: https://real-debrid.com/apitoken

# Build Instructions
1. Compile it somewhere to your PC, just make sure it can read/write to that directory.
If manually using g++, make sure to use the `-municode` flag or else it will not compile.
Otherwise, hopefully the CMake project will properly include the flag for you. Realistically, it is likely more efficient to only work on the `.cpp` files.
using a simple text editor instead of using this CMake project. I use CLion, so the bloat is there by default and I have yet to remove it.
I am aware that it is a rather inefficient implementation, but the real runtime bottleneck is the user's internet connection so any code-runtime savings are insignificant.
The RdFile class realistically does not need to exist, but a little bit of OOP doesn't hurt. Even if it's unwarranted. 
3. Create settings.txt and replace `string` with the real values you want. The `outdir` should not be surrounded with quotes. Do `C:\foobar`, not `"C:\foobar"`.

  Example `settings.txt`:
  ```
  token=string
  outdir=string
   ```
3. Make sure to save `settings.txt` to the same directory as the executable.

   **Alternative:** Pass the PATH location of your `settings.txt` as the second command line argument right after the `.torrent` PATH.
5. Make sure to have a copy of cURL accessible from your PATH.
If not, please install cURL.cURL from WinGet (recommended) or go to https://curl.se/download.html.
6. Run the executable with the `.torrent` file directory as the first command line argument, and you're golden. 
You can also drag and drop files on the executable!
This program should work with any software that can properly pass file directories as the only argument.

# This software is free to use, modify, and hopefully improve.
# The author is not responsible for any misuse of said software.
# This tool should only be used to download torrents that the user has the legal right to download.
# No help whatsoever will be provided regarding potentially unauthorized torrents not functioning with Torrent-RD-PC.
